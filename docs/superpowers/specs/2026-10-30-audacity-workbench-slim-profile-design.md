# Audacity 音频开发工作台：首轮功能精简设计

日期：2026-10-30
状态：已批准
范围：只定义第一阶段的功能精简，不包含算法接入、轨道模型扩展或既有交互优化。

## 1. 意图

以 Audacity 4 为底座，形成一个本地优先、界面干净、能够持续跟进上游的“音频开发与验证工作台”基础版本。

第一阶段的目标不是尽可能删除代码，而是建立一个可重复构建和验证的工作台产品配置：保留音频时间轴、编辑、录放、频谱、标注、插件和工程能力，关闭与 Audio.com、MuseHub、发布、遥测及推广相关的产品功能，并保证后续添加算法与结果视图时不需要重新恢复被误删的基础设施。

## 2. 背景判断

Audacity 4 采用 Qt/QML 与 Muse Framework 构建新界面，通过 `au3` 与 `au3wrap` 继续复用成熟的音频工程、录放和编辑能力。

代码已经为 Cloud、MuseHub、UsageInfo 和部分插件宿主提供模块开关及 stub。这说明首轮精简最适合采用“产品 Profile + 模块关闭 + UI 收口”，而不是先物理删除源码。

当前轨道领域模型仍只明确支持 Mono、Stereo 和 Label。分析结果轨、F0、噪声区间等属于后续架构改造，不在本阶段加入。

## 3. 目标

1. 增加一个明确的工作台构建 Profile，同时保留原始 Audacity 构建方式。
2. 工作台 Profile 默认不构建真实 Cloud、MuseHub 和 UsageInfo 模块，使用现有 stub 满足暂时存在的接口依赖。
3. 移除工作台 UI 中的登录、云工程、发布、分享、获取效果器推广和商业欢迎内容。
4. 启动后直接进入本地工程工作流，不要求账号、不访问云服务。
5. 保留后续需求会依赖的音频编辑、录音、播放、频谱、标签、插件和工程持久化能力。
6. 保持上游 Audacity 默认 Profile 的行为不变，以降低长期合并成本。
7. 为被关闭功能建立回归测试或静态检查，防止入口重新出现。

## 4. 非目标

第一阶段不做以下工作：

- 不新增 F0、噪声、事件或其他分析结果轨。
- 不设计算法模块接口、调度器或后台结果交付。
- 不优化现有时间轴、拖动、吸附、快捷键和对齐交互。
- 不修改 `.aup4` 工程格式。
- 不删除 `au3`、Muse Framework、Project、Trackedit、ProjectScene 等核心框架。
- 不大规模物理删除 Cloud 等源码；只有在 Profile 稳定并确认没有回退需要后才另行处理。
- 不在本阶段确定最终项目名称、图标和完整品牌替换。
- 不承诺与上游所有功能或工程永久双向兼容。

## 5. 功能边界

### 5.1 保留

- 本地新建、打开、保存、另存和最近工程。
- 音频文件导入、常用格式导出和多文件操作。
- Mono、Stereo、Label 轨道。
- Clip 选择、移动、复制、剪切、粘贴、修剪、分割和分组。
- 多轨播放、静音、独奏、循环、选区和定位。
- 录音、音频设备选择、输入输出电平显示。
- 波形、频谱及两者组合显示。
- 缩放、滚动、时间标尺、吸附和现有快捷键。
- Undo/Redo、历史、自动保存和崩溃恢复。
- 本地 `.aup4` 工程及 Audacity 旧工程导入能力。
- 内置效果器、Nyquist、VST3、macOS Audio Unit，以及对应预设和实时效果框架。
- 偏好设置、主题、工作区、Dock、多窗口和无障碍能力。
- About、许可证和必要的本地帮助信息。

### 5.2 工作台 Profile 中关闭

- Audio.com 登录、授权和账号状态。
- 云工程列表、云音频列表、云保存、云打开和同步。
- Publish 页面、分享音频、生成云预览。
- MuseHub 入口和“获取效果器”推广入口。
- UsageInfo/遥测提交。
- 欢迎页中的视频、商店、Audio.com、MuseHub 和其他商业推广卡片。
- 首次启动流程中的 Audio.com 登录、更新与遥测页面。
- Audacity 自身在线更新检查。
- 仅为上述功能存在的菜单项、工具栏按钮、命令注册和深链入口。

### 5.3 暂时保留源码但不暴露

- `src/au3cloud` 及相关 Project 层云接口。
- `src/musehub`。
- `src/usageinfo`。
- Publish、云工程和欢迎推广相关 QML。
- 云相关工程元数据兼容代码。

