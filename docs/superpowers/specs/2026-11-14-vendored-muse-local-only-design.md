# Vendor Muse for the local-only application

Date: 2026-11-14

## Intent

Make the Muse framework source used by OnlyAudios part of this repository, so
the local-only cleanup is reproducible without a private fork of Muse or a
submodule commit unavailable from the upstream Muse repository.

## Goals and constraints

- Replace the `muse` Git submodule with tracked source at the same path. Remove
  its gitlink and `.gitmodules` entry, retain `muse_deps`, and record the
  vendored starting revision `ad836ff9e0f0858c83e611e43c73091ee923cff1`.
- Delete Muse Cloud, Network, Update, and Learn implementations and stubs,
  including their QML modules, CMake options, and registrations.
- Remove online language-pack update and download APIs while preserving
  locally installed languages and language selection. This includes the
  service and configuration interfaces, language stubs, tests, and the server
  URL in the language configuration resource.
- Keep QML access to local `qrc:`, `file:`, and `data:` resources; reject remote
  URLs without a Muse Network service.
- Preserve local editing, projects, effects, plugins, labels, Diagnostics,
  and optional Crashpad. Leave the separate `muse_deps` dependency submodule.
- Keep the original Muse license and source notices.

## Approach

Two viable source layouts were considered. A private Muse fork would keep the
small Git submodule pointer but would require publishing a fork and updating
the submodule URL. Vendoring adds roughly 3,000 tracked files but makes this
branch self-contained. The user chose vendoring.

Convert the submodule without moving its path, then prune the four online
module families and simplify the two active cross-module dependencies:
LanguagesService's update path and UiEngine's network manager factory. The
local QML policy becomes the framework default, so the app-specific
network-configuration provider and duplicate factory can be removed.
Remove obsolete Learn, Cloud, and Update translations from the packed
`share/locale/audacity_*.ts` catalogs where they belong solely to deleted
source files.

## Verification

Fresh OnlyAudios configure and full build, full CTest, focused QML network-policy and
language tests, a source-reference audit, and an application link-edge audit.
Verify that a fresh checkout needs no `muse` submodule initialization.
