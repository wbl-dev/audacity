# Scriptable label import and playback design

## Intent and scope

Make label files work through the normal file drop and import paths, let users bind a file suffix to an in-app JavaScript parser, and make timeline labels useful for immediate audition. The bundled TXT parser uses the same JavaScript contract as user scripts, so new formats extend one mechanism.

## Goals, constraints, and boundaries

- A supported label file dropped into a project creates one label track. The general project import command, Labels Editor import command, and drop use the same format detection and parser path.
- Only TXT has a bundled parser in the first release. Its JavaScript must preserve the existing Audacity TXT grammar for valid files, including point and range rows, compatible numeric forms, and optional spectral-frequency continuation lines. A malformed row fails the file with its line number, rather than importing only the other rows. SRT and WebVTT are removed from the built-in import filters and suffix registry; users may add either later with their own JavaScript profile.
- Users can add, edit, enable, disable, and remove suffix-to-script bindings in Preferences, and test a script against a chosen sample file before saving it.
- A parser explicitly returns `type: "point"` with one timestamp or `type: "range"` with `start < end`. The label model and saved project retain that type. Invalid output creates no track and produces a useful error with the row index.
- A normal click on an interval label selects its time span across audio tracks and immediately plays that span once. A normal click on a point label moves the playhead there and starts ordinary playback. Dragging, resizing, modifier-assisted multiselect, and double-click title editing do not start playback.
- Give point labels a distinct visible marker with a minimum 16-pixel hit target at any zoom; keep range labels as spans with edge handles. Preserve existing label editing/export behavior. Scripts run only on import; projects store the resulting labels, not a live script dependency.
- Retain the optional frequency bounds already stored with labels and exposed by the TXT format. Spectrogram overlay, creation, and editing of frequency annotations are a separate follow-up design.
- First release reads UTF-8 text files. Binary input, remote URLs, writing project data from scripts, and automatic reimport when the source changes are outside scope.

## Approaches considered

1. **Embedded JavaScript — selected.** Qt already provides `QJSEngine`; the parser can receive file text and return plain data without exposing project objects. It is cross-platform and needs no user-installed interpreter.
2. **External Python or command-line parser.** This supports existing Python libraries but needs interpreter discovery, environment management, subprocess diagnostics, and packaging rules.
3. **Both engines initially.** This gives more choice but duplicates configuration, limits, and testing before the import workflow is proven.

## Script contract and configuration

Each profile has a display name, one or more case-insensitive suffixes, an enabled flag, and JavaScript source stored in the user's application data. The bundled TXT profile is a read-only JavaScript resource that users can inspect and duplicate. For example, `.ann` is normalized to `ann`. An explicit user profile may override a built-in *label* suffix only after a conflict warning; one enabled parser owns each suffix. Project and audio suffixes are reserved and cannot be assigned to a label script. If a parser rejects a file, report the failure rather than silently trying an audio importer.

The host calls `parse(input)` with `{ fileName, extension, text }`. The function returns `{ trackName, labels }`, where each label is `{ type: "point", start, title }` or `{ type: "range", start, end, title }`; times are seconds. A label may also have `lowFrequency` and `highFrequency` in Hz. Each frequency bound is independently optional, matching existing Audacity data. Custom scripts can omit both. Example:

```js
function parse(input) {
    return {
        trackName: input.fileName,
        labels: input.text.trim().split(/\r?\n/).map(line => {
            const [time, title] = line.split("\t");
            return { type: "point", start: Number(time), title: title ?? "" };
        })
    };
}
```

The host validates the explicit type, finite nonnegative times, `end > start` for ranges, string titles, optional finite nonnegative frequency bounds (ordered when both exist), output count, and total output size. An empty `labels` array is an error. The bundled TXT script maps each legacy `-1` undefined frequency bound to an omitted field; persistence maps an omitted bound back to Audacity's undefined sentinel. The script receives no filesystem, network, or project API. Run `QJSEngine` in a dedicated helper process with bounded input/output and time, a platform memory limit, and termination on timeout. Process isolation prevents a script crash or hang from taking down the editor. Show script exceptions and validation errors in the test preview and import result. Text decoding errors are reported rather than replaced silently.

