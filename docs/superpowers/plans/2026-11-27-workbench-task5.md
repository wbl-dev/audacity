# Workbench Task 5 Implementation Plan

**Goal:** Give the workbench a consistent dark studio palette, compact track headers, aligned 44/40 px top rows, and grouped toolbar controls.

**Spec:** `docs/workbench/2026-11-27-ui-ux-optimization-spec.md`, C1–C3.

**Constraints:** Preserve track heights, split ratios, editing, playback selection, and label metadata. Build `build/workbench-debug`, run relevant and full CTest serially, format changed QML, and commit Task 5 alone.

## Steps

1. Add a failing test that a track header color follows its track color index and updates when the track changes. Expose the configured color on `TrackItem` and paint the 4 px swatch in its spacer.
2. Set dark theme selection and split guide colors. Give mute, solo, and expand controls distinct active colors and readable text. Make mouse selection use the swatch and subtle header fill while keeping keyboard focus visible.
3. Restyle colorful clip headers with a dark tint, pale text/icons, a 2 px track color strip, and a subdued colored border; preserve classic clip style.
4. Split `TracksTitleBar` into 44 px overview and 40 px timeline rows, keep add track and effects controls accessible, and remove the extra list top margin. Compact collapsed audio headers into two rows.
5. Group transport/timecode and edit buttons visually without changing the toolbar model or commands. Use semantic meter colors and a 120 ms falling level transition.
6. Run `qmlformat -i --semicolon-rule=essential -l unix` on touched QML, `git diff --check`, full build, focused CTest, and serial full CTest. Inspect UI if possible without disturbing an unsaved project, then commit Task 5.
