# 构建本地版 Audacity Workbench

本仓库只提供本地版 Workbench。工程、音频编辑、录放、效果器、插件、标签、Diagnostics 均在源码中保留；Audio.com 云工程、MuseHub、遥测、在线更新等产品服务及其应用模块已经移除。可选 Crashpad 仍可由打包方配置。

## 准备

按仓库根目录 [BUILDING.md](../../BUILDING.md) 安装编译器、Ninja、Qt 6.11 和所需 Qt 模块，并初始化依赖子模块：

```bash
git submodule update --init --recursive
```

`muse/` 框架源码已直接纳入本仓库，基于 Muse 修订版 `ad836ff9e0f0858c83e611e43c73091ee923cff1` 清理在线模块；`muse_deps/` 仍是独立的依赖子模块。

本机验证使用 Qt 6.11.3。首次配置或更换 Qt 后，使用 `--fresh` 清理旧缓存：

```bash
cmake --preset workbench-debug --fresh \
  -DMUSE_ENABLE_UNIT_TESTS=ON \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.3/macos \
  -DQt6_DIR=/path/to/Qt/6.11.3/macos/lib/cmake/Qt6
cmake --build build/workbench-debug --parallel 8
ctest --test-dir build/workbench-debug --output-on-failure
```

macOS 如需指定 SDK，给 CMake 进程设置匹配的 `DEVELOPER_DIR`、`SDKROOT`，并传入 `-DCMAKE_OSX_SYSROOT=/path/to/MacOSX.sdk`。不要把本机 SDK 路径写进共享 Preset。

调试应用位于 `build/workbench-debug/src/app/OnlyAudios.app`。其他可用 Preset 为 `workbench-asan` 和 `workbench-release`。

## Crashpad

开发构建默认不启用 Crashpad。需要崩溃报告上传时，打包方必须提供目标平台的 handler 和报告地址：

```bash
cmake --preset workbench-release \
  -DMUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT=ON \
  -DMUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH=/absolute/path/to/crashpad_handler \
  -DMUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL=https://crash.example.invalid/report
```

启用后，崩溃报告设置会显示在偏好页面。不开启时仍保留本地 Diagnostics 日志能力。
