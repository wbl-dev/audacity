# Scriptable Label Import Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Import user-defined text annotation formats through JavaScript, with explicit point/range labels and immediate label playback.

**Architecture:** A label parser registry chooses a bundled TXT script or a user script by suffix. A bounded helper process runs JavaScript and returns validated plain label data; only the host mutates the project. AU3 labels store an explicit kind, while drop and menu import share the same parser path.

**Tech Stack:** C++/Qt 6 (`QJSEngine`, `QProcess`), QML, CMake, GoogleTest, existing AU3 label and playback modules.

**Spec:** `docs/superpowers/specs/2026-11-10-scriptable-label-import-design.md`

## Global Constraints

- Bundle only TXT; remove built-in SRT and WebVTT import registrations. User scripts may claim either suffix.
- Input is UTF-8 text; `parse({fileName, extension, text})` returns `{trackName, labels}`. Each label has `type: "point"` and `start`, or `type: "range"`, `start`, and `end > start`, plus `title` and optional independent `lowFrequency`/`highFrequency`.
- Project files persist explicit point/range kind; old files infer it from time equality. Point times remain equal in legacy `t`/`t1` fields.
- Scripts get no project, filesystem, or network API. Enforce input/output, time, and process memory limits; kill stalled workers.
- A failed file creates no track. A multi-file batch keeps successful files, reports failures, and creates one undo state for its successes.
- Interval click plays the exact span once; point click plays from its time until stopped. Drag, resize, modified selection, and double-click do not play.
- F0 and MIDI spectrogram overlays are a separate phase.

## Review Focus

- A file ending `.TXT` routes to the same parser as `.txt`; test in Task 4.
- An audio or project suffix claimed by a script is rejected before import routing changes; test in Task 5.
- An infinite loop or oversized script result cannot hang the editor or leave a track; test in Task 3.
- A legacy project without kind loads and saves point labels correctly, including frequency bounds; test in Task 1.
- Clicking a point's widened hit target does not play when the gesture becomes a drag; test in Task 2.

---

### Task 1: Persist label kind

**Files:** Modify `au3/libraries/au3-label-track/LabelTrack.h`, `LabelTrack.cpp`, `src/trackedit/dom/label.h`, `src/au3wrap/internal/domconverter.cpp`, `src/trackedit/internal/au3/au3labelsinteraction.cpp`, and `src/projectscene/view/tracksitemsview/tracklabelitem.cpp`. Test in `src/trackedit/tests/au3labelsinteractions_tests.cpp` and `src/project/tests/`.

**Interfaces:** Define `LabelKind { Point, Range }` on `LabelStruct` and mirrored `trackedit::Label::kind`; expose `LabelStruct::GetKind()` and `SetKind(LabelKind)`. XML `kind="point|range"` is optional on read and always written. Old files infer once from `t == t1`.

- [ ] Write failing tests for old/new XML, copying/undoing labels, explicit kind surviving reopen, and frequency bounds surviving reopen.
- [ ] Run focused CTest labels/project suites and confirm failures.
- [ ] Add kind persistence and propagate it through conversion, creation, edits, and copy paths; reject zero-duration `Range`.
- [ ] Run focused CTest suites and confirm pass.
- [ ] Commit the task using configured Git identity.

### Task 2: Point and range interaction

**Files:** Modify `src/projectscene/qml/Audacity/ProjectScene/tracksitemsview/LabelItem.qml`, `TrackLabelsContainer.qml`, `src/projectscene/view/tracksitemsview/tracklabelslistmodel.{h,cpp}`, `src/projectscene/view/tracksitemsview/labeleditor/labelstableviewmodel.{h,cpp}`, and playback wiring. Test in `src/projectscene/tests/` and `src/playback/tests/playbackcontroller_tests.cpp`.

**Interfaces:** Add model action `activateLabel(const LabelKey&)`; it dispatches existing `PLAYBACK_PLAY_SELECTION_COMMAND` after selecting a range, or seeks and starts ordinary playback for a point. The editor exposes kind and explicit conversions: Range→Point keeps start; Point→Range ends one second later.

- [ ] Write failing tests for interval/point activation, no playback after drag/resize/multiselect/double-click, conversions, and a 16-pixel point hit target.
- [ ] Run focused projectscene/playback CTest suites and confirm failures.
- [ ] Implement kind-specific rendering, click-on-release activation, editor fields and conversions; keep keyboard access.
- [ ] Run focused suites and QML style check, then commit.

### Task 3: JavaScript parser contract and worker

