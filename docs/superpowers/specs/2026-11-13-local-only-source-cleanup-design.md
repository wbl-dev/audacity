# Local only source cleanup

Date: 2026-11-13

## Intent

Make the current Workbench behavior the only product. Remove disabled online
product code, its stub interfaces, cloud project metadata, and the dual
Audacity/Workbench profile. Future merges from upstream are not a constraint.

## Boundaries

Keep local project creation, opening, saving, importing, exporting, audio
editing, recording, effects, plugins, labels, diagnostics, and optional
Crashpad. Keep the generic libraries needed for those features.

Remove Audio.com and MuseHub account, cloud project and audio, publish/share,
usage telemetry, online updates, online content, and their UI, commands,
models, interfaces, implementation, and stubs. Do not retain cloud metadata
handling for old projects. A local file that happens to contain legacy cloud
metadata may be opened for its local audio, but the metadata is ignored.

## Approach

Use the existing Workbench behavior as the baseline, then remove each online
dependency from the application boundary inward. Replace cloud-aware project
save and open flows with local-only flows. Remove the now unreferenced service
trees and build switches. Verify with a fresh CMake configure, full build,
focused project tests, and full CTest. Audit source and build graph for
remaining product-service references.
