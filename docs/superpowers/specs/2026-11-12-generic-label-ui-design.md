# Generic label track UI

## Intent

Show the two core label kinds clearly at ordinary and dense timeline zoom levels. The UI must work for any imported or manually created label, independent of a file format or business category.

## Behavior

- A range is a 12 px high rounded horizontal bar covering its time interval at every zoom level. Its title appears inside only when the bar has enough room. A selected or hovered narrow range reveals the full title without moving other labels.
- A point is a visible pin at its exact time with a small neutral title card beside it. At dense zoom, the pin remains and the card appears only on selection or hover. Its clickable area stays larger than the pin. Hide the card when its pin scrolls out of view.
- Existing automatic overlap levels remain; compact marks use their compact visual width so dense labels do not create empty layout rows.
- Selection, single-click activation, double-click title editing, range edge resizing, keyboard navigation, and playback behavior stay available. Range edge handles are visible when selected or hovered.
- Appearance is derived from the existing track color. Business-specific colors, rows, grouping, and frequency overlays are outside this UI pass.

## Verification

Build the app and run `projectscene_tests`. Check a dense range sample and point labels in the running app. The mockup is in `docs/design-mockups/label-track-primitives-preview.html`.
