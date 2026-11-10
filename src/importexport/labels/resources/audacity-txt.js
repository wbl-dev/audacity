// Audacity TXT label import. Times are seconds; -1 means an undefined frequency bound.
function parse(input) {
    const lines = input.text.split(/\r\n|\n|\r/);
    if (lines.length && lines[lines.length - 1] === "") {
        lines.pop();
    }
    const labels = [];
    const numberPattern = /^[+-]?(?:\d+(?:[.,]\d*)?|[.,]\d+)(?:[eE][+-]?\d+)?$/;
    function number(text, lineNumber, field, allowUndefined) {
        const value = text.trim();
        if (!numberPattern.test(value)) {
            throw new Error("Line " + lineNumber + ": invalid " + field);
        }
        const parsed = Number(value.replace(",", "."));
        if (!Number.isFinite(parsed) || (parsed < 0 && !(allowUndefined && parsed === -1))) {
            throw new Error("Line " + lineNumber + ": invalid " + field);
        }
        return parsed;
    }
    for (let index = 0; index < lines.length; index++) {
        const line = lines[index];
        const lineNumber = index + 1;
        if (line.startsWith("\\")) {
            throw new Error("Line " + lineNumber + ": frequency line has no label");
        }
        const fields = line.split("\t");
        if (fields.length < 2 || fields.length > 3) {
            throw new Error("Line " + lineNumber + ": expected start, end, and title");
        }
        const first = number(fields[0], lineNumber, "start time", false);
        let second = first;
        let title;
        if (fields.length === 3) {
            second = number(fields[1], lineNumber, "end time", false);
            title = fields[2];
        } else if (numberPattern.test(fields[1].trim())) {
            second = number(fields[1], lineNumber, "end time", false);
            title = "";
        } else {
            title = fields[1];
        }
        const start = Math.min(first, second);
        const end = Math.max(first, second);
        const label = start === end
            ? { type: "point", start, title }
            : { type: "range", start, end, title };

        if (index + 1 < lines.length && lines[index + 1].startsWith("\\")) {
            index++;
            const frequencyLine = index + 1;
            const frequencyFields = lines[index].split("\t");
            if (frequencyFields.length !== 3 || frequencyFields[0] !== "\\") {
                throw new Error("Line " + frequencyLine + ": invalid frequency continuation");
            }
            const low = number(frequencyFields[1], frequencyLine, "low frequency", true);
            const high = number(frequencyFields[2], frequencyLine, "high frequency", true);
            if (low >= 0 && high >= 0) {
                label.lowFrequency = Math.min(low, high);
                label.highFrequency = Math.max(low, high);
            } else {
                if (low >= 0) label.lowFrequency = low;
                if (high >= 0) label.highFrequency = high;
            }
        }
        labels.push(label);
    }
    return { trackName: input.fileName.replace(/\.[^.]+$/, ""), labels };
}