这些代码暂留是为了让第一阶段变化可逆，并避免一次性重写仍被 Project 层引用的接口。工作台构建不得链接真实服务实现，也不得在正常 UI 中到达这些页面。

### 5.4 延后评估

- 诊断工具、开发工具和 TestFlow 在发布构建中的范围。
- 是否保留所有内置效果器、生成器和分析器。
- 是否保留全部内置工作区，只提供一个工作台默认布局。

这些项与开发效率或问题定位有关，不应在没有实际使用反馈前直接删除。

## 6. 方案比较

### 方案 A：先物理删除功能源码

优点是源码目录迅速变小。缺点是 Cloud 和产品界面仍穿过 Project、AppShell、命令、设置和工程元数据等多个层次，初期删除会产生大量与目标无关的重构，并显著增加跟进上游的冲突。

结论：不采用。

### 方案 B：工作台 Profile、关闭模块并收口 UI

增加独立产品 Profile，使用现有 CMake 开关和 stub 关闭服务模块；通过条件注册或工作台专用 UI 配置去掉入口，保留源码和兼容接口。

优点是改动可逆、容易验证、适合持续跟进上游，也能较快得到日常可用的精简版本。缺点是仓库体积短期不会显著下降，部分无用接口仍会存在。

结论：采用。

### 方案 C：只隐藏 UI，不改变构建

实现最快，但真实云服务、遥测和商业模块仍会编译、初始化，无法形成可信的本地工作台边界。

结论：不采用。

## 7. 设计

### 7.1 产品 Profile

在顶层 CMake 增加字符串配置，例如：

```cmake
set(AU4_PRODUCT_PROFILE "audacity" CACHE STRING "Product profile: audacity or workbench")
```

默认值必须保持 `audacity`，使上游默认构建行为不变。

当 Profile 为 `workbench` 时，统一设置或强制覆盖：

- `AU_BUILD_CLOUD_AUDIOCOM=OFF`
- `AU_BUILD_MUSEHUB_MODULE=OFF`
- `AU_BUILD_USAGEINFO_MODULE=OFF`
- 在线更新相关能力关闭
- 在线语言包下载关闭
- 在线扩展、在线内容和其他非崩溃诊断网络能力关闭
- 工作台专用的产品特性宏或配置值开启

现有 `src/stubs` 为关闭后的依赖提供空实现。Profile 应成为唯一入口，避免开发者每次手工组合多个开关。

### 7.2 应用启动与页面

工作台 Profile：

- 启动时直接进入本地 Home 或 Project 页面。
- Home 只显示新建、本地打开和最近本地工程。
- 不注册或不展示 Publish 页面。
- 不注册 Audio.com 登录和云同步对话框的可达入口。
- 首次启动只保留真正必要的主题、界面或工作区设置；也可以首期完全跳过首次启动向导。
- 不显示商业欢迎弹窗。

页面源码首期可以保留，但工作台 Profile 的页面注册、工具栏模型和启动流程不能暴露它们。

### 7.3 菜单与命令

工作台 Profile 中不展示：

- Open from Cloud / Save to Cloud。
- Share Audio / Publish。
- Sign in to Audio.com。
- Get Effects / MuseHub。
- 云工程相关上下文菜单。
- 上游更新检查和商业链接。

命令后端是否完全不注册，按依赖风险分两步：优先从 UI 模型移除；若命令没有内部兼容用途，再停止注册。不能留下可通过普通快捷键或菜单访问、但必然失败的空入口。

### 7.4 网络边界

工作台正常启动、打开本地工程、录放和编辑不应发起产品服务网络请求。

Workbench Profile 只允许保留崩溃诊断所需的网络能力。语言包下载、在线更新、Audio.com、MuseHub、在线扩展、在线教学内容及其他产品服务网络能力全部关闭。网络基础库可以因崩溃诊断的技术依赖继续参与构建，但不得被其他工作台功能使用。

### 7.5 兼容性

- 工作台应继续打开本地 `.aup4` 工程。
- 工程中已有云元数据时，不进行同步；本地可用内容应尽量正常打开。
- 保留云元数据字段，不在首轮执行不可逆迁移。
- 默认 Audacity Profile 的现有测试和行为不因工作台 Profile 而改变。

## 8. 验收标准

### 8.1 构建

- 默认 Audacity Profile 能完成原有配置阶段。
- Workbench Profile 能在 macOS 完成配置、编译和启动。
- Workbench 构建不链接真实 `au3cloud`、MuseHub 和 UsageInfo 实现。
- 除崩溃诊断外，Workbench 构建不启用在线语言、更新、扩展或内容服务。
- 自动化测试可在 Workbench Profile 下运行核心模块测试。

