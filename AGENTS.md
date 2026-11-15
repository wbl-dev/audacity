# Repository Guidelines

## Project Structure & Module Organization

AGu's application and feature modules live in `src/` (for example, `src/trackedit/`, `src/effects/`, and `src/app/`). Each module keeps its C++ implementation, QML views, and `tests/` together. The retained Audacity 3 engine lives in `au3/`; the vendored Muse framework lives in `muse/`. Shared assets and test flow scripts are in `share/`. Build and CI helpers are in `buildscripts/`; contributor documentation starts with `BUILDING.md` and `CONTRIBUTING.md`. Initialize the remaining dependency submodule before configuring the build.

## Build, Test, and Development Commands

- `git submodule update --init --recursive` fetches remaining dependency sources.
- `cmake --preset workbench-debug` configures a Ninja debug build with unit tests in `build/workbench-debug/`. Install the dependencies and Qt kit described in `BUILDING.md` first.
- `cmake --build build/workbench-debug` compiles the application and enabled tests.
- `ctest --test-dir build/workbench-debug --output-on-failure` runs the configured CTest suite and prints failed test output.

For interactive development, open the root `CMakeLists.txt` in Qt Creator with a suitable Qt kit, then build and run there.

## Coding Style & Naming Conventions

Follow `.editorconfig`: four spaces and a final newline for most files, two spaces for YAML, and tabs for Makefiles. Keep new C++ and QML names consistent with the surrounding module. Unit test files use names such as `macroserialization_tests.cpp`. Format QML with `qmlformat -i --semicolon-rule=essential -l unix <file.qml>`; CI checks QML and C++ style. Consult the coding standards linked from `CONTRIBUTING.md` for C++ details.

## Testing Guidelines

Most `src/<module>/tests/` suites use GoogleTest through CMake's `SetupGTest`. Add focused tests beside the affected module and register new test sources in its `tests/CMakeLists.txt`. Run CTest after building; use `ctest --test-dir build/workbench-debug -R <name> --output-on-failure` for a focused suite. No repository-wide numeric coverage minimum is documented.

## Commit & Pull Request Guidelines

Recent commits use short, purpose-focused subjects, often with a module prefix such as `macro:` or `effects:`. Make each commit describe its purpose and effects. Follow `.github/pull_request_template.md`: link the issue and explain the change and motivation. Keep extensive changes in reviewable commits. Check the AI-assistance boxes when applicable and follow `AI_POLICY.md`. Include test results and screenshots for visible UI changes.
