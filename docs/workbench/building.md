# 构建与验证 Audacity Workbench

最新阶段状态、已知测试失败和继续工作步骤见 [`2026-11-11 交接记录`](2026-11-11-handoff.md)。

Workbench 是本仓库中的本地优先产品 Profile。它复用 Audacity 的音频编辑底座，但把 Cloud、MuseHub、UsageInfo、应用更新、在线语言更新及其他产品网络能力替换为 stub。Diagnostics 保留；Crashpad 是否启用由打包方决定。

## 环境要求

先满足仓库根目录 [`BUILDING.md`](../../BUILDING.md) 的要求。当前指定的 Qt 版本是 **Qt 6.11 Desktop**，并需要安装：

- Qt 5 Compatibility Module
- Qt Network Authorization
- Qt Shader Tools
- Qt State Machines

macOS 需要 macOS 13 或更新版本以及 Xcode 15 或更新版本。不要用 Qt 6.12 代替 Qt 6.11：当前源码会在 Qt 6.12 生成的 Muse Extensions QML AOT 代码中遇到 `Type` 名称冲突。

### 使用 aqtinstall 安装 Qt（无需 Qt 账户）

`aqtinstall` 可以把多个 Qt 版本并排安装，不会覆盖系统中已有的 Qt。以下示例把 Qt 6.11.3 安装到 `/Users/your-name/Qt/6.11.3/macos`：

```bash
python3 -m venv /private/tmp/audacity-aqt-venv
/private/tmp/audacity-aqt-venv/bin/python -m pip install aqtinstall==3.3.0
/private/tmp/audacity-aqt-venv/bin/aqt install-qt mac desktop 6.11.3 clang_64 \
  -O /Users/your-name/Qt \
  -m qt5compat qtnetworkauth qtshadertools qtscxml qttasktree
```

确认当前安装：

```bash
/Users/your-name/Qt/6.11.3/macos/bin/qmake -query QT_VERSION
```

第一次配置，或切换过 Qt 版本后，使用 `--fresh` 丢弃旧的 CMake 探测结果，并显式指定 Qt 路径：

```bash
cmake --preset workbench-debug --fresh \
  -DCMAKE_PREFIX_PATH=/Users/your-name/Qt/6.11.3/macos \
  -DQt6_DIR=/Users/your-name/Qt/6.11.3/macos/lib/cmake/Qt6
```

如果下载镜像不稳定，可给 `aqt install-qt` 增加 `-b https://mirrors.aliyun.com/qt/`，或分开补装失败的模块。Qt 版本之间不需要修改全局 shell 配置；为每个构建目录传入对应的 `CMAKE_PREFIX_PATH` 即可。

## 获取源码

新克隆时可直接获取所有子模块：

```bash
git clone --recurse-submodules https://github.com/audacity/audacity.git
cd audacity
```

已有工作区需要同步子模块时运行：

```bash
git submodule update --init --recursive
```

## 配置、构建和测试

完成上述 Qt 配置后，在仓库根目录运行：

```bash
cmake --build build/workbench-debug
ctest --test-dir build/workbench-debug --output-on-failure
```

产品 Profile 的配置策略还可以不依赖完整编译单独验证：

```bash
cmake -DTEST_CASE=default_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=workbench_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=cached_workbench_override -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=workbench_to_audacity -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=invalid_profile -P buildscripts/cmake/tests/product_profile_test.cmake
```

`workbench_to_audacity` 用例用于防止同一构建目录切回默认 Profile 后，Workbench 的模块关闭状态污染 Audacity 配置缓存。

构建成功后，macOS 应用位于：

```text
build/workbench-debug/src/app/audacity.app
```

可以直接运行可执行文件，方便在终端查看日志：

```bash
build/workbench-debug/src/app/audacity.app/Contents/MacOS/audacity
```

也可以用 Finder 方式启动：

