# Audacity Workbench Slim Profile Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a reproducible Audacity Workbench build profile that preserves local audio workflows, removes Audacity product-service surfaces, and permits network access only through crash diagnostics.

**Architecture:** Keep the upstream `audacity` profile unchanged and introduce a compile-time `workbench` product profile. The profile selects existing Muse/Audacity stub modules for cloud, update, usage, MuseHub, and generic networking; C++ product-policy helpers remove unreachable commands and UI surfaces while Diagnostics/Crashpad remains independent of the generic network module.

**Tech Stack:** CMake 3.24+, C++20, Qt 6/QML, Muse Framework modules and stubs, GoogleTest/GoogleMock.

**Spec:** `docs/superpowers/specs/2026-10-30-audacity-workbench-slim-profile-design.md`

## Global Constraints

- The default product profile is exactly `audacity`; its current build options and UI behavior must remain unchanged.
- The new product profile is exactly `workbench`.
- Workbench keeps local project, import/export, playback, recording, waveform, spectrogram, label, built-in effects, Nyquist, VST3, and macOS Audio Unit capabilities.
- Workbench disables Audio.com, MuseHub, UsageInfo, app updates, online language updates, online learning/content, and generic Muse networking.
- Workbench keeps `MUSE_MODULE_DIAGNOSTICS=ON`; Crashpad remains controlled by the existing `MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT`, handler-path, and report-URL settings.
- Workbench does not physically delete `src/au3cloud`, `src/musehub`, `src/usageinfo`, Publish QML, or cloud project metadata in this change.
- Workbench must continue to load built-in translations from local `.qm` resources; only language downloading is removed.
- No algorithm interfaces, result tracks, `.aup4` schema changes, branding replacement, or interaction optimization belong in this plan.

## Review Focus

- A cached build directory switched from `audacity` to `workbench` must force forbidden modules off rather than retaining stale `ON` values; Task 1 tests the cache override.
- An explicit `audacity://` cloud or publish URI supplied on the command line must not reach a cloud command in Workbench; Task 3 tests URI rejection and page registration.
- A local project containing historical cloud metadata must still open as a local project without initiating sync; Task 4 tests local-open behavior with cloud stubs.
- Selecting a bundled language must still work while “check for language updates” is unavailable; Task 5 tests both behaviors.
- Crash diagnostics must remain registered when generic networking is stubbed, while all ordinary Muse network requests return `NotSupported`; Tasks 2 and 6 test the allowlist boundary.

---

### Task 1: Product Profile and Deterministic Build Policy

**Files:**
- Create: `buildscripts/cmake/SetupProductProfile.cmake`
- Create: `buildscripts/cmake/tests/product_profile_test.cmake`
- Create: `src/shared/productprofile.h`
- Modify: `CMakeLists.txt:53-145`
- Modify: `src/shared/CMakeLists.txt`
- Modify: `src/app/app_config.h.in`
- Modify: `CMakePresets.json`
- Test: `buildscripts/cmake/tests/product_profile_test.cmake`

**Interfaces:**
- Consumes: Existing Audacity and Muse CMake module options.
- Produces: `AU4_PRODUCT_PROFILE` cache string, `AU4_WORKBENCH_PROFILE` compile definition, and `au::shared::isWorkbenchProfile() noexcept -> bool`.

- [ ] **Step 1: Write the failing CMake policy test**

Add cases named `default_profile`, `workbench_profile`, `cached_workbench_override`, and `invalid_profile` to `product_profile_test.cmake`. Assert:

```cmake
# default_profile
assert_strequal(AU4_PRODUCT_PROFILE "audacity")

# workbench_profile and cached_workbench_override
assert_strequal(AU4_PRODUCT_PROFILE "workbench")
assert_false(AU_BUILD_CLOUD_AUDIOCOM)
assert_false(AU_BUILD_MUSEHUB_MODULE)
assert_false(AU_BUILD_USAGEINFO_MODULE)
assert_false(MUSE_MODULE_UPDATE)
assert_false(MUSE_MODULE_NETWORK)
assert_false(MUSE_MODULE_LEARN)
assert_true(MUSE_MODULE_DIAGNOSTICS)
assert_true(MUSE_MODULE_LANGUAGES)

# invalid_profile
# configuration exits non-zero and contains "AU4_PRODUCT_PROFILE must be audacity or workbench"
```

- [ ] **Step 2: Run the policy cases and verify they fail**

Run:

