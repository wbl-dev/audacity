# Building AGu

AGu is the Audacity 4 based application in this repository. Its Muse framework source is included in `muse/`. Initialize the remaining dependency submodule before configuring:

```sh
git submodule update --init --recursive
```

Install Git, CMake, Ninja, a C++20 compiler, and a Qt 6 desktop kit. On macOS, use macOS 13 or later and Xcode 15 or later. The Qt kit needs Qt 5 Compatibility, Shader Tools, State Machines, and Network Authorization. Additional platform dependencies are listed in `buildscripts/ci/`.

## Configure and compile

```sh
cmake --preset workbench-debug
cmake --build build/workbench-debug --target agu
```

On macOS, run `build/workbench-debug/src/app/AGu.app/Contents/MacOS/AGu`. On Linux, run the `agu` executable in the build tree. The Windows executable is `AGu.exe`. To build tests and run them:

```sh
cmake --build build/workbench-debug
ctest --test-dir build/workbench-debug --output-on-failure
```

Open the root `CMakeLists.txt` in Qt Creator with a matching Qt kit for interactive development. The VS Code workspace is `.vscode/agu.code-workspace`.

## Install on macOS

After configuring `workbench-debug`, quit AGu and run `./buildscripts/tools/build_and_install_agu.sh` from the repository root. This builds the app and installs `/Applications/AGu.app`.
