# Generic Label Track UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render point and range labels clearly at ordinary and dense zoom levels.

**Architecture:** Keep the current label model and overlap layout. Change the QML label primitives to use compact presentation when space is insufficient; preserve interaction signals and model selection behavior.

**Tech Stack:** Qt Quick/QML, C++/Qt 6, CMake, GoogleTest.

**Spec:** `docs/superpowers/specs/2026-11-12-generic-label-ui-design.md`

## Global Constraints

- Keep the stored label kinds and playback behavior unchanged.
- Do not add file-format or business-specific rows, colors, or fields to the core model.
- Preserve keyboard access and editing controls at dense zoom.

## Review Focus

- A short range still has a selectable bar and reveals its full title when selected.
- A point stays clickable when its title card is hidden.
- Selecting a compact label does not alter overlap levels or the stored times.
- Range resizing and double-click editing remain accessible.
- A label near the right edge does not lose its full-title reveal outside the track.

---

### Task 1: Label primitives and density

**Files:** `src/projectscene/qml/Audacity/ProjectScene/tracksitemsview/{LabelItem,LabelHeader,TrackLabelsContainer}.qml`

- [x] Record the existing `projectscene_tests` result and inspect current QML interactions.
- [x] Give ranges rounded bars, visible selected/hovered handles, and narrow-range title reveal; make point pins compact at low zoom and retain click targets.
- [x] Build `audacity` and run `projectscene_tests`; inspect QML compiler output.
- [x] Check `git diff --check` and review changed interaction handlers and label geometry.
