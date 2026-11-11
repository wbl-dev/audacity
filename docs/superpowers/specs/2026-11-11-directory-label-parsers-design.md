# Directory-backed label parsers

## Intent

Let users add a UTF-8 label format by saving one JavaScript file in a known folder. Discover parsers at startup and on demand from the Tools menu. Keep the bundled Audacity TXT parser on the same execution path.

## Behavior

- The user folder is `<userDataPath>/LabelParsers`. A lowercase file named `<suffix>.js` handles that file suffix; `ann.js` handles `.ann`. Optional metadata may claim additional suffixes. The folder is created on startup.
- Startup and **Tools → Rescan label parser scripts** scan filenames and load script source, without executing scripts. The importer and drag/drop use the resulting cached suffix list. Changed files take effect after rescan or restart. Manual rescan reports supported, added, and removed suffixes.
- The bundled `txt` script remains a fallback. A user `txt.js` overrides it. Unsupported or reserved suffixes, invalid UTF-8, empty scripts, and files over 1 MiB are skipped and reported on manual rescan. A valid parser remains usable if another file is bad.
- Preferences shows the parser folder, the discovered scripts, test-with-sample, and an Open folder control. Its existing editor may remain, but Save and Remove operate on the script files; no second profile store is maintained. A first-line metadata comment preserves display names, multiple suffixes, and enabled state for UI-created scripts. A plain `<suffix>.js` needs no metadata.
- On first launch after upgrade, enabled entries from `label-parsers.json` are copied to matching script files without replacing existing files. The JSON is retained as a backup but no longer consulted once migration is marked complete. Disabled entries remain in the backup.

## Constraints

Only UTF-8 text input is supported. Scripts still run only when importing, in the existing time-limited worker. Import output and project storage remain unchanged. This feature does not change the root `AGENTS.md`.
