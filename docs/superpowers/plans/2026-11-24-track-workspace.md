# Native track workspace implementation

**Goal:** Implement the approved track workspace in the real AGu editor.
**Architecture:** Existing Qt/QML views, project view state, selection and playback controllers, native WaveView renderer.
**Tech:** C++20, Qt 6.11, QML, GoogleTest, Ninja/CTest.
**Spec:** `docs/superpowers/specs/2026-11-24-track-workspace-design.md`.

## Global constraints

Preserve original colors, clip header drag/trim, label metadata, recording behavior, undo/redo, and user output. Never share the stereo split ratio with the waveform/spectrum split. Overview changes view only. Reuse the existing application modules and configured workbench build.

## Tasks

1. **Fixed toolbar:** curate PlaybackToolBarModel, simplify PlaybackToolBar QML, preserve menu access to import/audio settings. Integrate fixed docking in ProjectPage. Verify action wiring and menu tests.
2. **Selection playback:** add meaningful regression coverage in playbackcontroller_tests, then make normal Play use the selected interval once. Preserve label range playback and recording state. Inspect existing waveform click/drag and label behavior before changing it.
3. **Track controls and split:** revise TrackItem/WaveTrackItem spacing and volume visibility. Add independent multiview split state/API, integrate with TrackClipsContainer and ruler hit mapping. Exercise bounds/state with project view state tests.
4. **Overview and ruler:** reuse real WaveView and clip models for a project overview; add validated view-range navigation in TimelineContext with regression tests. Replace separate ruler loop gestures with selection controller gestures. Integrate timeline/overview geometry in TracksItemsView.
5. **Integration:** build configured workbench-debug target, run focused then full tests, inspect native runtime and gesture behavior. Review all diffs for label/clip/state regressions.

## Review focus

Incorrect edit target tracks, selection vs navigation side effects, playback region end handling, split hit coordinates, high-cost overview repaint, QML runtime errors, cross-track clip movement, and saved project compatibility.

## Implementation and validation, 2026-11-25

- Implemented the fixed toolbar and I-beam command, removed toolbar customization/audio setup and duplicate sidebar master meter, retained File import and Edit audio settings.
- Normal time selections and saved range labels play once even when a legacy loop is active. Playback, pause/resume and device teardown regression coverage retained.
- Added a real project waveform overview using the native cached painter, with viewport panning and edge zoom. View-range navigation is tested to leave selection untouched. Additional tracks share lanes with Lighten composition to preserve signals from opaque native caches.
- Added independent waveform/spectrogram split state and XML attachment storage, minimum pane sizes, per-track dual-view toggle, and always-visible thin track volume. Label tracks retain their own 44/72px height/collapse constraints.
- Refined dark chrome to neutral grays; original track hue now colors the waveform against a quiet canvas. Footer names editing target tracks.
- Fresh review found and resolved ruler Escape recreation, label default collapse, zero-height overview channels, opaque overview overlaps, and narrow-window toolbar allocation. Regression tests cover label layout and 13-track overview rendering, including opaque overlapping signals.
- Native window verification with generated stereo audio confirmed clip header drag and undo, waveform/spectrum divider drag, time drag selection, overview edge zoom preserving the interval, plain-click cancellation, range-label creation and restoration, and volume visibility at minimum height. Repository fixtures and unrelated generated assets are unchanged.
- Native startup uncovered and fixed an unsupported FlatToggleButton text property. Final restart is blocked before QML load in CoreAudio's AudioDeviceGetProperty during device probing. Final color/toolbar refinements have compiled and passed tests, but could not be visually rechecked after that environmental startup block.
- Final verification: `cmake --build build/workbench-debug --parallel 8` succeeded; `ctest --test-dir build/workbench-debug --output-on-failure -j 4` passed 40/40. `git diff --check` is clean.

## Layout and crash regression correction, 2026-11-26

- The earlier checks missed a restored-dock failure. Reproduced the toolbar overlapping navigation and the editor squeezed to a narrow strip. Restoring defaults helped only until the next launch. In the active `dockwindow_v2`, `resizable: false` caps both dimensions at their minimums, including the playback dock's 300px minimum width. Allow the dock to stretch while retaining its non-floating controls, and set its minimum height to the content thickness.
- The reported native crash followed import and undo: a delayed popup dereferenced an anchor with no window. Guard anchor lifetime with `QPointer`, refresh filters when the anchor changes windows, and give each macOS controller its own minimize observer with destructor cleanup.
- Five new regression tests failed against the original controller, including two native SIGSEGV reproductions. The corrected component suite passes 34 tests using Cocoa. Full validation: `QT_QPA_PLATFORM=cocoa ctest --test-dir build/workbench-debug --output-on-failure -j 4` passes 40/40. Running the native cases without a usable screen aborts, so the full verification used the native desktop environment.
- Rebuilt and launched the actual app with the existing workspace, without another layout reset. Verified full-width editor restoration, maximize/restore, import and undo, normal exit, then another launch and project reopen with the correct layout. The original audio remains untouched; the imported project was preserved as `/tmp/agu-crash-recovery.aup4`.
