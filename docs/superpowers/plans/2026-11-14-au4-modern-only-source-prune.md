# Audacity 4 Modern-only source prune Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep the existing Audacity 4 editor as a Modern-only shell and delete obsolete Audacity 3 application code and resources.

**Architecture:** Keep the AU4 `src/` app, Muse layout persistence, and the AU3 engine libraries required by AU4. Narrow workspace loading to Modern, remove workspace selection UI, simplify workspace-dependent preferences, then prune legacy AU3 source outside the cross-platform AU4 build closure.

**Tech Stack:** CMake, C++20, Qt/QML, GoogleTest, CTest.

**Spec:** `docs/superpowers/specs/2026-11-14-au4-modern-only-source-prune-design.md`

## Global Constraints

- Preserve existing AU4 recording, editing, effects, project, and import/export features.
- Keep all AU4 conditional platform/plugin libraries, not only files seen in the macOS compile database.
- Ignore old user workspace files without deleting them; existing Classic/Music/custom selection falls back to Modern.
- Preserve the pre-existing build script edit and untracked `output/`.
- Do not delete old local project import capability solely because it reads AU3 formats.

## Review Focus

- Existing `Classic` selection with a stale user `Classic.mws` must load Modern and rewrite the setting (Task 1 test).
- Existing user `Modern.mws` must override the packaged Modern file without allowing other user workspaces (Task 1 test).
- Workspace-dependent tempo/stereo preferences with Modern in the saved list must become Always, otherwise Never (Task 3 tests).
- Modern's packaged dock state must have no `workspacesToolBar` reference after toolbar removal (Task 2 resource check).
- Optional LV2, Audio Unit, VST3, Nyquist, and platform AU4 code must survive AU3 pruning (Task 4 source audit).

---

### Task 1: Enforce the Modern workspace

**Files:**
- Modify: `src/app/configs/workspaces.cfg`, `share/workspaces/CMakeLists.txt`
- Delete: `share/workspaces/Classic.mws`, `share/workspaces/Music.mws`
- Modify: `muse/framework/workspace/internal/workspacemanager.cpp`
- Test: `muse/framework/workspace/tests/workspacemanager_tests.cpp`, `muse/framework/workspace/tests/workspaceconfig_tests.cpp`

**Interfaces:** `WorkspaceManager::findWorkspaceFiles() const` returns only built-in or user Modern path; existing `setupCurrentWorkspace()` rewrites stale names through `IWorkspaceConfiguration::setCurrentWorkspaceName`.

- [ ] Add focused tests for stale Classic/current custom names and user Modern override while ignoring other user files.
- [ ] Run `muse_workspace_tests` and confirm the new assertions fail.
- [ ] Limit built-in config and loader to Modern and delete unused `.mws` files.
- [ ] Run focused workspace tests and confirm pass.

### Task 2: Remove workspace selection surfaces

**Files:**
- Modify: `src/appshell/qml/Audacity/AppShell/{CMakeLists.txt,appmenumodel.cpp,appmenumodel.h,firstlaunchsetupmodel.cpp,projectpagemodel.cpp,projectpagemodel.h,ProjectPage/ProjectPage.qml}`
- Modify: `src/projectscene/{CMakeLists.txt,projectscenemodule.cpp,projectscene.qrc}` and `src/projectscene/qml/Audacity/ProjectScene/qmldir`
- Delete: first-launch `WorkspaceLayoutPage.qml` and its model; `WorkspacesToolBar.qml` and its model; `src/shared/types/workspacetitles.h`
- Modify: `muse/framework/workspace/{CMakeLists.txt,workspacemodule.cpp}`; remove workspace selection QML/actions/commands after checking any remaining consumers.
- Modify: `share/workspaces/Modern.mws` (zip archive)

**Interfaces:** Workspace data service remains registered; workspace selection commands, menu, toolbar, and dialogs disappear.