### 8.2 UI

- 启动过程不要求登录，也不显示商业欢迎内容。
- 主工具栏没有 Publish、Audio.com 或 MuseHub 入口。
- Home 页面只包含本地工程操作。
- 菜单和偏好设置中没有云保存、云打开、分享、遥测或上游更新入口。
- 普通操作不会打开已被关闭功能的空页面或错误对话框。
- 除崩溃诊断外，普通运行过程不产生产品服务网络请求。

### 8.3 核心回归

至少手工验证：

1. 新建并保存 `.aup4` 工程。
2. 导入三条音频并完成播放、静音、独奏和移动。
3. 切换波形、频谱和组合显示。
4. 创建和编辑 Label。
5. 录制一段音频。
6. 添加一个内置效果和一个受支持插件效果。
7. 撤销、重做、关闭并恢复工程。
8. 导出音频。

## 9. 实施阶段

### 阶段 1：Profile 和模块关闭

建立 Workbench Profile，关闭 Cloud、MuseHub、UsageInfo 和更新能力，保证配置与链接通过。

### 阶段 2：UI 收口

处理页面注册、启动流程、主工具栏、Home、菜单、偏好设置和欢迎内容，使关闭功能不再可见或可达。

### 阶段 3：验证与清理

运行自动化测试和核心手工流程，修复 stub 模式下的残留入口、空状态和错误提示；记录仍存在的网络行为。

### 阶段 4：物理删除评估

只有当 Workbench Profile 稳定后，才统计完全不可达且无兼容用途的代码。物理删除作为单独设计和变更处理，不与本轮混合。

## 10. 后续优化 Backlog

现有 Audacity 功能优化将在精简完成后单独排序，候选包括：

- Clip 的采样点/毫秒数值偏移。
- 键盘细调、粗调和可解释的吸附控制。
- A/B 对比与快速独奏切换。
- 时间轴横向移动、缩放和长音频导航。
- 轨道高度、面板布局和信息密度。
- Label 的显示、导入和素材绑定语义。
- 频谱显示和结果叠加体验。
- 快捷键一致性和高频工作流精简。

这些优化应分别写出用户场景和验收标准，不在功能删减阶段顺手修改。

## 11. 主要风险及控制

- **上游变化快**：默认 Profile 保持原样；工作台差异集中在少数配置和 UI 组合点。
- **Cloud 接口渗入 Project 层**：首轮使用 stub，不进行大范围接口删除。
- **误删未来算法能力所需模块**：保留录音、播放、频谱、插件和工程基础设施；未来算法如确需网络能力，再以独立需求显式加入。
- **隐藏功能但仍初始化服务**：验收同时检查构建链接、模块注册和运行网络行为，而不只检查视觉结果。
- **精简过程中夹带交互重构**：现有功能优化进入独立 backlog，精简阶段只修复因关闭功能产生的问题。

## 12. 设计中的项目决策

以下内容是本设计为推进工作作出的默认决定，尚需确认：

- 首轮采用独立 Workbench Profile，不改变默认 Audacity Profile。
- 首轮不物理删除 Cloud、MuseHub 和 UsageInfo 源码。
- 保留现有效果器、插件宿主和录音能力。
- 暂不改名和替换完整品牌资产。
- 现有交互优化延后，以独立需求逐项实施。
- Workbench Profile 的运行时网络白名单只包含崩溃诊断。

## 13. 验证记录（2026-11-01）

### 13.1 被验证的版本和环境

- 初始实现提交：`c12afde5f9ce9d47e6808b67e551151692888a2e`（Task 1-5 完成后的源码）。
- 代码审查修复提交：`d358f4569347faaf8d45f93f428710ca597b3420`（关闭剩余在线入口、隔离双 Profile 测试、强制 Workbench 本地保存）。
- 主机：macOS arm64，AppleClang 17，Xcode SDK 26.2。
- 实际被 CMake 发现的 Qt：6.12.0。
- 项目要求：Qt 6.11。由于版本不匹配，本记录区分“通过”“失败”和“未执行”，不把部分构建视为完整验收。

### 13.2 已通过

以下检查通过：

