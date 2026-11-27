# Workbench Task 3 implementation plan

## Scope

Implement B1, B2, and B4 items 5–7 from the workbench UI/UX specification. Keep the existing time selection, clip editing, and project persistence contracts intact. Submit the batch as one reviewable commit after verification.

## Steps

1. Add focused `projectscene_tests` for narrow overview hit testing, slider bounds and zero gain snapping, and toolbar toggle independence. Observe the relevant failures before changing production behavior.
2. Remove the track header's ordinary-click request to select all audio data. Keep target-track selection, modifier selection, and explicit selection commands.
3. Update `OverviewNavigator.qml`: make narrow viewport centers draggable; project the time selection; toggle fit and the prior local range on double click; support wheel zoom and horizontal pan; align the strip with the timeline when vertical rulers are present. Keep overview gestures from changing the edit selection.
4. Update the track `VolumeSlider.qml`: draw the zero gain marker, snap within ±1.2 dB outside Shift mode, preserve the offset when grabbing the handle, and clamp fine movement.
5. Make the time-selection toolbar button independent of Clip Gain Automation. Correct local import coordinates using the full header height and the canvas left inset.
6. Run `qmlformat` for touched QML, `git diff --check`, `cmake --build build/workbench-debug --parallel 8`, focused CTest, and the full CTest suite without concurrent full runs. Inspect the built app's project view and commit only this Task 3 batch.
