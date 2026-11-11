# Workbench Diagnostics-Only 收尾设计

日期：2026-11-10
状态：已批准（基于功能删减评估后的执行指令）

## 意图

Workbench 已关闭产品在线服务，但调试构建仍包含完整 DevTools 和 TestFlow。最后一轮删减要把开发入口收缩到崩溃诊断所需的最小集合，避免将 UI Gallery、Interactive、Profiler、TestFlow 等通用开发工具作为工作台产品能力继续暴露。

## 保留边界

- Diagnostics 真实模块、本地日志和诊断文件导出。
- 工程自动保存、崩溃恢复和会话恢复。
- 打包方显式配置时启用的 Crashpad handler 和唯一报告地址。
- 默认 Audacity Profile 的完整 DevTools、TestFlow 和诊断菜单行为。

## Workbench 中移除

- TestFlow 模块、运行器和脚本资源。
- 顶部导航中的 DevTools Tab，以及 `audacity://devtools` 的 Workbench 页面注册。
- Diagnostics 菜单中的 Paths、Graphics Info、Profiler、Actions、Accessibility、Extensions API Dump 和 TestFlow 入口。

Workbench 不再提供 DevTools 工作区页面。必要的诊断能力放在窗口顶部菜单栏的 Diagnostics 菜单中；当前只保留诊断文件导出。故意触发崩溃的 Crash handler 测试按钮不作为产品入口保留，Crashpad handler 本身仍由 Diagnostics 模块在后台管理。默认 Audacity 继续保留原 DevTools Tab 和页面。

## 方案比较

### 方案 A：只隐藏 QML 菜单项

改动最少，但 TestFlow 仍会编译进调试构建，Diagnostics 菜单仍暴露 Profiler 等入口，不能满足“只保留崩溃诊断”。不采用。

### 方案 B：Profile 关闭 TestFlow，Workbench 移除 DevTools Tab，并保留顶部 Diagnostics 菜单

保持默认 Audacity 不变，同时从 Workbench 构建、页面注册和顶部导航三端收口。必要的本地诊断出口继续存在于顶部菜单，不再占用工作区 Tab。改动集中、容易测试，也不物理删除上游源码。采用。

### 方案 C：物理删除 DevTools/TestFlow 源码

会破坏默认 Audacity Profile并增加跟进上游的冲突，不符合当前可逆 Profile 策略。不采用。

## 验收标准

1. Workbench 的有效 CMake 配置中 `MUSE_MODULE_TESTFLOW=OFF`；默认 Audacity 仍为 `ON`。
2. Workbench Diagnostics 菜单只保留 `DIAGNOSTICS_SAVE_FILES_COMMAND`。
3. Workbench 不注册 `audacity://devtools`，即使开启 developer mode 也不显示 DevTools 顶部 Tab。
4. Workbench 不构建 TestFlow 静态库、脚本资源或应用运行器对象。
5. Diagnostics、本地工程恢复及可选 Crashpad 配置不受影响。
6. 默认 Audacity 的完整开发工具行为保持不变。
7. Qt 6.11.3 Workbench 完整构建和全部 `workbench_*` 测试通过。

## 非目标

- 不物理删除默认 Audacity 仍需使用的 DevTools、TestFlow 或 Cloud 源码。
- 不启用或配置真实 Crashpad 上传地址。
- 不修改音频编辑、插件、轨道或算法能力。
- 不处理既有 Clip tempo persistence 测试失败。
