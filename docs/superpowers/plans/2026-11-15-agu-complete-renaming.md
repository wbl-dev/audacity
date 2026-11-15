# AGu Complete Renaming Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rename the AU4 application, build project, owned interfaces and packages to AGu while preserving its audio engine and existing project formats.

**Architecture:** Change the application identity first, then rename AU4 C++ and QML interfaces, then package metadata and user-facing links. Keep `au3/` engine symbols and legacy project format identifiers as compatibility boundaries. AGu uses fresh settings and coexists with OnlyAudios.

**Tech Stack:** CMake, C++20, Qt/QML, macOS bundles, Windows WiX, Linux desktop packaging, CTest.

**Spec:** `docs/superpowers/specs/2026-11-15-agu-complete-renaming-design.md`

## Global Constraints

- Display name `AGu`; macOS `AGu.app`/`AGu`, Windows `AGu.exe`, Linux `agu`; version remains `4.0.0`.
- CMake project `AGu`, main target `agu`; application URL scheme `agu://`, QML namespace `AGu.*`, owned C++ namespace `agu::`.
- Bundle ID `org.agu.app`; Windows uses a new UpgradeCode/ProgID; Linux uses a new desktop/appdata ID. Existing OnlyAudios installations and settings are untouched.
- Do not rename retained `au3/` engine types, legal attribution, `.aup3`/`.aup4` extensions, or format compatibility metadata.
- No Audacity website/help/support links in product UI or package metadata until AGu destinations exist. Preserve license and provenance links.
- Preserve untracked `output/` with candidate artwork.

## Review Focus

- AGu and OnlyAudios installed together: installing AGu must not upgrade/remove OnlyAudios or reuse its settings.
- Existing `.aup3` and `.aup4` files: AGu must still open them after file association and URI changes.
- QML static and dynamic registrations: each renamed `AGu.*` import, registered type and resource path must agree.
- Linux/Windows conditional packages: desktop IDs, executable paths, installer ProgIDs and resource locations must resolve to AGu.
- UI links: menu/about/splash/help and package metadata must not send users to Audacity product support as AGu support.

---

### Task 1: Product identity and main build target

**Files:** `version.cmake`, root `CMakeLists.txt`, `src/app/{CMakeLists.txt,main.cpp}`, `SetupConfigure.cmake`, `CMakePresets.json`, active build scripts and `BUILDING.md`.

**Interfaces:** Produce CMake project `AGu` and target `agu`; output paths specified in Global Constraints. Set Qt app/organization identity to AGu without reading OnlyAudios settings.

- [ ] Capture a failing baseline: `OnlyAudios --version` currently does not report AGu; CMake project/target still read audacity.
- [ ] Rename root project, executable target and all target references; update `version.cmake`, `main.cpp`, preset descriptions, setup paths, installer script path/name and active build docs.
- [ ] Reconfigure and build the app; `AGu --version` reports AGu and the macOS bundle has bundle ID `org.agu.app`.

### Task 2: Owned C++ interfaces

**Files:** C++ and tests under `src/` using `namespace au`, `au::`, or app-owned `Audacity` names; no mechanical edits under `au3/` or `muse/`.

**Interfaces:** Replace AU4-owned `au::` with `agu::`; preserve external AU3 engine ABI and file types.

- [ ] Inventory exact identifier matches and add an audit check for remaining `namespace au`/`au::` in active `src/` code.
- [ ] Rename declarations, definitions, uses and tests together; update app-owned class/API identifiers named Audacity where they express the product rather than format compatibility.
- [ ] Build all configured targets and run affected CTest suites; audit remaining matches individually.

### Task 3: QML modules and internal URI contracts

**Files:** `src/**/qml/Audacity/` trees, `qmldir`, QML CMake files, `src/**/*.qrc`, C++ QML registrations, URI users and related tests; translation source locations in `share/locale/`.

**Interfaces:** QML module prefix `AGu.` and resource path `AGu/`; page/dialog URI prefix `agu://`; extension API prefix `AGu.`.

- [ ] Record failing baseline for old URI/QML names using current tests and source audit.
- [ ] Move QML module directories to `AGu`, update imports, CMake URI, registered types, QRC paths, tests and all producer/consumer URI strings in one coherent pass.
- [ ] Reconfigure, build and run app shell, menu, project, effects, import/export and QML resource tests; confirm no active old URI remains.

### Task 4: Icons, packages, translations and links

**Files:** `share/icons/`, `share/locale/`, `buildscripts/packaging/`, `buildscripts/ci/`, `src/appshell/`, active docs and installer/testflow scripts.

**Interfaces:** Formal icon source `AGu.png`; package identities and filenames use AGu; Windows new UpgradeCode and ProgID; Linux new desktop/appdata ID. Keep old project MIME/extension where needed to open projects.

- [ ] Rename icon source and platform derivatives with updated build references; rename translation catalog/resource prefix and update changed QML source paths.
- [ ] Remove Audacity product destinations from menu/about/splash/package metadata where there is no AGu URL; retain license/upstream attribution.
- [ ] Update macOS/Windows/Linux package names, installer identities and executable references; audit potential replacement of OnlyAudios on Windows.
- [ ] Build and inspect macOS package metadata, icon and signing; statically audit Windows/Linux generated installation rules.

### Task 5: Complete verification and commit

**Files:** Updated active documentation and final naming audit only.

- [ ] Reconfigure and build all targets from `workbench-debug`, run `ctest --test-dir build/workbench-debug --output-on-failure` and `git diff --check`.
- [ ] Run `AGu --version`, inspect bundle ID/name/icon/resources and verify old OnlyAudios user paths and untracked `output/` remain untouched.
- [ ] Audit active source and generated rules for old product names; document compatibility/legal exceptions and platform checks unavailable on this macOS host.
- [ ] Request a fresh whole-change code review, address material findings, then commit the completed rename without including `output/`.