```bash
cmake -DTEST_CASE=default_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=workbench_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=cached_workbench_override -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=invalid_profile -P buildscripts/cmake/tests/product_profile_test.cmake
```

Expected: the first three fail because `SetupProductProfile.cmake` does not exist; the invalid case has no required validation message.

- [ ] **Step 3: Implement `SetupProductProfile.cmake` and apply it after base module defaults are assigned**

Define exactly two accepted values and expose them through the CMake cache UI. Include the policy after the existing `AU_BUILD_*` options and `MUSE_MODULE_*` defaults have been declared, but before `SetupConfigure.cmake`, so Workbench can force stale cache values and the source defaults for Cloud, MuseHub, UsageInfo, Update, Learn, and Network to `OFF`. Keep `MUSE_MODULE_DIAGNOSTICS` and `MUSE_MODULE_LANGUAGES` `ON`; do not force Crashpad `ON`, because handler path and upload URL remain packaging decisions.

- [ ] **Step 4: Add the shared compile-time policy helper**

Add this public interface to `src/shared/productprofile.h`:

```cpp
namespace au::shared {
constexpr bool isWorkbenchProfile() noexcept;
}
```

Its value must be selected only by `AU4_WORKBENCH_PROFILE`; no runtime preference may change it.

- [ ] **Step 5: Add Workbench configure presets**

Add `workbench-debug` and `workbench-release` configure presets inheriting the existing debug/release presets and setting `AU4_PRODUCT_PROFILE=workbench`. Do not modify the existing Audacity presets.

- [ ] **Step 6: Run the policy tests and configure both profiles**

Run the four CMake script cases again, then:

```bash
cmake --preset audacity-debug
cmake --preset workbench-debug
```

Expected: all script cases pass; both configure commands complete; Workbench output reports Cloud, MuseHub, UsageInfo, Update, Learn, and Network disabled, with Diagnostics and Languages enabled.

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt CMakePresets.json buildscripts/cmake/SetupProductProfile.cmake buildscripts/cmake/tests/product_profile_test.cmake src/shared/CMakeLists.txt src/shared/productprofile.h src/app/app_config.h.in
git commit -m "build: add workbench product profile"
```

### Task 2: Crash-Only Network Allowlist

**Files:**
- Modify: `src/app/appfactory.cpp`
- Create: `src/app/tests/CMakeLists.txt`
- Create: `src/app/tests/workbench_module_policy_tests.cpp`
- Modify: `src/app/CMakeLists.txt`
- Test: `src/app/tests/workbench_module_policy_tests.cpp`

**Interfaces:**
- Consumes: `au::shared::isWorkbenchProfile()` and Muse's real/stub module selection produced by Task 1.
- Produces: A Workbench module graph with Diagnostics first, the Network stub registered, and product-service modules represented only by stubs.

- [ ] **Step 1: Extract and test the module policy**

Introduce an internal query usable by `AppFactory` and tests:

```cpp
namespace au::app {
struct WorkbenchModulePolicy {
    bool diagnostics;
    bool genericNetwork;
    bool cloud;
    bool museHub;
    bool usageInfo;
    bool update;
};

WorkbenchModulePolicy modulePolicy() noexcept;
}
```

Write tests asserting the Workbench values are `{true, false, false, false, false, false}` and that the default Audacity profile continues to reflect its CMake module options.

- [ ] **Step 2: Run the focused test and verify it fails**

Run:

```bash
cmake --build build/workbench-debug --target app_tests
ctest --test-dir build/workbench-debug -R workbench_module_policy --output-on-failure
```

Expected: FAIL because `modulePolicy()` is missing.

- [ ] **Step 3: Implement the module policy without removing Diagnostics**

Keep `DiagnosticsModule` as the first module in both GUI and plugin-registration applications. Make `appfactory.cpp` include `framework/network/networkmodule.h` when `MUSE_MODULE_NETWORK` is defined and `framework/stubs/network/networkstubmodule.h` otherwise. The two modules intentionally expose the same `muse::network::NetworkModule` type. Confirm that the stub request methods return `Ret::Code::NotSupported`; do not route Crashpad through this stub.

- [ ] **Step 4: Add a test for the generic network stub boundary**

Using `muse::network::NetworkManagerStub`, assert `get`, `post`, `put`, `patch`, and `del` return `Ret::Code::NotSupported`. Assert `modulePolicy().diagnostics` remains true in the same test binary.

- [ ] **Step 5: Run the module-policy tests**

Run:

```bash
cmake --build build/workbench-debug --target app_tests
ctest --test-dir build/workbench-debug -R 'workbench_module_policy|workbench_network_stub' --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add src/app/CMakeLists.txt src/app/appfactory.cpp src/app/tests/CMakeLists.txt src/app/tests/workbench_module_policy_tests.cpp
git commit -m "build: restrict workbench networking to diagnostics"
```

### Task 3: App Shell, Startup, and External Page Removal

**Files:**
- Modify: `src/appshell/appshellmodule.cpp`
- Modify: `src/appshell/internal/startupscenario.cpp`
- Modify: `src/appshell/qml/Audacity/AppShell/appmenumodel.cpp`
- Modify: `src/appshell/tests/CMakeLists.txt`
- Create: `src/appshell/tests/workbench_startup_tests.cpp`
- Create: `src/appshell/tests/workbench_menu_tests.cpp`
- Test: `src/appshell/tests/workbench_startup_tests.cpp`
- Test: `src/appshell/tests/workbench_menu_tests.cpp`

**Interfaces:**
- Consumes: `au::shared::isWorkbenchProfile()` from Task 1 and stubbed update/cloud services from Tasks 1-2.
- Produces: Workbench page registration, startup behavior, and menus with no external product-service destinations.

- [ ] **Step 1: Write the Workbench page-registration tests**

Extract a pure page manifest used by `AppShellModule::resolveImports()`:

```cpp
namespace au::appshell {
struct AppShellPageManifest {
    std::vector<muse::Uri> pages;
    std::vector<muse::Uri> dialogs;
};

AppShellPageManifest appShellPageManifest();
}
```

Assert the Workbench manifest contains `audacity://home`, `audacity://project`, `audacity://about/audacity`, and `audacity://firstLaunchSetup`, but not `audacity://publish`, `audacity://signin/audiocom`, or `audacity://welcomedialog`.

