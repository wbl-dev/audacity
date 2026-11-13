# Label Owner Metadata Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Support per-label `color` and `owner` from TXT through project persistence and display, with stable owner-based rows.

**Architecture:** The TXT JavaScript parser emits typed metadata. The worker validates it, the AU3 label stores it in project XML, and the track edit model exposes it to QML. Layout assigns rows by owner, point/range kind, and actual time overlap rather than pixel width.

**Tech Stack:** JavaScript parser, C++/Qt, AU3 XML, QML, GoogleTest.

**Spec:** `docs/superpowers/specs/2026-11-13-label-owner-metadata.md`

## Global Constraints

- Preserve legacy TXT and frequency continuation parsing.
- Keep existing uncommitted label UI changes intact.
- Do not add `lane` or use edit `groupId` as owner.
- Do not install or commit automatically.

## Review Focus

- Metadata with duplicate, unknown, or malformed keys fails with a line number.
- A title containing `//` is unchanged because metadata requires a fourth tab field.
- Color and owner survive project XML save/reopen and duplication.
- Tiny adjacent ranges stay on one row at all zoom levels.
- True overlaps use only the required subrows, with a gap between owners.

---

### Task 1: Parse TXT metadata

**Files:** `src/importexport/labels/resources/audacity-txt.js`, `src/importexport/labels/internal/labelscriptdata.h`, `src/importexport/labels/internal/labelscriptengine.cpp`, `src/importexport/labels/tests/labelscriptengine_tests.cpp`.

- [x] Add failing tests for legacy text, valid color/owner, and invalid metadata.
- [x] Run `labels_tests` focused cases and confirm expected failures.
- [x] Implement fourth-field parsing and typed validation.
- [x] Re-run focused cases.

### Task 2: Persist and render metadata

**Files:** `au3/libraries/au3-label-track/LabelTrack.h/.cpp`, `src/importexport/labels/internal/au3/au3labelsimporter.cpp`, `src/trackedit/dom/label.h`, `src/au3wrap/internal/domconverter.cpp`, `src/projectscene/view/tracksitemsview/tracklabelitem.cpp`, `src/trackedit/tests/au3labelsinteractions_tests.cpp`.

- [x] Add failing tests for XML and model metadata round trip.
- [x] Run focused tests and confirm expected failures.
- [x] Store per-label color and owner in AU3 XML, copy them, import them, and expose them in the view model.
- [x] Re-run focused tests.

### Task 3: Replace visual collision rows

**Files:** `src/projectscene/view/tracksitemsview/tracklabelslayoutmanager.h/.cpp`, `src/projectscene/view/tracksitemsview/tracklabelitem.h/.cpp`, `src/projectscene/qml/Audacity/ProjectScene/tracksitemsview/TrackLabelsContainer.qml`, `src/projectscene/tests/tracklabelslayoutmanager_tests.cpp`.

- [x] Add failing tests for tiny adjacent ranges, true overlap, owner groups, and point/range separation.
- [x] Run focused tests and confirm expected failures.
- [x] Assign rows from real intervals and owner/kind groups; compact subrows and leave a blank owner separator.
- [x] Re-run focused tests.

### Task 4: Integration verification

- [x] Build `audacity`, `labels_tests`, `trackedit_tests`, and `projectscene_tests`.
- [x] Run full CTest suite and `git diff --check`.
- [x] Inspect the final diff and report how to install with the existing script.
