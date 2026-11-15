# Audacity 4 only, Modern workspace only

Date: 2026-11-14
Status: approved by user; retain existing Audacity 4 features as the Modern shell

## Intent

Continue the independent, local OnlyAudios source cleanup. Remove Audacity 3
application source and resources that Audacity 4 does not use. Ship Modern as
the sole workspace, with no Classic, Music, custom workspace, or workspace
selection UI. Preserve the Audacity 4 audio editor and its required engine.
This replaces the earlier slim-profile design's decision to retain old source
for possible upstream merges.

## Constraints and current evidence

- Audacity 4 lives in `src/`, but `src/au3wrap` builds and links
  `au3/libraries` and `au3/modules/import-export`. The existing
  `workbench-debug` compile database contains 289 library and 67 module source
  files under `au3/`; it contains no source under `au3/src/`. The `au3/`
  directory cannot be removed wholesale.
- `au3/libraries/au3-builtin-effects/EqualizationCurves.cpp` loads
  `EQDefaultCurves.xml` at runtime. That resource must be retained and installed
  in the location used by Audacity 4 before old resource trees are deleted.
- `au3/locale` is referenced by legacy translation migration scripts but not
  by the current build. Those migration tools are obsolete once the catalogs
  are removed.
- The macOS compile database proves only its own configuration. Linux LV2,
  Windows, and other AU4 conditional libraries and import/export modules must
  remain when referenced by a supported AU4 option or platform path.
- Workspace configuration currently packages Classic, Music, and Modern.
  First launch offers Modern and Classic; the project toolbar permits selecting
  and creating workspaces; some preferences depend on workspace names.
- A pre-existing edit to `buildscripts/tools/build_and_install_onlyaudios.sh`
  and untracked `output/` are unrelated and must be preserved.

## Approaches

1. Keep the Audacity 3 tree and hide old UI. Small change, but old source and
   configuration remain. This contradicts the request.
2. Remove all of `au3/` and rewrite the audio engine. This would remove
   required project, track, effect, and import/export implementations.
3. **Chosen:** delete Audacity 3 files outside the Audacity 4 build and runtime
   closure, retain only required engine code and assets, and make Modern the
   single workspace in both resources and behavior. The user chose physical
   deletion and a single Modern workspace.

## Source boundary

Remove `au3/src`, old platform installer and package trees, old tests,
documentation generation, scripts, legacy locales, Nyquist translation source,
unused modules, and libraries excluded from the Audacity 4 CMake library list.
For active import/export and library directories, remove old application
entry points, preference pages, and other files only after checking includes,
CMake sources, generated resources, and runtime file lookup. Keep `au3defs.cmake`,
the necessary `au3/include` declarations, active `au3/libraries`, active
`au3/modules/import-export`, all conditional AU4 platform and plugin code, and
required assets. Do not remove Audacity 4
support for opening old local projects merely because it handles an old file
format.

Remove stale scripts, documents, and CI entries whose only purpose is the
deleted Audacity 3 app or its PO translation migration. Update current
repository documentation so it describes the actual Audacity 4 layout and
build. No deleted file may remain in active build, installation, or packaging
references.

## Modern-only behavior

- Package only `Modern.mws`; remove Classic and Music `.mws` files and their
  names from `workspaces.cfg` and shipped translations.
- Remove first-launch workspace selection, the project workspace toolbar,
  View > Workspaces menu, workspace management UI and commands, and associated
  registration and QML resources. Remove the obsolete toolbar from the
  `Modern.mws` dock layout. Preserve the workspace data service needed for
  docking and UI layout persistence.
- The workspace loader should load Modern only, including a user-edited
  `Modern.mws` when present. Ignore old Classic, Music, and custom workspace
  files without deleting the user's files. A persisted current workspace name
  other than Modern must fall back to Modern and be rewritten in settings.
- Replace workspace-dependent options in tempo detection and stereo-height
  preferences with global Always/Never choices. Migrate an existing
  WORKSPACE_DEPENDENT preference to Always exactly when its saved workspace
  list contains Modern; otherwise migrate it to Never. For a fresh install,
  tempo detection defaults to Never (the former Music-only default was off in
  Modern), and asymmetric stereo heights keeps its existing Never default.
  Retain ordinary music editing and tempo detection; the deleted Music
  workspace must not gate them.
- Remove obsolete workspace title translation helpers and QML list controls.
  Generic Muse workspace internals may be pared down if the Audacity 4 runtime
  and tests need no remaining multi-workspace API.

## Verification

1. Audit deleted paths against a fresh CMake configure and generated compile,
   resource, and install rules; ensure no path references deleted files.
2. Build the complete Audacity 4 app and tests from `workbench-debug`, then run
   CTest. Check both local engine linking and runtime resource packaging.
3. Add focused coverage for Modern-only configuration and migration of a
   persisted Classic/Music/custom workspace name. Exercise preference fallback.
4. Check the built app resources contain Modern but not Classic or Music and
   that no workspace selection path remains in the QML tree.
5. Report any runtime checks that could not be performed in the environment.
