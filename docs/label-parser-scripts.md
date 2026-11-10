# Label parser scripts

OnlyAudios imports TXT labels through a bundled JavaScript parser. To import another UTF-8 text format, open **Preferences → Label file parsers**, add a parser, enter its file suffix (for example `ann`), and write a `parse(input)` function. Test it with a sample file before saving. The built-in TXT script is read-only; duplicate it to adapt its behavior. A user script can claim `srt` or `vtt`, but neither format has a bundled importer.

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
