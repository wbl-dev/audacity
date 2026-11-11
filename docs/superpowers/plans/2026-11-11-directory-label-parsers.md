# Directory-backed Label Parsers Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Discover suffix parsers from JavaScript files at startup and on demand from the Tools menu.

**Architecture:** `LabelsConfiguration` owns a cached scan of `<userDataPath>/LabelParsers`; `LabelParserRegistry` reads that cache and falls back to bundled TXT. Plain `<suffix>.js` files need no metadata, while UI-created scripts carry a first-line metadata comment. A context command invokes rescan and reports its result; Preferences edits files and offers folder access and sample testing.

**Tech Stack:** C++/Qt 6, QML, GoogleTest, existing label script worker.

**Spec:** `docs/superpowers/specs/2026-11-11-directory-label-parsers-design.md`

## Global Constraints

- Preserve the built-in TXT JavaScript parser and import result contract.
- Scan on startup and by explicit Tools command; run scripts only during import or sample test.
- Do not modify the root `AGENTS.md` as part of this feature.

## Review Focus

- Invalid script beside valid script: valid suffix still imports.
- Removed script after rescan: suffix disappears from drag/drop and import filters.
- `txt.js` override: user source wins; removing it restores built-in TXT.
- Legacy JSON migration: existing file wins and migration does not repeat after removal.
- Reserved audio/project suffix: cannot hijack audio or project import.

---

### Task 1: Directory scanner and migration

**Files:** `src/importexport/labels/internal/labelsconfiguration.{h,cpp}`, `src/importexport/labels/ilabelsconfiguration.h`, `src/importexport/labels/internal/labelparserregistry.cpp`, `src/importexport/labels/tests/labelscriptengine_tests.cpp`, `src/importexport/labels/labelsmodule.cpp`.

- [x] Write scanner, rescan, TXT override, invalid-file, and migration tests; run `labels_tests` to observe failures.
- [x] Implement cached file scan and one-time migration. Initialize at module startup. Keep the worker and importer unchanged.
- [x] Rebuild and run `labels_tests` until green.

### Task 2: Manual rescan command

**Files:** `src/importexport/labels/labelscommands.h`, `src/importexport/labels/internal/labelscommandsregister.{h,cpp}`, `src/importexport/labels/internal/labelscommandscontroller.{h,cpp}`, `src/importexport/labels/labelsmodule.{h,cpp}`, `src/importexport/labels/CMakeLists.txt`, `src/appshell/qml/Audacity/AppShell/appmenumodel.cpp`.

- [x] Add a command registration test and rescan tests showing that supported suffixes refresh.
- [x] Register **Tools → Rescan label parser scripts**, invoke configuration rescan, and show a success/error dialog with supported, added, and removed suffixes.
- [x] Build `audacity` and run focused command and label tests.

### Task 3: Preferences and documentation

**Files:** `src/preferences/qml/Audacity/Preferences/LabelParsersPreferencesPage.qml`, `src/preferences/qml/Audacity/Preferences/labelparserspreferencesmodel.{h,cpp}`, `docs/label-parser-scripts.md`.

- [x] Change the page to show the folder, discovered parser list, Open folder, Rescan, and sample testing of selected script; retain the editor with file-backed Save and Remove.
- [x] Document filename mapping, startup/menu scan, overrides, migration, and script API.
- [x] Build the app, run all CTest tests, and run `git diff --check`.