```bash
git diff --check
cmake -DTEST_CASE=default_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=workbench_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=cached_workbench_override -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=workbench_to_audacity -P buildscripts/cmake/tests/product_profile_test.cmake
cmake -DTEST_CASE=invalid_profile -P buildscripts/cmake/tests/product_profile_test.cmake
cmake --preset workbench-debug
ctest --test-dir build/workbench-debug --output-on-failure -R '^workbench_'
ctest --test-dir build/workbench-debug --output-on-failure -R '^(appshell_tests|preferences_tests|effects_base_tests|app_tests)$'
```

结果：

- 五个 Profile 策略场景全部通过；其中包含 Workbench 切回 Audacity 的缓存恢复检查。
- Workbench 配置成功。
- 9 个 Workbench 回归测试全部通过：页面清单、启动路由、菜单、工程命令、历史云元数据的本地加载、偏好设置、效果器命令、模块策略和 Network stub。
- AppShell、Preferences、Effects Base 和 App 四个相关测试套件全部通过。
- `preferences_qml` 目标单独编译成功。
- `au_project_tests` 运行了 48 个用例，其中 45 个通过；3 个既有 Clip tempo persistence 用例失败，与本轮改动文件没有交集。
- Workbench 的命令注册器和 dispatcher 均不再注册 Handbook、Ask Help，FFmpeg 偏好页保留本地 Locate 但不显示在线 Download。
- Workbench 不注册云工程页面和云 QML 类型，不允许 `link-account`；带历史云元数据的本地工程仍能加载、保存、关闭后重开，并保持非云工程状态。
- 默认 Audacity Profile 不注册任何 `workbench_*` 测试；受影响的 AppShell、Preferences、Effects Base 三个默认测试套件通过。

### 13.3 全量构建和测试结果

以下命令已执行但没有通过：

```bash
cmake --build build/workbench-debug
ctest --test-dir build/workbench-debug --output-on-failure
```

Workbench 全量构建在 Qt 6.12 生成的 `muse_extensions_qml` AOT C++ 中停止：`QQmlPrivate::AOTLookupValidation::Type` 与 `muse::extensions::Type` 名称冲突。失败发生在未修改的 Muse Extensions 代码，并且默认 Audacity Profile 在同一位置以同一错误失败。

全量 CTest 是在构建失败后执行的，因此 39 个已注册测试中有 25 个因可执行文件尚未生成而 `Not Run`；13 个已生成的测试通过，`au_project_tests` 因上述 3 个既有 Clip tempo 用例失败。该结果不是全量测试通过。

默认 Audacity 对照验证中，裸 `cmake --preset audacity-debug` 首先被本机 Command Line Tools 的 MacOSX 27 SDK/TAPI 组合阻断。给 CMake 和其依赖子构建统一设置 Xcode 26.2 的 `DEVELOPER_DIR`、`SDKROOT` 和 `CMAKE_OSX_SYSROOT` 后，默认 Profile 配置成功，并显示上游默认的真实 Cloud、Network、Update、au3cloud、MuseHub 和 UsageInfo 模块均启用。随后全量编译在与 Workbench 相同的 Qt 6.12 Extensions AOT 错误处停止。

### 13.4 模块和网络边界证据

Workbench 配置输出显示：

- `muse_cloud`、`muse_network`、`muse_update`、`au3cloud`、`musehub`、`usageinfo` 均配置为 stub。
- `muse_diagnostics` 为真实模块。
- 本次缓存中 `MUSE_MODULE_DIAGNOSTICS=ON`，`MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT=OFF`，handler 路径和 report URL 均为空。

生成的最终链接计划同样引用上述 stub archive，并引用真实 `libmuse_diagnostics.a`。链接计划还包含 `au3-network-manager`：它由保留的工程持久化/崩溃与 Sentry 兼容依赖带入，不是 Muse Network 产品服务实现。因为最终应用二进制没有生成，尚未完成静态库对象抽取后的符号检查，也未执行运行时抓包；因此不能宣称已经动态证明“除崩溃诊断外零网络请求”。

### 13.5 未执行的人工验收

应用没有成功链接，故以下检查均未执行：

- GUI 中所有在线入口的最终可达性检查。
- 本地新建、打开、保存和导出。
- 三轨导入、移动、静音、独奏、循环和播放。
- 波形、频谱、组合视图和 Label 编辑。
- 录音、输入输出电平、内置效果器和本地插件。
- Undo/Redo、关闭重开和崩溃恢复。
- Crashpad 开启构建中的偏好项、handler 和唯一上传地址。
- 最终二进制符号检查及运行时网络流量检查。

### 13.6 当前结论

产品 Profile、stub 边界、界面/命令收口、强制本地保存和相关自动化回归已通过。完整构建、全量测试和人工烟雾验收仍需在仓库指定的 Qt 6.11 环境中重新执行；在这些检查完成前，本阶段状态是“代码和定向回归完成，最终运行验收待补”，不是可发布验收通过。

