# Local Only Source Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Make Workbench the sole local-only product and physically remove disabled online features and their dependencies.

**Architecture:** Keep the local editor and project model. Remove the dual profile and service modules, then simplify all project, app shell, preference, and QML paths that refer to them. Delete unreferenced implementation and stub trees only after the application no longer includes their interfaces.

**Tech Stack:** CMake, C++20, Qt/QML, GoogleTest.

**Spec:** `docs/superpowers/specs/2026-11-13-local-only-source-cleanup-design.md`

## Global Constraints

- Preserve local editing, recording, effects, plugin, label, and `.aup4` file operations.
- Preserve Diagnostics and optional Crashpad.
- Remove legacy cloud metadata handling.

## Review Focus

- Existing local `.aup4` files still open and save.
- Recent projects list remains local and opens files.
- Autosave and crash recovery still work.
- Export audio and labels still work.
- Build graph contains no online product modules or stubs.

---

### Task 1: Single local product build

**Files:** `CMakeLists.txt`, `CMakePresets.json`, `buildscripts/cmake/SetupProductProfile.cmake`, `buildscripts/cmake/tests/product_profile_test.cmake`, `src/CMakeLists.txt`, `src/app/appfactory.cpp`, `src/app/appfactory.h`.

- [x] Change product policy tests to require the local-only defaults; observe failure.
- [x] Remove dual profile, configure online modules off, and simplify app registration.
- [x] Configure and build application and policy tests; observe pass.

### Task 2: Local-only project core

**Files:** `src/project/types/projecttypes.h`, `src/project/internal/*`, `src/project/view/*`, `src/project/qml/**`, `src/project/projectmodule.*`, `src/project/project.qrc`, project tests.

- [x] Add focused tests for local save/open and no cloud metadata; observe failure.
- [x] Remove cloud save/open, sync, metadata, commands, pages, and models.
- [x] Build and run project tests; observe pass.

### Task 3: Remove remaining online product surfaces

**Files:** `src/appshell/**`, `src/preferences/**`, `src/projectscene/**`, `src/importexport/**`.

- [x] Add focused tests for remaining local UI and commands; observe failure.
- [x] Remove online first launch, publish, account, preferences, and share paths.
- [x] Build and run affected tests; observe pass.

### Task 4: Delete dead implementations and verify

**Files:** `src/au3cloud/**`, `src/musehub/**`, `src/usageinfo/**`, `src/stubs/**`, relevant `au3/libraries/**`, build files and documentation.

- [x] Remove unreferenced modules, interfaces, resources, and build options.
- [x] Fresh configure and full build.
- [x] Run full CTest and audit source/build graph for residual references.
- [x] Update build documentation to describe the sole local product.