**Files:** Create `src/importexport/labels/internal/labelscriptdata.{h,cpp}`, `labelscriptworker.cpp`, `labelscriptengine.{h,cpp}`, and tests under `src/importexport/labels/tests/`. Modify `src/importexport/labels/CMakeLists.txt` and application packaging as needed.

**Interfaces:** `LabelScriptEngine::parse(const QString& source, const LabelScriptInput& input) -> muse::RetVal<ParsedLabelTrack>`. `ParsedLabelTrack` contains track name and typed labels with optional independent frequency bounds. Host validates result before mutation. Worker uses `QJSEngine` with only plain JSON input/output over a length-prefixed IPC stream.

- [ ] Write failing tests for valid points/ranges, missing kind, invalid numbers, optional `-1` frequency mapping, exceptions, non-UTF-8 input, timeout, and output-size rejection.
- [ ] Run focused labels CTest suite and confirm failures.
- [ ] Implement bounded helper process and host validator; terminate on timeout/crash and report a file-specific error.
- [ ] Run focused suite and confirm pass, then commit.

### Task 4: Bundled TXT script and registry

**Files:** Create `src/importexport/labels/resources/audacity-txt.js` and `internal/labelparserregistry.{h,cpp}`. Modify `labels.qrc`, `internal/au3/au3labelsimporter.{h,cpp}`, `labelsutils.h`, and `internal/labelsconfiguration.cpp`. Test under `src/importexport/labels/tests/`.

**Interfaces:** `LabelParserRegistry::parserForSuffix(QString) -> optional<LabelParserProfile>` and `supportedExtensions() -> vector<string>`. Bundled TXT uses the same `LabelScriptEngine` and output contract as user profiles.

- [ ] Write failing fixture tests for TXT point/range rows, mixed-case `.TXT`, locale-compatible numbers, frequency continuation lines with each bound independently undefined, and malformed-line atomic failure.
- [ ] Run focused labels CTest suite and confirm failures.
- [ ] Implement bundled TXT script, registry, and label-track creation only after successful validation. Remove built-in SRT/WebVTT import filters.
- [ ] Run TXT round-trip and focused labels tests, then commit.

### Task 5: User parser profiles

**Files:** Modify `src/importexport/labels/ilabelsconfiguration.h` and `internal/labelsconfiguration.{h,cpp}`. Create a Preferences model/page in `src/preferences/qml/Audacity/Preferences/` and register it in its CMake/preferences model.

**Interfaces:** Profiles have ID, display name, normalized suffix list, enabled flag, and script source under user data. Expose add/edit/remove/enable and test-with-sample operations. One enabled profile owns a suffix; reserved audio/project suffixes are rejected; overriding bundled TXT requires a warning.

- [ ] Write failing configuration tests for save/reload, case-insensitive conflicts, reserved suffixes, disabled profiles, and preserving a valid profile after a failed edit.
- [ ] Run focused configuration tests and confirm failures.
- [ ] Implement profile storage and Preferences editor with sample-file preview and script errors.
- [ ] Run focused tests and manual Preferences QA, then commit.

### Task 6: Shared menu and drop routing

**Files:** Modify `src/importexport/import/internal/au3/au3importer.cpp`, `src/projectscene/view/tracksitemsview/dropcontroller.{h,cpp}`, `src/projectscene/qml/Audacity/ProjectScene/tracksitemsview/ImportDropArea.qml`, `src/project/internal/audacityproject.cpp`, and Labels Editor import filters.

**Interfaces:** Classify files as project, audio, or label before probing; label paths call the parser registry and create label tracks without reserving wave tracks or calling `importIntoTrack`. General import, Labels Editor import, and drop share this path.

- [ ] Write failing tests for TXT/custom drop, mixed audio+label batch, invalid label file, partial batch success, dynamic file-dialog suffixes, and one undo state for successful batch items.
- [ ] Run focused import/projectscene CTest suites and confirm failures.
- [ ] Implement routing and visible error aggregation, then run focused suites and commit.

### Task 7: End-to-end verification

**Files:** Add integration fixtures/tests where needed; update user-facing documentation for the script API and the SRT/WebVTT import change.

- [ ] Configure with `cmake --preset audacity-debug -DMUSE_ENABLE_UNIT_TESTS=ON`, build with `cmake --build build/audacity-debug`, and run `ctest --test-dir build/audacity-debug --output-on-failure`.
- [ ] Run C++ and QML CI style checks; manually verify TXT/custom drag, script preview, label playback, project reopen, and undo on supported desktop platforms.
- [ ] Review the resulting diff against every spec requirement and fix gaps.
- [ ] Commit tests and documentation using configured Git identity.
