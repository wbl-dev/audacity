# Workbench Task 4 Implementation Plan

**Goal:** Make inline spectrum transitions smooth while keeping manual resizing immediate, preserving scroll position, and improving resize and reorder controls.

**Spec:** `docs/workbench/2026-11-27-ui-ux-optimization-spec.md`, B3 and B4 items 1–4.

**Constraints:** Preserve single and multi view heights, stereo and multi view ratios, playback selection behavior, clip editing, and label metadata. Do not run two complete CTest processes concurrently. Format changed QML with `qmlformat -i --semicolon-rule=essential -l unix`.

## Steps

1. Add `ProjectViewStateTests` for above-track collapse anchoring, below-track non-anchoring, and an ephemeral interactive resize flag. Observe failures before implementing.
2. Add the 170 ms OutCubic height behavior to the track header, canvas, and vertical ruler delegates; use the shared resize flag to disable it during direct border dragging. Animate the scroll compensation with the same curve.
3. Add an 8 px track bottom hit area with a visible hover line and double-click reset. Audio tracks restore the default height for their current view; label tracks fit the layout manager's row count.
4. Add a 10 px waveform/spectrogram divider hit area with a 32 × 4 px pill grip, a 120 ms highlight, and continuous direct ratio changes. Delay track reorder until the pointer moves more than 6 px. Add 110 ms button and header color transitions.
5. Add a `ProjectOverviewTests` case proving a view-only track notification does not repaint the overview. Cache track color indices in `ProjectOverview` and refresh only when color or clip data changes.
6. Format touched QML, run `git diff --check`, build `build/workbench-debug`, run `ProjectViewStateTests.*`, `projectscene_tests`, and complete CTest serially if needed. Inspect the built UI without disrupting the open project, then commit Task 4 alone.