```bash
open build/workbench-debug/src/app/audacity.app
```

默认 Audacity Profile 的对照构建命令为：

```bash
cmake --preset audacity-debug
cmake --build build/audacity-debug
```

如果 macOS 同时安装了 Command Line Tools 和完整 Xcode，且 CMake 选错 SDK，可在当前 shell 中设置 `DEVELOPER_DIR` 和 `SDKROOT` 指向同一套 Xcode，并在 `--fresh` 配置命令中用 `-DCMAKE_OSX_SYSROOT=/absolute/path/to/MacOSX.sdk` 选择同一套 SDK。不要把本机 SDK 路径写入共享 Preset。

## Crashpad 打包

开发 Preset 默认不启用 Crashpad。需要崩溃上传的打包方必须显式提供同一目标平台的 handler 和唯一的报告地址：

```bash
cmake --preset workbench-release \
  -DMUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT=ON \
  -DMUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH=/absolute/path/to/crashpad_handler \
  -DMUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL=https://crash.example.invalid/report
cmake --build build/workbench-release
cmake --install build/workbench-release
```

启用后，配置阶段会拒绝空的或不存在的 handler 路径。macOS 安装规则会把 handler 放到 `audacity.app/Contents/MacOS`。`MUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL` 编译进 Diagnostics；打包流程不得同时注入 Audio.com、更新、遥测、在线语言或扩展市场地址。

不需要上传时保持 Crashpad 关闭；Diagnostics 的本地日志和诊断能力仍会编译。崩溃报告偏好项只应在 Crashpad client 实际编译时显示。

## 发布前验收

在使用受支持的 Qt 6.11 完成全量构建后，至少执行：

```bash
ctest --test-dir build/workbench-debug --output-on-failure
```

并按设计文档第 8.3 节完成 GUI 核心流程。还需检查最终二进制的链接依赖和运行时网络流量，确认除配置的崩溃报告地址外没有其他产品服务请求。

## 2026-11-10 验证基线

使用 Qt 6.11.3、AppleClang 17 和 Xcode 26.2 SDK 对当前 Workbench 源码执行了以下检查：

- `cmake --build build/workbench-debug` 完整通过并生成 `audacity.app`。
- `ctest --test-dir build/workbench-debug --output-on-failure -R '^workbench_'`：11/11 通过。
- AppShell、Shared、Preferences、ProjectScene、Effects Base 和 App 六个受影响测试套件：6/6 通过。
- 全量 CTest：41 个测试程序中 40 个通过；唯一失败仍为 `au_project_tests`，内部是三个既有 Clip tempo persistence 用例。
- 默认 Audacity Profile 使用同一 Qt 6.11.3 重新配置并构建 `shared_tests`；`ProductProfile.*` 通过，确认默认 Profile 仍允许原有外链策略。

本轮新增的 Workbench 策略会移除实时效果帮助链接和 About 中的网站/隐私/GPL/依赖库链接；首次启动仍只包含三个本地配置页。后续 Diagnostics-only 收尾还会关闭 Workbench 的 TestFlow 构建、取消 `audacity://devtools` 页面注册和顶部 DevTools Tab，并把顶部 Diagnostics 菜单限制为诊断文件导出。一次空闲进程的 `lsof -nP -a -p <pid> -i` 快照没有发现网络套接字，但这只是时点检查，不等同于完整抓包。开发构建的 Crashpad client 为关闭状态。

最终 GUI 验收由验证者在最新构建上完成，至少检查：

1. `View > Show effects panel` 只显示实时效果说明，不显示 `Watch video`。
2. `About Audacity` 的 Audacity/Legal 两页没有可点击外链；Legal 页说明只有可选崩溃报告需要网络。
3. 顶部导航不显示 DevTools Tab；顶部 Diagnostics 菜单只保留诊断文件导出。
4. Help、Preferences、首次启动、保存/导出和插件管理中没有非诊断在线入口。