- [ ] **Step 2: Write startup tests for first launch, updates, welcome, and external URLs**

Add tests with existing mocks that assert:

- First launch may open `audacity://firstLaunchSetup` and create a new local project.
- A completed first launch does not open `audacity://welcomedialog`.
- `runOnSplashScreen()` does not call the update scenario in Workbench.
- `setStartupUrl("audacity://open?..." )` does not dispatch a cloud command in Workbench.
- Session recovery and explicit local project startup remain unchanged.

- [ ] **Step 3: Run the AppShell tests and verify they fail**

Run:

```bash
cmake --build build/workbench-debug --target appshell_tests
ctest --test-dir build/workbench-debug -R 'workbench_page_manifest|workbench_startup' --output-on-failure
```

Expected: FAIL because the page manifest and Workbench branches do not exist.

- [ ] **Step 4: Implement Workbench page and startup behavior**

Use the manifest for URI registration. In Workbench, keep local Home, Project, About, and first-launch customization; skip update checks, the commercial welcome dialog, Publish, Audio.com sign-in, and cloud URL dispatch.

- [ ] **Step 5: Write menu tests before changing the menu**

Assert the Workbench File/Help/Effects surfaces do not contain commands for:

```text
PROJECT_SAVE_TO_CLOUD_COMMAND
PROJECT_UPDATE_CLOUD_AUDIO_PREVIEW_COMMAND
PROJECT_SHARE_AUDIO_COMMAND
APP_ONLINE_HANDBOOK_COMMAND
UPDATE_CHECK_COMMAND
EFFECTS_GET_EFFECTS_COMMAND
```

Also assert local New, Open, Save, Export Audio, About, Preferences, Plugin Manager, and diagnostics remain present.

- [ ] **Step 6: Implement the Workbench menu filtering**

Construct menu lists without forbidden entries rather than creating disabled or broken items. Leave the Audacity profile lists unchanged.

- [ ] **Step 7: Run AppShell tests**

Run:

```bash
cmake --build build/workbench-debug --target appshell_tests
ctest --test-dir build/workbench-debug -R 'workbench_page_manifest|workbench_startup|workbench_menu' --output-on-failure
```

Expected: PASS.

- [ ] **Step 8: Commit**

```bash
git add src/appshell/appshellmodule.cpp src/appshell/internal/startupscenario.cpp src/appshell/qml/Audacity/AppShell/appmenumodel.cpp src/appshell/tests
git commit -m "feat: remove online surfaces from workbench shell"
```

### Task 4: Cloud and MuseHub Command Deactivation

