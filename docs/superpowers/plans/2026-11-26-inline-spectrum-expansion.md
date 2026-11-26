# Restore Inline Spectrum Expansion

> **For agentic workers:** Use superpowers:executing-plans to implement this focused regression correction.

**Goal:** Restore the expansion interaction confirmed by the user on 2026-11-26.
**Architecture:** Own expansion transitions in ProjectViewState so menu and header controls agree. Extend the existing track height attachment to remember both heights; preserve the separate divider ratio and native renderers.
**Tech Stack:** C++20, Qt/QML, GoogleTest, CMake/Ninja.
**Spec:** `docs/superpowers/specs/2026-11-24-track-workspace-design.md`.

## Constraints and review focus

One expanded track at a time; single/dual heights independent; first expansion adds 240px; dual-view minimum 280px. Preserve waveform colors, stereo split, selection, clip movement, compact track volume, menu view modes, project compatibility and the fixed dock layout. Existing projects without the new heights use their saved current height and sensible defaults for the other mode.

## Task 1: Restore expansion state and verify the native UI

- [x] Add failing ProjectViewState tests for round-trip height restoration, switching expanded tracks, restored attachment state, and view transitions/minimum heights.
- [x] Extend `internal/au3/trackheightattachment.{h,cpp}` with single/dual heights, XML serialization and copy support. Test serialization with the real attachment.
- [x] Update `internal/projectviewstate.{h,cpp}` to restore/store heights on view changes and close a previously expanded track. Remove the 320px override from `view/common/trackviewstatemodel.cpp`; the header expansion button selects its track.
- [x] Build and run focused tests, then the full configured CTest suite under Cocoa. Inspect actual expansion, divider drag, closing/reopening, switching tracks and project reopening in the native app.
- [x] Review the final diff, record validation, and leave the corrected app available to the user.

## Validation, 2026-11-26

- Four new behavioral cases failed before the implementation and passed afterward. Review found that duplication copied expanded state; a real native track duplicate plus insertion notification reproduced this, then passed after insertion normalization. The original stays expanded and the copy retains its remembered dimensions for later expansion.
- `ProjectViewStateTests.*`: 18/18 passed, including seven new cases. Native metadata writer/parser round-trip covers the two remembered heights and divider ratio; attachment copying and legacy projects without the new attributes are covered.
- A preliminary Save As test could not copy the existing test fixture because its sample-block database fails SQLite quick_check. Replaced that test with metadata XML round-trip and verified actual file save/reopen separately in the real application. No fixture changes retained.
- `cmake --build build/workbench-debug --parallel 8` succeeded. `QT_QPA_PLATFORM=cocoa ctest --test-dir build/workbench-debug --output-on-failure -j 4`: 40/40 passed. `git diff --check` clean.
- In the native app, used `/tmp/agu-spectrum-interaction.aup4`, a copy of the preserved recovery project. Resized single view to approximately 133px, expanded view to 412px, and dragged the divider to approximately 0.34. Collapse/reopen and save/quit/relaunch/reopen preserved the independent heights and ratio. Importing a second track and opening each in turn confirmed that only one remains expanded. The corrected app is left open on the test copy; the user's source audio is unchanged.
- Focused review and follow-up review found no remaining actionable issues after the duplication correction.
