# Label owner metadata and stable rows

The built-in TXT label parser accepts its existing two- or three-column lines. A fourth tab-separated field may start with `//` and contain comma-separated `key:value` entries. The first version recognizes `color` (`#RRGGBB`) and `owner` (nonempty text without a comma). Missing owner means the default group; missing color inherits the label track color. Invalid or duplicate keys fail import with a line number. Frequency continuation lines remain valid.

Imported color and owner survive project save/reopen and label duplication. A label's visible color uses its own color when present. TXT export preserves metadata on labels that have it, while ordinary labels remain standard three-column lines.

Within one label track, owners form stable groups, with an empty default owner first and named owners ordered alphabetically. Each owner has range rows followed by point rows. Range intervals use half-open time boundaries: `[start, end)`. Adjacent ranges share a row; actually overlapping ranges use the minimum number of compact subrows. Points with different timestamps share a row even when their cards or hit targets visually overlap; identical timestamps use compact subrows. A full blank row separates owners. Zoom and minimum clickable width never affect row assignment.
