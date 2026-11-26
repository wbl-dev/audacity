# Track workspace, first native implementation

This first native implementation brings a fixed, restrained editing workspace to AGu, informed by GarageBand track controls and Audition time selection and dual waveform/spectrogram views.

## Intent and approach

Build on the existing Qt/QML editor, native audio engine, selection controller and clip movement. Keep track waveform colors. Use restrained neutral chrome, generous track controls, thin sliders and a dark editing canvas. The HTML prototype supplies visual direction, not application state or engine code.

The alternatives were further mockup iterations or an independent replacement editor. Native integration is the chosen implementation approach because it retains real editing, effects, undo, recording and saved projects.

## Required behavior

- Curated fixed transport/edit toolbar; remove toolbar customization, import/audio setup and independent loop controls. Import stays in File; audio setup stays in settings/menu.
- Project-wide waveform overview with a draggable viewport and draggable zoom edges. Overview navigation does not change the audio selection.
- One project time interval, independently selected editing tracks. Waveform dragging selects time, clip headers move clips, ruler dragging selects across tracks. A plain click clears a range and seeks; Shift extends; Escape cancels. Toolbar actions preserve selection.
- Standard Play plays a selected interval once. With no interval it plays from the cursor. Do not require a separate loop region.
- Labels use this selection: ranges from intervals, points from cursors; selecting a saved range restores it. New selections do not mutate saved labels. Owner metadata remains grouping metadata.
- Roomy track header, mute/solo and a long thin volume slider visible when collapsed. Inline waveform above spectrogram, with a draggable divider rather than a ratio control. Keep stereo-channel and waveform/spectrum divider state separate.
- Restore the approved expansion interaction: opening a track's inline spectrogram closes the previous expanded track. Remember each track's single-view and dual-view heights independently; closing restores the single-view height, reopening restores the edited dual-view height and divider ratio. First expansion adds 240px, as in the approved prototype. Keep these dimensions with the project across reopening. Selecting audio alone does not change its view mode.
- Preserve native clip header horizontal/cross-track movement, trim and undo/redo. Do not substitute mock clips.

## Builder decisions and limits

Use existing native waveform/spectrum renderers. Default tool is time selection (I-beam); arbitrary spectral paint masks are outside this first version.

## Acceptance

Build the application, run relevant controller/model tests and the configured CTest suite. Compare any failures with the baseline. Launch the native application and inspect toolbar, collapsed/expanded controls, overview and actual selection/clip movement. Report any unverified interactions plainly.
