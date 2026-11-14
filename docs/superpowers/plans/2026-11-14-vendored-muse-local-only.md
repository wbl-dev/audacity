# Vendored Muse Local Only Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make OnlyAudios independent of the upstream Muse submodule and remove its online product modules.

**Architecture:** Track Muse source in this repository at the existing `muse/` path. Replace Muse's online language updater and network-aware QML manager with local-only behavior, then remove unused online module trees and build options.

**Tech Stack:** Git, CMake, C++20, Qt/QML, GoogleTest, Qt Linguist TS.

**Spec:** `docs/superpowers/specs/2026-11-14-vendored-muse-local-only-design.md`

## Global Constraints

- Preserve local editing, `.aup4` projects, recording, effects, plugins, labels, Diagnostics, optional Crashpad, and installed language selection.
- QML may load `qrc:`, `file:`, and `data:` resources; remote URLs must be rejected.
- Keep the Muse license and source notices; keep `muse_deps` as a separate submodule.
- The supported build is OnlyAudios; the vendored Muse source need not build as standalone MuseScore Studio.

## Review Focus

- A fresh checkout contains Muse source and has no `muse` gitlink.
- Language choice and locally installed `.qm` files still load.
- Remote QML requests fail without touching the network, while local files load.
- The executable link rule contains no Cloud, Network, Update, or Learn module or stub.
- Translation catalogs retain shared local strings when removing online-only locations.

---

### Task 1: Vendor the Muse source

**Files:** `.gitmodules`, `muse/**`, `docs/superpowers/specs/2026-11-14-vendored-muse-local-only-design.md`.

- [x] Record the source revision `ad836ff9e0f0858c83e611e43c73091ee923cff1`.
- [x] Remove the `muse` gitlink and its `.gitmodules` entry, keep `muse_deps`, and track all retained Muse source at the same path.
- [x] Verify `git ls-files -s muse` contains ordinary files and no mode `160000` entry.

### Task 2: Keep languages local

**Files:** `muse/framework/languages/{ilanguagesservice.h,ilanguagesconfiguration.h,internal/*,languageserrors.h}`, `muse/framework/stubs/languages/*`, `src/preferences/tests/workbench_general_preferences_tests.cpp`, `src/app/configs/languages.cfg`.

- [x] Keep the focused installed-language preference test and adjust its mocks to the local-only interfaces.
- [x] Remove `LanguagesService::update(const QString&) -> Progress`, download and hash helpers, network injection, server URL methods, and unused update errors. Keep local language loading and language change notifications.
- [x] Remove the `server_url` language configuration value and adjust mocks/stubs.
- [x] Build and run language and preference tests.

### Task 3: Centralize local QML resource access

**Files:** `muse/framework/ui/internal/qmlnetworkaccessmanagerfactory.{h,cpp}`, `muse/framework/ui/tests/*`, `src/appshell/internal/localnetworkpolicy.{h,cpp}`, `src/appshell/appshellmodule.cpp`, `src/appshell/{CMakeLists.txt,tests/*}`.

- [x] Test that Muse's QML manager reads `file:` resources and rejects `https:` with `ContentAccessDenied`.
- [x] Make the Muse QML manager the local-only factory; remove its `INetworkConfiguration` injection and outgoing header logic.
- [x] Remove the duplicate app factory/provider and keep one focused test for the resulting policy.
- [x] Build Muse UI and run the AppShell test against Muse's QML network manager.

### Task 4: Delete online module families

**Files:** `muse/framework/{cloud,network,update,learn}/**`, matching `muse/framework/stubs/**`, `muse/framework/{CMakeLists.txt,stubs/CMakeLists.txt,cmake/MuseDeclareOptions.cmake,cmake/MuseModules.cmake}`, `CMakeLists.txt`, `buildscripts/cmake/SetupProductProfile.cmake`.

- [x] Remove the four implementation and stub trees and their CMake options, directory registrations, and root disabled flags.
- [x] Fresh configure and full OnlyAudios build; resolve any remaining compile or link references.
- [x] Confirm the app link rule excludes all four module families.

### Task 5: Clean catalogs and verify

**Files:** `share/locale/audacity_*.ts`, documentation and tests as needed.

- [x] Remove translation locations and messages belonging solely to deleted online source; preserve messages with local source locations.
- [x] Run full CTest, `git diff --check`, source-reference search, and a clean-checkout structural audit.
- [x] Record results and any remaining external dependency boundary.

## Verification results

- Fresh `workbench-debug` configure and complete OnlyAudios build passed on macOS with Qt 6.11.3.
- Full CTest passed: 40/40 tests at parallelism 4.
- All 70 Audacity TS files parse as XML, and all generated `.qm` files rebuilt.
- `git diff --check HEAD` and `bash -n tools/translations/run_lupdate.sh` passed.
- The application link edge contains none of the removed online modules. The vendored `muse/` index contains 2971 ordinary files and no gitlink. `muse_deps` remains the only external source submodule.