- [ ] Remove the app's first-launch layout page, View menu item, dock toolbar, QML registration/resources, and unused title helper.
- [ ] Remove selection/creation command and dialog registration from Muse workspace while preserving manager/data provider.
- [ ] Remove `workspacesToolBar` from Modern's zipped `ui_states` and confirm archive entries still parse.
- [ ] Configure/build the app and check no deleted QML or toolbar reference remains in active registration.

### Task 3: Remove workspace-dependent preferences

**Files:**
- Modify: `src/importexport/import/{types/importtypes.h,iimporterconfiguration.h,internal/importerconfiguration.cpp,internal/importerconfiguration.h,internal/au3/tempodetection.cpp,internal/au3/tempodetection.h}`
- Modify: `src/projectscene/{types/projectscenetypes.h,iprojectsceneconfiguration.h,internal/projectsceneconfiguration.cpp,internal/projectsceneconfiguration.h,view/tracksitemsview/trackclipslistmodel.cpp,view/tracksitemsview/trackclipslistmodel.h}`
- Modify: `src/preferences/qml/Audacity/Preferences/{CMakeLists.txt,MusicPreferencesPage.qml,EditPreferencesPage.qml,musicpreferencesmodel.cpp,musicpreferencesmodel.h,editpreferencesmodel.cpp,editpreferencesmodel.h,internal/AsymmetricStereoHeightsSection.qml}`
- Delete: `internal/WorkspacesTempoDetectionSection.qml`, `internal/WorkspacesAsymmetricChannelsSection.qml`
- Test: affected import/projectscene configuration tests, or focused migration tests beside them.

**Interfaces:** Keep persisted enum value `WORKSPACE_DEPENDENT` for migration, but expose only Always/Never in UI; translate saved list containing `Modern` to Always, otherwise Never. Fresh tempo and stereo defaults are Never.

- [ ] Add migration tests for Modern present/absent in both preference lists.
- [ ] Run focused tests to observe failures.
- [ ] Implement preference migration and remove workspace list APIs and UI controls; retain enum numeric compatibility with old settings.
- [ ] Run focused tests and build affected modules.

### Task 4: Delete legacy AU3 app source outside AU4 closure

**Files:**
- Delete: `au3/src`, `au3/locale`, `au3/nyq-po`, `au3/tests`, `au3/scripts`, `au3/dox2-src`, `au3/linux`, `au3/mac`, `au3/win`, `au3/crashreporter`, and old Doxygen inputs.
- Delete: inactive `au3/modules` trees except `import-export`, plus unused AU3 libraries and uncompiled legacy files whose absence is verified by CMake/include/resource audit.
- Modify: `au3/libraries/CMakeLists.txt`, applicable source comments and docs; remove `tools/translations/au3-au4-migration`.
- Preserve: `au3/au3defs.cmake`, `au3/include`, AU4-linked libraries/modules and conditional platform/plugin code.
- Retain and install `EQDefaultCurves.xml` in the AU4 runtime resources location; delete unused `EffectsMenuDefaults.xml`.

**Interfaces:** CMake target names and public headers of the AU4 engine remain intact.

- [ ] Generate a deletion manifest from CMake source sets and include/resource references; review conditional platform paths before removal.
- [ ] Delete confirmed dead trees/files and stale migration tools; update CMake comments/references.
- [ ] Configure the complete app and tests from `workbench-debug`; restore any required cross-platform dependency mistakenly classified as dead.
- [ ] Build app and tests; verify EQ curve resource is copied to the app bundle/install tree.

### Task 5: Repository verification

**Files:**
- Modify: active build and repository docs that still describe obsolete AU3 application paths or multiple workspaces.
- Test: workspace configuration, app resources, all CTest suites.

**Interfaces:** Fresh checkout can configure/build AU4 without old AU3 app trees; no user workspace files are touched.

- [ ] Reconfigure `workbench-debug`, build all targets, and run `ctest --test-dir build/workbench-debug --output-on-failure`.
- [ ] Audit build rules, packed resources, and source references for deleted paths and Classic/Music workspace entry points.
- [ ] Record exact build/test results and remaining limitations; keep unrelated workspace changes intact.