**Files:**
- Modify: `src/project/internal/projectcommandsregister.cpp`
- Modify: `src/project/internal/projectuiactions.cpp`
- Modify: `src/project/internal/projectactionscontroller.cpp`
- Modify: `src/project/tests/CMakeLists.txt`
- Create: `src/project/tests/workbench_project_commands_tests.cpp`
- Modify: `src/effects/effects_base/internal/effectsuiactions.cpp`
- Modify: `src/effects/effects_base/internal/effectsactionscontroller.cpp`
- Modify: `src/effects/effects_base/tests/CMakeLists.txt`
- Create: `src/effects/effects_base/tests/workbench_effect_commands_tests.cpp`
- Test: `src/project/tests/workbench_project_commands_tests.cpp`
- Test: `src/effects/effects_base/tests/workbench_effect_commands_tests.cpp`

**Interfaces:**
- Consumes: Workbench compile-time policy from Task 1.
- Produces: Command registries and action-to-command mappings with no Workbench path to Cloud, share, preview upload, or MuseHub/Get Effects.

- [ ] **Step 1: Write Project command-registry tests**

Assert Workbench does not register open-cloud, save-to-cloud, share-audio, open-cloud-audio, or update-cloud-preview commands. Assert local new/open/save/save-as/import/export and label export remain registered.

- [ ] **Step 2: Add the local-open-with-cloud-metadata regression test**

Load an existing local `.aup4` test fixture with a `CloudProjectRecord`, inject the cloud stubs, and assert local loading succeeds without invoking upload, download, or synchronization calls.

- [ ] **Step 3: Write Effects command tests**

Assert Workbench omits `EFFECTS_GET_EFFECTS_COMMAND` and legacy `get-effects`, while keeping plugin manager, effect open/apply, presets, and vendor UI commands.

- [ ] **Step 4: Run the focused tests and verify they fail**

Run:

```bash
cmake --build build/workbench-debug --target project_tests effects_base_tests
ctest --test-dir build/workbench-debug -R 'workbench_project_commands|workbench_effect_commands' --output-on-failure
```

Expected: FAIL because the forbidden commands are still registered.

- [ ] **Step 5: Filter Project command/action registration**

Build the Project command info and UI action lists conditionally. Do not delete cloud controller implementations or cloud project metadata types. Keep default Audacity registration byte-for-byte equivalent in behavior.

- [ ] **Step 6: Filter Get Effects registration**

Exclude the Get Effects UI action, legacy action conversion, and command request in Workbench. Keep scanning and managing locally installed VST3, Audio Unit, Nyquist, and built-in effects.

- [ ] **Step 7: Run Project and Effects tests**

Run the Task 4 focused command again.

Expected: PASS.

- [ ] **Step 8: Commit**

```bash
git add src/project/internal/projectcommandsregister.cpp src/project/internal/projectuiactions.cpp src/project/internal/projectactionscontroller.cpp src/project/tests src/effects/effects_base/internal/effectsuiactions.cpp src/effects/effects_base/internal/effectsactionscontroller.cpp src/effects/effects_base/tests
git commit -m "feat: deactivate cloud and marketplace commands"
```

### Task 5: Preferences and Local-Only Language Behavior

**Files:**
- Modify: `src/preferences/qml/Audacity/Preferences/generalpreferencesmodel.h`
- Modify: `src/preferences/qml/Audacity/Preferences/generalpreferencesmodel.cpp`
- Modify: `src/preferences/qml/Audacity/Preferences/GeneralPreferencesPage.qml`
- Modify: `src/preferences/qml/Audacity/Preferences/internal/LanguagesSection.qml`
- Modify: `src/preferences/tests/CMakeLists.txt`
- Create: `src/preferences/tests/workbench_general_preferences_tests.cpp`
- Test: `src/preferences/tests/workbench_general_preferences_tests.cpp`

**Interfaces:**
- Consumes: Workbench profile and Network stub from Tasks 1-2; existing `ILanguagesService` for local catalog loading.
- Produces: `GeneralPreferencesModel::onlineLanguageUpdatesAvailable() const noexcept -> bool` exposed as a constant QML property.

- [ ] **Step 1: Write preferences tests**

Assert in Workbench:

```cpp
EXPECT_FALSE(model.onlineLanguageUpdatesAvailable());
EXPECT_FALSE(updateModel.isAppUpdatable());
EXPECT_FALSE(usageInfoModel.isUsageInfoAvailable());
```

