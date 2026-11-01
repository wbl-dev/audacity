# 构建与验证 Audacity Workbench

Workbench 是本仓库中的本地优先产品 Profile。它复用 Audacity 的音频编辑底座，但把 Cloud、MuseHub、UsageInfo、应用更新、在线语言更新及其他产品网络能力替换为 stub。Diagnostics 保留；Crashpad 是否启用由打包方决定。

## 环境要求

先满足仓库根目录 [`BUILDING.md`](../../BUILDING.md) 的要求。当前指定的 Qt 版本是 **Qt 6.11 Desktop**，并需要安装：

- Qt 5 Compatibility Module
- Qt Network Authorization
- Qt Shader Tools
- Qt State Machines

macOS 需要 macOS 13 或更新版本以及 Xcode 15 或更新版本。不要用 Qt 6.12 代替 Qt 6.11：当前源码会在 Qt 6.12 生成的 Muse Extensions QML AOT 代码中遇到 `Type` 名称冲突。

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

在仓库根目录运行：

```bash
cmake --preset workbench-debug
cmake --build build/workbench-debug
ctest --test-dir build/workbench-debug --output-on-failure
```

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

如果 macOS 同时安装了 Command Line Tools 和完整 Xcode，且 CMake 选错 SDK，可在当前 shell 中设置 `DEVELOPER_DIR` 和 `SDKROOT` 指向同一套 Xcode 后重新配置。不要把本机 SDK 路径写入共享 Preset。

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
