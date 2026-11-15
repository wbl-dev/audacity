# OnlyAudios

OnlyAudios is a local Audacity 4 based audio editor. The application and its
feature modules are in `src/`. The retained engine code in `au3/libraries/` and
`au3/modules/import-export/` supports recording, editing, effects, and local
project and media import/export. The Audacity 3 application source is not part
of this repository.

The application ships a single Modern workspace. Existing local Modern layout
settings remain usable; older workspace files are ignored. Build instructions
are in [BUILDING.md](BUILDING.md).

## License

The code is open source under GPLv3, with third party and VST3 components under
their respective licenses. See [LICENSE.txt](LICENSE.txt) and the notices in
individual source directories.