When Crashpad is compiled, also assert `usageInfoModel.isCrashReportingAvailable()` is true. With a bundled-language fixture, assert changing `currentLanguageCode` loads the local catalog without calling `ILanguagesService::update()`.

- [ ] **Step 2: Run the preferences tests and verify they fail**

Run:

```bash
cmake --build build/workbench-debug --target preferences_tests
ctest --test-dir build/workbench-debug -R workbench_general_preferences --output-on-failure
```

Expected: FAIL because `onlineLanguageUpdatesAvailable` does not exist and the language update control is still exposed.

- [ ] **Step 3: Implement local-only language preferences**

Expose the constant policy property and hide the “check for language updates” affordance in Workbench. Keep the language dropdown and bundled translation loading unchanged. Keep Crash Reports visible when Crashpad is built; hide app update and anonymous usage sections.

- [ ] **Step 4: Run preferences tests and QML lint**

Run:

```bash
cmake --build build/workbench-debug --target preferences_tests
ctest --test-dir build/workbench-debug -R workbench_general_preferences --output-on-failure
cmake --build build/workbench-debug --target all
```

Expected: tests pass and QML compiles without unknown properties or signal handlers.

- [ ] **Step 5: Commit**

```bash
git add src/preferences/qml/Audacity/Preferences/generalpreferencesmodel.h src/preferences/qml/Audacity/Preferences/generalpreferencesmodel.cpp src/preferences/qml/Audacity/Preferences/GeneralPreferencesPage.qml src/preferences/qml/Audacity/Preferences/internal/LanguagesSection.qml src/preferences/tests
git commit -m "feat: keep workbench preferences local only"
```

### Task 6: Full Verification and Operator Documentation

**Files:**
- Create: `docs/workbench/building.md`
- Modify: `docs/superpowers/specs/2026-10-30-audacity-workbench-slim-profile-design.md`
- Test: existing unit tests and the built Workbench application.

**Interfaces:**
- Consumes: Completed Tasks 1-5.
- Produces: Reproducible build instructions and verified feature-reduction evidence.

- [ ] **Step 1: Document exact build and run commands**

Document submodule initialization, required Qt version from `BUILDING.md`, `cmake --preset workbench-debug`, build, test, application path, and how packaging supplies optional Crashpad handler/report URL settings.

- [ ] **Step 2: Run formatting and static checks**

Run:

```bash
git diff --check
cmake -DTEST_CASE=default_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=workbench_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=cached_workbench_override -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=invalid_profile -P buildscripts/cmake/tests/product_profile_test.cmake
```

Expected: no whitespace errors; all policy cases pass.

- [ ] **Step 3: Build and run the full test suite for Workbench**

Run:

```bash
cmake --preset workbench-debug
cmake --build build/workbench-debug
ctest --test-dir build/workbench-debug --output-on-failure
```

Expected: configure, build, and all enabled tests pass.

- [ ] **Step 4: Verify the default Audacity profile still configures and builds**

Run:

```bash
cmake --preset audacity-debug
cmake --build build/audacity-debug
```

Expected: PASS with the original Audacity modules and product-service surfaces enabled according to upstream defaults.

- [ ] **Step 5: Verify the built Workbench binary's module boundary**

Inspect configure output and linked symbols/libraries. Confirm real Cloud, MuseHub, UsageInfo, Update, and Muse Network implementations are absent or replaced by stubs; Diagnostics remains present. If Crashpad is enabled for this build, confirm its handler and report URL are the only configured upload path.

- [ ] **Step 6: Perform the core manual smoke test**

Run the Workbench application and verify:

1. No login, welcome promotion, Publish, cloud save/open/share, Get Effects, online handbook, app update, anonymous usage, or language update entry is reachable.
2. New/open/save local project works.
3. Import three audio files; move, mute, solo, loop, and play them.
4. Waveform, spectrogram, and combined view work.
5. Create and edit a Label track.
6. Record audio and display input/output meters.
7. Apply one built-in effect and scan/open one supported local plugin.
8. Undo/redo, close/reopen, crash recovery, and export work.
9. Crash report preference is present only when Crashpad is compiled.

- [ ] **Step 7: Record verified status in the design document**

Append a verification section with the exact commit, commands, passed tests, unavailable manual checks, and any observed residual network path. Do not mark an unperformed check as passed.

- [ ] **Step 8: Commit**

```bash
git add docs/workbench/building.md docs/superpowers/specs/2026-10-30-audacity-workbench-slim-profile-design.md
git commit -m "docs: document workbench build and verification"
```