## 14. Qt 6.11 补充验证（2026-11-09）

使用 `aqtinstall 3.3.0` 安装 Qt 6.11.3 `clang_64`，并补齐 `qt5compat`、`qtnetworkauth`、`qtshadertools`、`qtscxml` 和 `qttasktree` 模块。CMake 使用 Xcode 26.2 SDK 和该 Qt 安装重新执行 `workbench-debug --fresh`，配置输出确认实际发现版本为 Qt 6.11.3。

补充验证结果：

- `cmake --build build/workbench-debug` 完整通过，生成 arm64 `audacity.app`；最终二进制链接到 Qt 6.11.3。
- 9 个 `workbench_*` 测试全部通过。
- 39 个 CTest 测试程序中 38 个通过；唯一失败的 `au_project_tests` 内仍是第 13.2 节记录的 3 个既有 Clip tempo persistence 用例，没有新增 Workbench 回归失败。
- 编译出的应用已实际启动并显示 Home、Project、DevTools、时间轴和效果器面板；首次运行会显示本地音频插件校验对话框。
- GUI 烟雾检查发现 Realtime effects 空状态仍有 `Watch video` 外链，应在下一轮界面收口中删除；因此当前还不能宣称所有在线入口已完成最终人工验收。

Qt 6.12 的 QML AOT 冲突不再阻塞 Workbench 的 Qt 6.11 构建。仍待执行第 13.5 节中的音频核心流程、Crashpad 开启构建及运行时网络流量验证。

## 15. 在线入口收尾验证（2026-11-10）

本轮以统一的 `nonDiagnosticExternalLinksAvailable()` 产品策略收口剩余可达外链，同时保持默认 Audacity Profile 的原有行为。Workbench 现在：

- 实时效果空状态保留本地说明，移除 `Watch video`。
- About Credits、网站、隐私说明和 GPL 文本不生成可点击外链。
- 首次启动模型只提供 Themes、Clip Visualization 和 Workspace Layout 三个本地页。
- DevTools Interactive 页隐藏 `Open musescore.com`；Crash handler 工具继续保留。
- Cloud、Network、Update、MuseHub、UsageInfo 和 Audio.com 仍使用既有 stub/注册策略作为服务边界，Diagnostics 保持真实模块。

Qt 6.11.3 验证结果：

- Workbench 完整构建通过。
- 11 个 `workbench_*` 测试全部通过。
- AppShell、Shared、Preferences、ProjectScene、Effects Base 和 App 六个受影响测试套件全部通过。
- 全量 CTest 为 40/41；唯一失败的 `au_project_tests` 内仍是第 13.2 节记录的三个 Clip tempo persistence 用例。
- 默认 Audacity Profile 的 `ProductProfile.*` 对照测试通过，证明默认 Profile 仍报告非诊断外链可用。
- 一次 Workbench 空闲进程的 `lsof` 网络快照未发现套接字；这不是持续抓包，不能代替发布前网络流量观测。

构建应用已用于检查实时效果面板和 About 文本渲染；开发工具外链的最后一项修复随后通过 QML 构建和回归测试。按照交付约定，最终 GUI 点击验收由使用者在最新提交的构建上执行；因此本记录把自动化与构建验证标记为完成，不把人工产品验收或 Crashpad 开启构建标记为完成。

## 16. Diagnostics-only 收尾（2026-11-10）

根据“Workbench 只保留崩溃诊断，不保留独立 DevTools Tab”的要求，本轮进一步收紧开发工具边界：

- Workbench Profile 在配置阶段关闭 TestFlow；重新生成的 Ninja 构建图不包含 TestFlow 模块、运行器或脚本目标。
- Workbench 不再注册 `audacity://devtools`，顶部导航即使处于 developer mode 也不会显示 DevTools Tab。
- 必要的本地诊断出口保留在窗口顶部 Diagnostics 菜单中，该菜单在 Workbench 只包含诊断文件导出。
- Diagnostics 真实模块、工程崩溃恢复以及可选 Crashpad 打包接口继续保留。
- 默认 Audacity Profile 仍启用 TestFlow、完整 DevTools 页面和原诊断菜单。

Qt 6.11.3 下完成了 Workbench 全量构建，11 个 `workbench_*` 测试全部通过。默认 Audacity 对照构建的 `ProductProfile.*` 通过，并在配置输出中确认 TestFlow 仍启用。最终界面观感和核心音频工作流继续由使用者在最新构建上验收。
