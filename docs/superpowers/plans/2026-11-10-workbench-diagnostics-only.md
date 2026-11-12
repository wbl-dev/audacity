# Workbench Diagnostics-Only Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reduce the Workbench developer surface to crash diagnostics while preserving the default Audacity developer tools.

**Architecture:** The Workbench product profile disables TestFlow at configure time. Shared product policy removes the DevTools route and top navigation item, while a small AppShell policy keeps only diagnostic export in the top Diagnostics menu. Sources needed by default Audacity remain intact.

**Tech Stack:** CMake, C++17, Qt 6.11/QML, GoogleTest, CTest.

**Spec:** `docs/superpowers/specs/2026-11-10-workbench-diagnostics-only-design.md`

## Global Constraints

- Default Audacity retains its complete developer tools and TestFlow behavior.
- Workbench keeps Diagnostics, local diagnostic export and crash recovery.
- Crashpad remains opt-in and is not enabled by this change.
- DevTools and TestFlow source files remain in the repository.

## Review Focus

- Reconfiguring a Workbench build must actually omit TestFlow artifacts, not merely hide its menu command.
- Switching the same build tree back to Audacity must restore the cached TestFlow default.
- Workbench developer mode must not re-enable hidden diagnostic groups.
- Workbench developer mode must not restore the DevTools top navigation item.
- Default Audacity must still register DevTools and show its full menu.

---

### Task 1: Disable Workbench TestFlow and reduce the Diagnostics menu

**Files:**
- Modify: `buildscripts/cmake/SetupProductProfile.cmake`
- Modify: `buildscripts/cmake/tests/product_profile_test.cmake`
- Modify: `src/appshell/qml/Audacity/AppShell/appmenumodel.h`
- Modify: `src/appshell/qml/Audacity/AppShell/appmenumodel.cpp`
- Modify: `src/appshell/tests/workbench_menu_tests.cpp`

**Interfaces:**
- Produces: `DiagnosticMenuPolicy diagnosticMenuPolicy() noexcept` with `saveFiles`, `systemTools`, and `developerTools` flags.
- Preserves: the existing default Audacity menu when not in the Workbench profile.

- [ ] **Step 1: Write failing product-profile and menu-policy tests**

Extend the CMake policy cases to require TestFlow off in Workbench and restored in Audacity. Add a Workbench unit test requiring diagnostic export enabled and system/developer diagnostic groups disabled.

- [ ] **Step 2: Run the focused tests and verify RED**

Run the five product-profile scripts and the `workbench_menu.*` filter. Expected: the new TestFlow and diagnostic-menu assertions fail against the current implementation.

- [ ] **Step 3: Implement the minimal policies**

Set `MUSE_MODULE_TESTFLOW OFF` in the Workbench profile. Add the constexpr menu policy and make `AppMenuModel::makeDiagnosticMenu()` build only diagnostic export in Workbench, regardless of developer mode.

- [ ] **Step 4: Reconfigure and verify GREEN**

Fresh-configure `workbench-debug`, build `appshell_tests`, rerun the product-profile scripts and `workbench_menu.*`. Confirm the build tree contains no `libmuse_testflow.a`, TestFlow scripts, or `testflowrunner.cpp.o`.

- [ ] **Step 5: Commit**

Commit message: `fix: reduce workbench diagnostics surface`

### Task 2: Remove the Workbench DevTools Tab

**Files:**
- Modify: `src/shared/productprofile.h`
- Modify: `src/shared/tests/productprofile_tests.cpp`
- Modify: `src/appshell/appshellmodule.cpp`
- Modify: `src/appshell/qml/Audacity/AppShell/maintoolbarmodel.h`
- Modify: `src/appshell/qml/Audacity/AppShell/maintoolbarmodel.cpp`
- Modify: `src/appshell/tests/workbench_startup_tests.cpp`
- Modify: `docs/workbench/building.md`
- Modify: `docs/superpowers/specs/2026-10-30-audacity-workbench-slim-profile-design.md`

**Interfaces:**
- Produces: `shared::fullDeveloperToolsAvailable() noexcept`, true for default Audacity and false for Workbench; `shouldShowDeveloperToolsInMainToolbar(bool) noexcept` combines it with developer mode.
- Consumes: that shared policy in page registration and top navigation construction.

- [ ] **Step 1: Write failing DevTools availability tests**

Require Workbench product policy to report full developer tools unavailable, the Workbench page manifest to omit `audacity://devtools`, and toolbar policy to reject the Tab even when developer mode is true. The default-profile policy test requires the inverse.

- [ ] **Step 2: Run the focused test and verify RED**

Build `shared_tests` and `appshell_tests`; run `ProductProfile.*` and `workbench_page_manifest.*`. Expected: failure because the policy does not exist and the page is still registered.

- [ ] **Step 3: Implement the minimal QML restriction**

Add the shared policy, conditionally register the DevTools route, and require both developer mode and policy availability before adding the DevTools top navigation item. Preserve the full default Audacity behavior.

- [ ] **Step 4: Verify GREEN and update documentation**

Build the complete Workbench app; run all `workbench_*` tests and affected Shared/AppShell tests. Record the Diagnostics-only boundary and leave GUI verification to the user.

- [ ] **Step 5: Commit**

Commit message: `docs: record diagnostics-only workbench boundary`

### Task 3: Final verification and review

**Files:**
- Verify only; modify production files only for review findings proven by a failing test.

**Interfaces:**
- Consumes: Tasks 1-2 complete branch.
- Produces: final build/test evidence and a clean review decision.

- [ ] **Step 1: Run final verification**

Run `git diff --check`, the complete Workbench build, all `workbench_*` tests, affected suites and full CTest. Record the known three Clip tempo failures separately if unchanged.

- [ ] **Step 2: Request a fresh whole-branch code review**

Review the implementation against the spec and Review Focus. Fix any Critical or Important finding through RED-GREEN TDD and rerun the suite.

- [ ] **Step 3: Commit review fixes if any and push the branch**

Push `feature/wbl/online-entry-cleanup` to `wbl` without force.