## Integration and interaction

Introduce one label-parser registry behind `ILabelsImporter`. The bundled TXT JavaScript and user JavaScript profiles register their suffixes there; every format uses this contract. Remove the existing SRT and WebVTT built-in routes and their file-dialog filters. The import layer asks the registry to classify a file before audio probing. `DropController` then handles label files as label tracks instead of reserving wave tracks or calling `importIntoTrack`, which is audio-specific. Parse and validate each file before creating its track; a failed file never leaves a partial track. In a multi-file import, successful files remain imported, all failures are reported, and the successful batch gets one undoable import state.

For a click on a timeline label, defer the play action until release so drag and resize can cancel it. Reuse the existing time-selection and `play-selection` playback command for intervals, which already plays an exact range even when a loop region is active. Keep double-click editing and keyboard navigation intact. A point label has no bounded playback region, so it seeks and plays from that point until the user stops.

Add an explicit `Point`/`Range` kind to both the AU3 label structure and the track-edit label model, and persist it as a label XML attribute. Existing projects without the attribute infer the kind once from equal versus unequal times on load. For compatibility with older readers, write equal `t` and `t1` for a point, but the new code uses the explicit kind for behavior and rendering. Copy, undo, import, export, and edit paths must retain or intentionally convert the kind. Prevent a range from collapsing to zero duration during edge drag; offer an explicit conversion action instead. Render a point as a marker above its stalk, with a 16-pixel minimum pointer target and a keyboard-accessible name; render ranges as spans with their existing handles.

Expose the kind in the Labels Editor. A point has one editable time cell; moving it updates both legacy stored endpoints, and its end-time cell is disabled. A range has editable start and end cells, with `end > start` required for direct edits and edge dragging. No edit implicitly changes kind. Explicit Range→Point conversion keeps the start time; Point→Range sets the end to start plus one second, which the user may then edit. Creating a label from a collapsed selection makes a point; a nonempty selection makes a range. Each conversion is one undoable edit.

Keep legacy frequency bounds as optional label data through script output, project save/load, and TXT round trips. Do not couple the parser or timeline-label UI to the spectrogram module in this release. The follow-up spectrogram use case includes **F0 samples** (`time`, `frequencyHz`, optionally voiced/confidence) and **MIDI notes** (`start`, `end`, `pitch`), both overlaid on a target audio track/channel. The current separate label track does not identify that target. These are ordered data series, not thousands of independent labels; equal low/high frequency bounds are not an adequate representation. The follow-up design should define spectral series storage, audio-track association, and a separate overlay that shares the spectrogram's time/frequency coordinate conversion. It should query only the visible time window and batch or decimate drawing rather than create one QML item per sample. This does not change the first release's timeline `Point`/`Range` scope.

## Verification

Add parser-contract tests for points, intervals, invalid or missing types, zero-duration ranges, malformed data, exceptions, suffix conflicts, UTF-8 errors, timeouts, and no partial track on failure. Test old projects without a kind attribute, new project save/reopen, kind preservation through copy/undo, point hit targets, direct time edits, and both explicit conversions. Add drop routing tests for mixed audio and label files and menu-import parity. Test click versus drag, resize, double-click, modifier selection, loop playback, and point playback. Use TXT fixtures with point/range rows and frequency continuation lines, including independently undefined frequency bounds, plus a custom `.ann` fixture; verify frequency bounds survive import, project save/reopen, and export. Verify SRT and WebVTT are absent from built-in filters but can be assigned to user scripts. Manual QA should cover the Preferences script editor and visible error messages on each supported desktop platform.
