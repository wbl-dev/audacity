# Workbench Online Entry Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove every user-reachable non-diagnostic online entry from the Workbench profile and centralize the product policy that controls those affordances.

**Architecture:** Keep the default Audacity profile unchanged. A shared application-level product policy marks non-diagnostic external links unavailable, and every Workbench-reachable model consumes that policy so QML omits online affordances. Existing Cloud, network, update, marketplace, telemetry and online-help stubs remain the deny-by-default service boundary; diagnostics remains real.

**Tech Stack:** C++17, Qt 6.11/QML, GoogleTest, CMake/CTest, macOS runtime inspection.

**Spec:** `docs/superpowers/specs/2026-10-30-audacity-workbench-slim-profile-design.md`

## Global Constraints

- Workbench keeps Diagnostics and local crash logs; Crashpad remains opt-in at package time.
- Workbench must not expose or launch Audio.com, update, marketplace, telemetry, tutorial-video, online-help or general website URLs.
- Local file reveal/open behavior must continue to work.
- The default Audacity profile must retain its existing online links and URL-opening behavior.
- Qt 6.11 is the supported build environment; Qt 6.12 is not used for verification.

## Review Focus

- A product-policy regression must not re-enable non-diagnostic external links in Workbench; covered by Task 1 product-profile tests.
- Default Audacity must retain its external-link policy to avoid changing the upstream product; covered by Task 1 conditional product-profile tests.
- Realtime-effects empty-state text must remain informative after removing the tutorial link; covered by Task 2 model tests and GUI smoke verification.
- About credits and legal content must remain readable without clickable external anchors; covered by Task 2 model tests and GUI inspection.
- First-launch setup must contain only local appearance/workspace pages and must not show cloud, update, usage-info or video pages; covered by Task 2 startup-policy tests and GUI/source audit.

---

### Task 1: Define the Workbench external-link product policy

**Files:**
- Modify: `src/shared/productprofile.h`
- Create: `src/shared/tests/productprofile_tests.cpp`
- Modify: `src/shared/tests/CMakeLists.txt`

**Interfaces:**
- Produces: `shared::nonDiagnosticExternalLinksAvailable() noexcept`, consumed by Workbench-reachable UI models.
- Preserves: default Audacity reports external links available.

- [ ] **Step 1: Write failing product-policy tests**

Add tests asserting that Workbench reports non-diagnostic external links unavailable and default Audacity reports them available.

- [ ] **Step 2: Run the focused test target and verify RED**

Run: `cmake --build build/workbench-debug --target shared_tests`, then `build/workbench-debug/src/shared/tests/shared_tests --gtest_filter='ProductProfile.*'`

Expected: build fails because `nonDiagnosticExternalLinksAvailable` does not exist.

- [ ] **Step 3: Implement the minimal product-profile external-link policy**

Add the constexpr policy function beside `isWorkbenchProfile()`.

- [ ] **Step 4: Run the focused tests and verify GREEN**

Run: the Step 2 command.

Expected: all `ProductProfile` tests pass.

- [ ] **Step 5: Commit**

Commit message: `fix: centralize workbench external link policy`

### Task 2: Remove Workbench-visible external links

**Files:**
- Modify: `src/projectscene/view/trackspanel/addeffectmenumodel.h`
- Modify: `src/projectscene/view/trackspanel/addeffectmenumodel.cpp`
- Modify: `src/projectscene/qml/Audacity/ProjectScene/trackspanel/TrackEffectsSection.qml`
- Create: `src/projectscene/tests/workbench_online_surfaces_tests.cpp`
- Modify: `src/projectscene/tests/CMakeLists.txt`
- Modify: `src/appshell/qml/Audacity/AppShell/aboutmodel.h`
- Modify: `src/appshell/qml/Audacity/AppShell/aboutmodel.cpp`
- Modify: `src/appshell/qml/Audacity/AppShell/AboutDialogAudacityTab.qml`
- Modify: `src/appshell/qml/Audacity/AppShell/AboutDialogPrivacyTab.qml`
- Modify: `src/appshell/tests/workbench_menu_tests.cpp`
- Modify: `src/appshell/tests/workbench_startup_tests.cpp`

**Interfaces:**
- Consumes: Task 1's shared external-link product policy.
- Produces: `AddEffectMenuModel::externalLinksAvailable()` and `AboutModel::externalLinksAvailable()` as constant QML policy properties.
- Produces: Workbench About credit/legal data without clickable external anchors.

- [ ] **Step 1: Write failing UI-policy tests**

Add Workbench tests asserting that the realtime-effects model and About model report external links unavailable; About credits and GPL content contain no external anchor URLs. Extend startup tests to enumerate only the three local first-launch pages.

- [ ] **Step 2: Run affected tests and verify RED**

Run: `cmake --build build/workbench-debug --target projectscene_tests appshell_tests && ctest --test-dir build/workbench-debug --output-on-failure -R '^(workbench_online_surfaces|workbench_startup|workbench_menu)$'`

Expected: build fails because the new model policy properties do not exist.

- [ ] **Step 3: Implement minimal visible-surface cleanup**

Hide the realtime-effects tutorial link in Workbench while preserving its explanatory text. Render About credits, website, privacy and GPL content without external anchors in Workbench. Keep default Audacity rendering unchanged.

- [ ] **Step 4: Run affected tests and verify GREEN**

Run: the Step 2 command.

Expected: all selected tests pass.

- [ ] **Step 5: Commit**

Commit message: `fix: remove remaining workbench online surfaces`

### Task 3: Verify runtime behavior and record the result

**Files:**
- Modify: `docs/workbench/building.md`
- Modify: `docs/superpowers/specs/2026-10-30-audacity-workbench-slim-profile-design.md`

**Interfaces:**
- Consumes: Tasks 1-2 completed Workbench application and tests.
- Produces: reproducible verification evidence and an accurate remaining-risk record.

- [ ] **Step 1: Build and run automated verification**

Run the full Workbench build, all `workbench_*` tests, affected suites, and full CTest. Record the known three Clip tempo failures separately if unchanged.

- [ ] **Step 2: Run GUI and network smoke verification**

Launch the built app, confirm realtime effects and About contain no clickable non-diagnostic online affordances, and inspect the process for established external network sockets while idle and while opening the audited local dialogs. Do not enable or transmit Crashpad reports.

- [ ] **Step 3: Update verification documentation**

Replace the provisional `Watch video` follow-up note with the implemented policy, exact test/build evidence, runtime network observation and any honest limitations.

- [ ] **Step 4: Run final checks**

Run: `git diff --check`, the full build, `ctest --test-dir build/workbench-debug --output-on-failure -R '^workbench_'`, and the full test suite.

Expected: build and Workbench tests pass; any full-suite failure is limited to the already-recorded Clip tempo tests.

- [ ] **Step 5: Commit**

Commit message: `docs: record workbench online cleanup verification`
