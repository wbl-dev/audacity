# Label parser scripts

OnlyAudios imports TXT labels through a bundled JavaScript parser. To add a UTF-8 text format, save a script named `<suffix>.js` in the **LabelParsers** directory shown under **Preferences → Label file parsers**. For example, `ann.js` handles `.ann` files. The directory is scanned at startup; use **Tools → Rescan label parser scripts** or the Preferences button after changing files. Drag and drop and the import dialog use the scanned suffixes.

The script files are the only active configuration. The Preferences editor saves and removes files in that directory. A first-line `// @onlyaudios-label-parser {...}` comment is optional metadata for display name, multiple suffixes, and enabled state; scripts placed directly in the folder need no metadata. A `txt.js` file overrides the bundled TXT parser; remove it and rescan to restore the bundled parser. Previous `label-parsers.json` entries are copied once into the directory without replacing existing scripts. The JSON remains as a backup and is no longer read after migration.

`input` contains `{ fileName, extension, text }`. The extension is lowercase and has no leading dot. Return `{ trackName, labels }`, with at least one label. Times are nonnegative seconds. A point has one time; a range needs a strictly later end:

```js
function parse(input) {
    const labels = input.text.trim().split(/\r?\n/).map((line, index) => {
        const [time, title] = line.split("\t");
        const start = Number(time);
        if (!Number.isFinite(start) || start < 0) {
            throw new Error(`Line ${index + 1}: invalid time`);
        }
        return { type: "point", start, title: title || "" };
    });
    return { trackName: input.fileName, labels };
}
```

Each label needs a string `title`. A range is `{ type: "range", start, end, title }`. Either kind may include `lowFrequency` and `highFrequency` in Hz; each is independently optional. The host rejects invalid rows, reversed frequency bounds, empty output, and oversized input or output before creating a track. Errors include the file name and, for invalid returned labels, the row number. Throw an error with a line number for malformed source text.

Scripts run in a separate, time-limited process with no file, network, or project API. Sample files are limited to 4 MiB and script sources to 1 MiB. Scripts run once at import; the resulting labels are saved in the project. TXT accepts standard Audacity time and title rows and optional frequency continuation rows. Its undefined frequency value `-1` becomes an omitted bound in the script result.

Use lowercase suffix filenames. Invalid UTF-8, empty or oversized scripts, reserved audio or project suffixes, and conflicting suffixes are skipped; other valid scripts remain available. Rescan reports skipped files.
