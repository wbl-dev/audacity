# AGu 完整更名设计

日期：2026-11-15
状态：用户已确认

## 目标

把当前 OnlyAudios 派生应用的名称统一改为 **AGu**。名称不只用于窗口标题，还要用于 CMake 项目与主目标、可执行文件、应用包、安装与打包产物，以及本项目自有的 C++、QML 和内部 URI 接口。AGu 以全新本地设置启动，不自动迁移 OnlyAudios 的偏好、Modern 布局或最近项目。用户现有文件不删除。

保留已有 AU4 录放、编辑、效果器、项目和导入导出功能。继续能够打开现有 `.aup3` 与 `.aup4` 项目。改名不改变音频项目文件格式。

## 方案比较

1. **只改显示名。** 修改窗口与包名，CMake 目标及内部 URI 仍为 Audacity/OnlyAudios；改动小，但不满足“项目名字也改”。
2. **改应用、项目和本项目自有接口，保留旧引擎的兼容身份。采用。** CMake 与构建产物统一为 AGu；`src/` 自有的 `au::`、`Audacity.*` QML URI 和 `audacity://` 页面 URI 改名。保留 `au3/` 引擎、`.aup3` 格式及其必要的类型、MIME 和协议兼容标识。
3. **重命名全部历史源码和格式身份。** 连 AU3 引擎 API、第三方代码、许可证署名及 `.aup3`/`.aup4` 扩展名一起改；会破坏项目格式兼容和许可证记录，不属于产品改名。

用户选择了方案 2 所含的完整本项目接口更名，并选择全新设置；本设计中的 AU3/格式边界由现有功能与许可证约束确定。

## 命名边界

- 人类可见名称：`AGu`。macOS 包名 `AGu.app`，主进程 `AGu`；Windows 安装版主程序 `AGu.exe`；Linux 主程序 `agu`。版本仍为 `4.0.0`，不在程序文件名末尾追加版本号。
- CMake 根项目：`project(AGu)`；主目标改为 `agu`。更新主目标的链接、资源、安装、测试、Visual Studio 启动项、CI 和脚本引用。保留与 AU3 引擎相关的 `au3-*` 库目标名称，以免破坏保留引擎。
- 应用标识：macOS bundle ID `org.agu.app`，Windows 安装产品名与 Linux desktop/appdata ID 改为 AGu 对应值。新 Qt application/organization 名及目录使用 AGu，不读取或删除旧 OnlyAudios 设置。
- 安装策略：AGu 与已有 OnlyAudios 并存。Windows 使用新的 UpgradeCode、安装路径和注册表产品身份，不卸载或覆盖 OnlyAudios；macOS/Linux 也使用独立应用身份。
- 本项目自有 C++ 接口：`src/` 的 `au::` 命名空间统一改为 `agu::`，相应测试、前置声明、宏和应用类名同步。`au3/` 保留的引擎符号及第三方代码不做机械替换；`AU3`/`AU4` 作为引擎或版本标记的符号可保留。
- QML：`src/**/qml/Audacity/` 目录迁至 `src/**/qml/AGu/`，并同步 `AGu.*` 模块 URI、`qmldir`、CMake、QRC、QML imports、C++ 注册和资源查找路径。应用内部页面/对话框 URI 从 `audacity://` 改为 `agu://`，所有生产者、消费者与测试同步。
- 本项目对扩展暴露的 `Audacity.*` API 改为 `AGu.*`。旧扩展脚本 API 名不保留别名；这是用户选择完整内部接口更名的兼容代价。
- 正式图标的源文件改为 `AGu.png`，保留无文字的仓鼠画面；平台图标的旧 `AU4_AppIcon_*` 文件名若只供构建引用，改为 AGu 命名并同步引用。`output/` 中的候选图与提示词不是发行源码，不参与更名或提交。
- 活跃构建/使用文档、安装脚本及翻译目录中的产品名称改为 AGu。历史设计文档、版权署名、上游来源、代码许可证、AU3 引擎类名及 `.aup3`/`.aup4` 文件扩展名保持真实。AGu 在 Windows 使用新的 ProgID；旧 Audacity MIME 可继续作为 `.aup3` 关联，以保证现有项目的打开方式。
- 官网、帮助、论坛、支持等产品入口没有 AGu 目的地时暂不显示，安装包元数据也不指向 Audacity 网站。许可与上游来源说明仍保留真实链接。

## 验收

1. 从现有 checkout 重新配置并编译 AGu 全部目标；运行完整 CTest。主目标、运行进程和生成包显示 AGu。
2. 应用 `--version`、macOS `Info.plist`、签名和资源包检查通过。测试 AGu 的设置目录独立于 OnlyAudios，原设置文件未修改。
3. `src/` 活跃源码中不再有 `au::`、`Audacity.*` QML URI、`audacity://` 页面 URI 或 OnlyAudios 产品名；保留的 AU3 引擎、文件格式和法定署名例外逐条审计。
4. 检查 Windows/Linux 的条件构建与打包引用；本机没有对应平台工具链时，明确报告未执行的运行时验证。
