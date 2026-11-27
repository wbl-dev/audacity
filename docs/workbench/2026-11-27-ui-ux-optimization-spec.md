# AGu 界面设计与丝滑交互优化开发指南（2026-11-27）

> **致执行 Agent（ChatGPT / Codex）**：本文档是 AGu（基于 Audacity 4 的本地音频分析与脚本化标注工作台）的 UI/UX 深度诊断与工程落地规范。请严格按本文档的 **分阶段任务（Task 1 ~ Task 6）** 逐步实现，每完成一个 Task 必须通过构建与 CTest 回归验证，保持提交粒度清晰可审阅。

---

## 0. 基线信息与全局开发约束（必读）

- **当前分支与基线**：`feature/wbl/scriptable-label-import`（基线提交 `33793dc294`）
- **设计参考标杆**：**macOS 库乐队（GarageBand）/ Logic Pro / Adobe Audition** —— 追求静态沉稳的深色画布、零频闪的 GPU 选区叠加层、原生硬件光标，以及 `140~180ms` 带物理阻尼的细腻过渡。
- **构建与测试命令**：
  ```bash
  cmake --build build/workbench-debug --parallel 8
  QT_QPA_PLATFORM=cocoa ctest --test-dir build/workbench-debug --output-on-failure -j 4
  ```
  - **严禁并发运行两个完整的 `ctest` 进程**（会争用 SQLite 测试夹具 `empty_working.aup4` 导致假失败）。
  - 修改 `.qml` 文件后需执行格式化检查：`qmlformat -i --semicolon-rule=essential -l unix <file.qml>`，并确保 `git diff --check` 无空白错误。
- **不可破坏的既有行为契约**（详见 `docs/superpowers/specs/2026-11-24-track-workspace-design.md` 与 `2026-11-26-inline-spectrum-expansion.md`）：
  1. 保持单视图高度与内联双视图（波形+频谱）高度各自独立记忆，并连同 `multiViewHeightRatio` 持久化到工程 XML（`TrackHeightAttachment`）；同一时刻最多展开一条双视图音轨。
  2. 保持立体声左右声道比例 `channelHeightRatio` 与波形/频谱比例 `multiViewHeightRatio` 彻底解耦。
  3. 保持“存在时间选区或选中区间标注时，Play 播放该区间一次；无选区时从游标处播放”。
  4. 保持原生 Clip 水平/跨轨拖动、左右裁剪（Trim）、拉伸（Stretch）、撤销/重做（Undo/Redo），以及标注 `owner` / `color` 元数据的导入导出兼容。

---

## 一、核心痛点根因总览：为什么当前体验“一闪一闪、操作生硬”？

经源码级排查，“一闪一闪”和“生硬感”来自以下 **四类具体工程机制**：

1. **视觉频闪与帧差（6 处直接闪烁源）**：
   - 空移鼠标时每像素触发磁吸检测，全屏亮青/亮黄竖线高频闪灭；
   - 折叠音轨（`< 120px`）上鼠标划过 Clip 时，`20px` 高饱和亮色顶栏 `0ms` 突然弹出盖住波形、移开瞬间消失；
   - 点按/框选时多层选区矩形跳变 `z` 轴，同时触发 `WaveView` 与 `ClipChannelSpectrogramView`（主线程双层循环 `image.setPixelColor`）CPU 全图重绘，QML 图层与 CPU 位图存在 1~2 帧时间差；
   - `ClipItem` 根矩形底色为高明度亮色（`#66A3FF` / `#B0D4FF`），每次缩放/改轨高/拉分割线重建 `QQuickPaintedItem` 纹理时底板亮色外露闪烁；
   - 标注 `Loader` 同时开启 `asynchronous: true` 与 `active: !itemData.dragged`，起拖销毁真身、异步加载替身，松手销毁替身、异步重建真身，抓放瞬间各空闪 1~2 帧；
   - 多层 `MouseArea` 先设 `Qt.BlankCursor` 再通过 `restoreOverrideCursor()` / PNG 缩放抢占光标，在 macOS Retina 下跨区域移动指针闪烁。
2. **手势冲突与交互 Bug**：
   - 单击轨道头会调用 `dataSelectionRequested()` 把全局时间选区强行覆盖成整轨总长，直接违背设计规范；
   - `OverviewNavigator` 在放大视图（视口框宽度 `< 14px`）时左右各 `7px` 的缩放热区完全吞噬中间平移区，导致视口卡死无法拖动；
   - 轨道底边调高热区仅 `4px` 且完全透明；波形/频谱分割线仅 `1px` 无把手；轨道排序拖拽无位移阈值；
   - `VolumeSlider` 未启用 `0 dB` 刻度与磁吸、点击边缘跳变、`Shift` 微调未钳制上下限；`ImportDropArea` 漏减 `44px` 概览条高度。
3. **零动效硬切**：
   - 内联频谱展开/收起（`±240px`）与自动折叠上一轨均为 `0ms` 瞬移，且上方轨道自动收起时未做滚动锚定，当前点击的轨道头会从鼠标下方跳走。
4. **界面几何错位与色彩打架**：
   - 左上角 `84px` 单块 `TracksTitleBar` 与右上角 `44px + 40px`（概览条+时间尺）横线脱节；折叠轨最小高度 `100px` 却塞入 `111px` 的三行控件；
   - `dark.cfg` 同时混用 6 种高亮色相，`Timeline` 标尺重复叠画紫色+青色两层选区，分割辅助线 `#0121C0` 在深色画布上几乎不可见；
   - 首页残留 `260px` 宽却只有一个 `"Project"` 项的空壳侧边栏；标注轨 `owner` 分组仅留空白行而无泳道标题、超出高度无提示。

---

## 二、分模块详细优化方案与代码级实现建议

### 模块 A：彻底消除“一闪一闪”（De-flickering & GPU 图层解耦）

#### A1. 禁用空移鼠标时的磁吸对齐线频闪
- **涉及文件**：
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TrackClipsContainer.qml`（约 `#L269-L280`、`#L574-L578`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TrackLabelsContainer.qml`（约 `#L151-L152`、`#L173-L178`、`#L299-L303`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TracksItemsView.qml`（约 `#L785-L822`、`#L1324-L1336`）
- **开发建议**：
  1. **仅在主动编辑手势进行中显示 `snapGuideline`**：当没有任何按压/拖拽/裁剪/选区拉伸/外部拖入（即 `!mainMouseArea.pressed && !root.moveActive && !selectionViewController.selectionInProgress && !selectionViewController.selectionEditInProgress && !content.leftTrimPressedButtons && !content.rightTrimPressedButtons`）时，**不要在 `onPositionChanged` 或 `HoverHandler` 中调用 `findGuideline` 点亮 `snapGuideline`**。
  2. 为 `snapGuideline` 增加柔和透明度（`0.65`）与 `Behavior on opacity { NumberAnimation { duration: 80 } }`（改用 `opacity: root.guidelineVisible ? 0.65 : 0` + `visible: opacity > 0.01`，避免生硬开关）。
  3. 在 `TracksItemsView.qml`（`#L415-L423`）中：当鼠标离开时间标尺与画布区域时隐藏 `lineCursor`；当鼠标仅在下方音轨区移动时，将标尺上的 `lineCursor.opacity` 降至 `0.35`，避免顶部白线刺眼跳动。

#### A2. 移除折叠轨悬停时的 `20px` 顶栏闪现
- **涉及文件**：`src/projectscene/qml/AGu/ProjectScene/tracksitemsview/ClipItem.qml`（`#L608-L618`、`#L900-L905`）
- **开发建议**：
  - 将 `header` 的可见条件从 `visible: !root.collapsed || root.hover || menuBtn.isMenuOpened` 改为：
    - 当 `root.collapsed` 为 `true` 时，**不在普通悬停时弹出 `20px` 不透明横幅遮挡波形**。
    - 折叠态下的 Clip 移动与菜单交互可通过以下任一方式优雅保留：① 顶部保留 `6px` 透明拖拽热区（光标变为手型但不弹色块），仅当 `menuBtn.isMenuOpened` 或标题编辑激活时淡入顶栏；或 ② 悬停停顿 `180ms` 后以半透明胶囊（`opacity: 0 -> 0.88`, `120ms` 动画）轻柔浮现片段名，绝不做 `0ms` 硬闪。

#### A3. 统一 GPU 选区遮罩、优化频谱像素绘制并消除 `ClipItem` 亮底外露
- **涉及文件**：
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/ClipItem.qml`（`#L117-L120`、`#L919`）
  - `src/projectscene/view/tracksitemsview/waveview.cpp`（`#L123-L157`、`#L260-L263`、`#L327-L336`）
  - `src/spectrogram/view/clipchannelspectrogramview.cpp`（`#L102-L120`）
  - `src/spectrogram/internal/au3/au3spectrogramclipchannelpainter.cpp`（`#L189-L221`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TrackItemsContainer.qml`（`#L135-L162`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TracksItemsView.qml`（`#L425-L436`、`#L637`、`#L731`、`#L1276-L1287`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/Timeline.qml`（`#L97-L104`）
- **开发建议**：
  1. **`ClipItem` 根底色改为安静画布色**：将 `ClipItem.qml` 根 `Rectangle` 的 `color` 从高明度 `clipColor` / `clipSelectedColor` 改为深色画布底色（深色模式下基于 `waveform_background_color` `#202022` 混入 `6%` 轨道色，选中态混入 `12%` 轨道色）。这样在缩放、调整轨高或拖动分割线触发 `QQuickPaintedItem` 重新分配纹理尺寸的那 1 帧里，底层绝不会闪出刺眼的浅蓝底板。
  2. **修正 `WaveView::backgroundColor()` 跳变**：在 `waveview.cpp#L327-L336` 中，固定返回 `params.style.normalBackground`，不要因为 `selectionEndTime >= m_clipTime.endTime` 就把整个 `WaveView` 的 `fillColor` 翻转为亮色选区背景。
  3. **固定 `selectionRectangle` 层级并清理重复选区层**：
     - 在 `TrackItemsContainer.qml#L135-L149` 中，保持 `selectionRectangle` 恒定位于 `z: 2`（不再在 `0` 与 `2` 之间跳变），通过透明度区分目标编辑轨（如 `isDataSelected ? 0.18 : 0.06`）并在左右边界各画一条 `1px` 高亮边线，且仅当 `selectionEndTime > selectionStartTime` 时显示。
     - 移除 `TracksItemsView.qml#L637` 在普通单击按下瞬间就点亮全高 `itemsSelection` 的闪动逻辑。
     - 清理时间尺双重选区叠色：合并 `Timeline.qml#L97-L104`（紫色）与 `TracksItemsView.qml#L425-L436`（青色），在时间尺上只保留一层与轨道选区色相一致的高亮条。
  4. **频谱绘制性能与选区解耦（关键性能优化）**：
     - 在 `au3spectrogramclipchannelpainter.cpp#L190-L221` 中，当前使用双层循环逐像素调用 `image.setPixelColor(xx, imageHeight - yy - 1, QColor(...))`，在主线程开销极大。请改为按行获取 `QRgb* scanLine = reinterpret_cast<QRgb*>(image.scanLine(row))` 直接写入 `qRgb(rv, gv, bv)`（可提速 `10x` 以上）。
     - 当无频率选区（普通时间框选）时，不要因为 `selectionStartTime` / `selectionEndTime` 每帧变化就重跑整张频谱图的 CPU 像素着色；时间选区统一由顶层 QML GPU 选区矩形表达，保证框选在开启频谱时依然 60/120fps 丝滑无闪烁。

#### A4. 修复标注与片段拖拽时的异步销毁空闪
- **涉及文件**：`src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TrackLabelsContainer.qml`（`#L197-L200`）
- **开发建议**：
  - 将 `labelLoader` 的 `asynchronous: true` 改为 `asynchronous: false`。`LabelItem` 结构轻量，同步实例化耗时微乎其微，可彻底消除起拖销毁、松手重建以及水平快速滚动进出缓存区时的 1~2 帧空白闪烁。

#### A5. 原生硬件光标优先，修复位图光标切换闪烁
- **涉及文件**：
  - `src/projectscene/view/common/customcursor.cpp`（`#L53-L67`、`#L171-L185`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/{TracksItemsView,TrackClipsContainer,TrackLabelsContainer,ClipItem,ItemsSelection}.qml`
- **开发建议**：
  1. 对于系统原生已提供高精度矢量光标的形态：**时间选择统一直接使用 `Qt.IBeamCursor`，选区左右边缘调整使用 `Qt.SizeHorCursor`（或缓存好的高 DPI 光标），手型抓取使用 `Qt.OpenHandCursor` / `Qt.ClosedHandCursor`**，移除多层 `MouseArea` 先设 `Qt.BlankCursor` 再挂载 `IBeamCursor.png` 的做法。macOS WindowServer 硬件合成原生光标永远不会闪烁。
  2. 对于必须保留自定义图标的特殊工具（如采样点画笔 `Pencil`、平滑刷 `Smooth`、片段裁剪/拉伸 `ClipTrim` / `ClipStretch`）：
     - 在 `CustomCursorProvider` 中按 `(source, physicalSize)` 缓存 `QCursor` 对象，避免每次鼠标进出边缘都重新读取并 `scaled()` 缩放 PNG；
     - 在 `CustomCursor::refresh()` 中，若当前已有活跃的 overrideCursor，优先调用 `QGuiApplication::changeOverrideCursor(m_cursor)` 原地替换，**不要先调用 `restoreOverrideCursor()` 再 `setOverrideCursor()`**（这一退一进正是导致系统箭头闪现一帧的根因）。

---

### 模块 B：库乐队式细腻手感、动效阻尼与交互 Bug 修复

#### B1. 修复单击轨道头误改写全局时间选区（P0）
- **涉及文件**：
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/TrackItem.qml`（`#L131-L149`）
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/TracksPanel.qml`（`#L221-L223`、`#L288-L290`）
- **开发建议**：
  - 删除 `TrackItem.qml` 单击 `onPressed` 中的 `root.dataSelectionRequested()` 调用。左键单击轨道头（或配合 `Cmd` / `Shift` 多选）**只更新选中目标编辑轨（`selectionRequested`），绝不触碰当前全局时间选区 `startTime ~ endTime`**。
  - 若需保留“选中整条音轨首尾时间范围”的快捷方式，可改为“在轨道头非标题空白处双击”或在轨道右键菜单中提供 `Select Track Audio`。
  - 补充单元测试验证：已有 `[2.0s, 5.0s]` 时间选区时，点击切换目标轨道后选区仍为 `[2.0s, 5.0s]`。

#### B2. 修复 `OverviewNavigator` 放大后卡死无法平移 + 补齐选区投影与快捷手势（P0）
- **涉及文件**：
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/OverviewNavigator.qml`（`#L34-L119`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TracksItemsView.qml`（`#L286-L294`）
- **开发建议**：
  1. **修复窄视口边缘吞噬 Bug**：重构边缘判定，当视口像素宽度 `boxWidth = strip.viewRight - strip.viewLeft` 较窄时，将左缩放判定区放到视口框左外侧 `[viewLeft - 8, viewLeft + Math.min(3, boxWidth * 0.2)]`，右缩放判定区放到右外侧 `[viewRight - Math.min(3, boxWidth * 0.2), viewRight + 8]`。**保证无论放大到什么程度，点击视口框中心永远触发平移（`edge === 0`）**。
  2. **增加选区投影带**：当 `context.selectionActive` 为真时，在概览条内用半透明高亮条画出 `selectionStartTime ~ selectionEndTime` 所在全局位置。
  3. **补齐自然手势**：
     - 双击概览条视口框：在“全局适配缩放（Zoom to Fit Project）”与上一次局部缩放间切换；
     - 在概览条上支持滚轮/触控板缩放与水平滚动；
     - 点击视口框外空白处跳转时，调用平滑滚动动画（或支持按住后直接连续跟手拖走视口）。
  4. **右侧边界对齐**：在 `TracksItemsView.qml` 中，当右侧 `VerticalRulersPanel`（`32px`）可见时，让 `OverviewNavigator` 内部条带右边界与 `timeline.right` 严格对齐（避免错位 `20px`）。

#### B3. 内联频谱双视图展开/收起：平滑缓动 + 视口锚定 + 消除无关重绘
- **涉及文件**：
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/TrackItem.qml`（`#L67`、`#L260-L291`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/TrackItemsContainer.qml`（`#L66`、`#L175-L209`）
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/VerticalRulersPanel.qml`（`#L57`）
  - `src/projectscene/internal/projectviewstate.cpp`（`#L345-L353`、`#L423-L472`）
  - `src/projectscene/view/timeline/projectoverview.cpp`（`#L41`）
- **开发建议**：
  1. **区分“状态切换动画”与“手动拖拽跟手”**：
     - 在 `ProjectViewState`（或 `TrackViewStateModel`）暴露 `isResizingTrack` 状态（手动拖拽底边时为 `true`）。
     - 在 `TrackItem`、`TrackItemsContainer`、`VerticalRulersPanel` 的 `height` 上绑定：`Behavior on height { enabled: !trackViewState.isInteractiveResize; NumberAnimation { duration: 170; easing.type: Easing.OutCubic } }`，并在动画期间让 `TracksPanel` / `TracksItemsView` 实时同步布局高度。手动拉边框时 `enabled: false`，保持零延迟 `1:1` 跟手。
  2. **自动折叠上方轨道时的滚动锚定（Scroll Anchoring）**：
     - 当展开第 $k$ 轨导致排在它上方的第 $i$ 轨（$i < k$）自动收起（高度减少 $\Delta H$）时，若当前存在纵向滚动偏移，同步调整 `tracksVerticalOffset` 补偿 $\Delta H$，防止用户鼠标正指着的第 $k$ 轨轨道头突然往上窜走。
  3. **消除无关的 `ProjectOverview` 全量重绘**：
     - `projectviewstate.cpp#L350` 在切换 `viewType` 时调用了 `notifyAboutTrackChanged(*track)`，导致 `projectoverview.cpp#L41` 无谓地重画所有音轨概览波形。请在 `ProjectOverview` 中缓存各轨 `colorIndex`（仅当轨道颜色或音频片段变化时才 `refresh()`）。

#### B4. 拓宽拖拽热区、增加视觉抓手（Affordance）与微交互动画
- **涉及文件**：
  - `TrackItem.qml`、`TrackItemsContainer.qml`、`TrackClipsContainer.qml`（`#L769-L798`）、`TracksPanel.qml`（`#L360-L417`）、`VolumeSlider.qml`、`PlaybackToolBar.qml`、`ImportDropArea.qml`
- **开发建议**：
  1. **轨道底边调高把手**：热区高度从 `4px` 扩大到 `8px`（上下各跨 `4px`）；悬停或拖拽时底部分割线以 `120ms` 淡入高亮色；双击底边时，普通音轨恢复默认单/双视图高度，标注轨自动适配当前总标注行数高度。
  2. **波形/频谱分割线（`viewDivider`）**：悬停热区设为 `10px`，默认 `1px` 低对比细线，悬停或按住时平滑过渡为 `2px` 亮线并在轨道水平中央浮现一枚 `32×4px` 圆角胶囊把手（Pill Grip）。
  3. **轨道拖拽排序起拖阈值**：在 `TracksPanel.qml#L360-L417` 中记录按下坐标，移动距离超过 `6px` 后再进入 `dragging = true` 状态，防止普通点击闪出拖拽态。
  4. **补齐基础控件微动效**：为工具栏按钮背景、轨道头选中/悬停底色、`M / S / 展开` 切换按钮统一增加 `110ms` `ColorAnimation`。
  5. **修复 `VolumeSlider.qml`**：
     - 在 `83.33%`（即 `0.0 dB`）位置绘制一条 `1×6px` 零增益刻度短线；
     - 启用已声明但未使用的磁吸逻辑：非 `Shift` 状态下，距离 `0.0 dB` 小于 `±1.2 dB` 时自动吸附到 `0.0 dB`；
     - 点击在滑块手柄（Handle）范围内时记录相对偏移，不发生瞬间音量跳变；
     - 为 `fineSliderValue()` 加上 `Math.max(root.from, Math.min(root.to, ...))` 边界钳制。
  6. **拆分工具栏互斥状态（`playbacktoolbarmodel.cpp#L202-L212`）**：`PROJECTSCENE_TIME_SELECTION_TOOL_COMMAND` 仅与 `PROJECTSCENE_TOGGLE_SPLIT_TOOL_COMMAND` 互斥；开启 `Clip Gain Automation`（包络线视图叠加）时不再取消 `Time Selection` 按钮高亮。
  7. **修复 `ImportDropArea.qml#L90-L116` 坐标偏差**：将 `dropY -= root.timeline.height` 修正为减去完整的顶部区域高度（`overview.height + timeline.height`，即 `84px`），并将 `dropX` 减去左侧画布缩进 `12px`。

---

### 模块 C：沉稳深色界面视觉重构与几何对位（GarageBand / Audition 质感）

#### C1. 深色主题色彩语义收敛（`src/app/configs/dark.cfg`）
将当前杂乱的 6 套高亮色收敛为克制、语义明确的录音棚深色体系：

| 配置项 / 组件 | 当前表现与问题 | 建议优化值与视觉规范 |
| :--- | :--- | :--- |
| `guideline_split_color` | `#0121C0`（纯深蓝，在 `#202022` 画布上对比度仅 `1.6:1`，几乎隐形） | 改为高辨识度暖琥珀/珊瑚色 `#F59E0B` 或 `#FF7849` |
| `selection_highlight_color` & 时间尺选区 | 画布冰青 `#ABE7FF` + 时间尺叠画紫 `#926BFF` | 统一为同一套柔和冷青蓝 `#7DD3FC`（时间尺与画布同色系，单层绘制） |
| 轨道 `M`（Mute）/ `S`（Solo）按钮 | 激活态同为紫色 `accent_color`，无法一眼分辨静音与独奏 | **Mute 激活**：冷雾蓝 `#3B82F6`（白字）；**Solo 激活**：暖琥珀金 `#EAB308`（深黑字） |
| 工具栏主电平条（`PlaybackToolBar.qml#L288`） | 使用紫色 `accentColor` 且停止播放瞬间归零 | 统一为音频电平语义色（正常 `#2DD4BF` / `#4ADE80`，`-6~0dB` 琥珀黄，过载 `#EF476F`），并对下降过程加 `120ms` 视觉回落阻尼 |
| 轨道聚焦外框（`trackFocusState`） | `2px` 亮天蓝 `#7EB1FF` 粗框包住整条轨道头和整行画布，像调试线框 | 仅在键盘 Tab 导航激活时显示细焦点环；鼠标常规选中轨道改用**左侧 `4px` 轨道色标竖条高亮 + 轨道头微提亮（`#3D3E44`）**表达 |

#### C2. 重塑沉稳深色 `ClipItem` 外观与轨道头左侧色标竖条
- **涉及文件**：
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/ClipItem.qml`（`#L36-L42`、`#L117-L120`、`#L608-L631`、`#L1243-L1349`）
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/TrackItem.qml`（`#L154-L163`）
  - `src/projectscene/view/trackspanel/paneltrackslistmodel.cpp`（向轨道头暴露轨道 `color`）
- **开发建议**：
  1. **告别“粉彩贴纸条”，改用深色半透明染色 Clip Header**：
     - 在暗色主题下，将 `ClipItem` 顶部 `20px` `header` 的底色由高明度纯 `clipColor` + `#000000` 黑字，改为：`ui.blendColors("#202022", root.clipColor, root.clipSelected ? 0.40 : 0.24)`（悬停微提亮至 `0.32` / `0.48`），文字与按钮图标改为清晰浅白 `#ECECEF`；
     - 在 `ClipItem` 最顶部画一条 `2px` 高的纯 `clipColor` 细色带，外边框改为 `1px` `ui.colorWithAlphaF(root.clipColor, root.clipSelected ? 0.85 : 0.35)`（选中态清晰高亮，常态克制内敛）。
  2. **轨道头左侧 `4px` 主题色标条（Track Color Swatch）**：
     - 利用 `TrackItem.qml#L154-L163` 现有的 `spacer` 区域，绘制一条宽 `4px`、圆角 `2px` 的竖向轨道主题色条（音频轨取对应轨道波形色，标注轨取标注轨主题色；当前选中轨道 `opacity: 1.0`，未选中轨道 `opacity: 0.45`），让左侧轨道控制栏与右侧波形色彩建立直观映射。

#### C3. 左右面板像素级几何对齐、折叠轨紧凑排版与工具栏视觉岛
- **涉及文件**：
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/TracksTitleBar.qml`
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/TracksPanel.qml`（`#L157`）
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/WaveTrackItem.qml`（`#L18-L64`）
  - `src/projectscene/qml/AGu/ProjectScene/toolbars/PlaybackToolBar.qml`
- **开发建议**：
  1. **对齐左上角 `84px` `TracksTitleBar` 与右上角 `44px + 40px` 双层头**：
     - 将左侧 `84px` 标题栏拆为上下两层：上层 `44px`（底色与 `OverviewNavigator` 一致，显示精简的全局轨道统计或“收起全部频谱”快捷按钮，并在 `y = 44px` 处画贯通分割线）；下层 `40px`（与 `Timeline` 时间尺齐平，放置 `Tracks` 标题与 `+ Add track` 按钮）。
     - 移除 `TracksPanel.qml#L157` 多余的 `Layout.topMargin: 1`，确保左右音轨列表 `y` 坐标 `0px` 严丝合缝。
  2. **修复折叠轨内部挤压（`111px > 100px`）**：
     - 在 `WaveTrackItem.qml` 中，当 `root.collapsed` 为真（轨高 `< 116px`）时，利用 `TrackItem` 已预留的 `headerTrailingControlsComponent` 插槽，将紧凑版 `[M][S][▾]` 移至第一行标题右侧，第二行仅保留全宽细长 `VolumeSlider`。两行总高仅约 `68px`，即使缩到 `84~100px` 也宽敞舒展、绝不顶穿底边。
  3. **工具栏“视觉岛（Visual Islands）”分组**：
     - 在 `PlaybackToolBar.qml` 中，将走带控制与时间码 `[⏮ | ▶ | ⏹ | ● | 00:00.000]` 包裹在一个微下沉的深色圆角底座（`#202022`, `radius: 6`, `border: 1px #38383E`）中；将互斥编辑工具 `[I-Beam | Split]` 做成相连的分段胶囊按钮组（Segmented Control），从视觉上一眼区分“走带仓”、“光标工具”与“视图辅助”。

---

### 模块 D：核心差异化功能（标注轨闭环）与顶层壳层瘦身（P2）

#### D1. 标注轨（Label Track）可读性与操作闭环升级
- **涉及文件**：
  - `src/projectscene/view/tracksitemsview/tracklabelslayoutmanager.{h,cpp}`
  - `src/projectscene/qml/AGu/ProjectScene/tracksitemsview/{TrackLabelsContainer,LabelItem,LabelHeader}.qml`
  - `src/projectscene/qml/AGu/ProjectScene/trackspanel/LabelTrackItem.qml`
- **开发建议**：
  1. **`owner` 语义泳道可视化**：由 `TrackLabelsLayoutManager` 导出每个 `owner` 分组的起始行号 `startLevel` 与名称，在 `TrackLabelsContainer.qml` 中为分组间隔行绘制极细水平虚线/浅分割线，并在视口左边缘悬浮一枚半透明贴边胶囊徽标（如 `Speaker_A`），不再让用户面对无解释的空白行。
  2. **多行溢出提示**：由 `TrackLabelsLayoutManager` 暴露总行数 `totalRowCount`；当 `totalRowCount * 24 > trackHeight` 时，在标注轨右下角浮现一枚可点击胶囊 `▼ +N 行（点击展开）`，点击即调用 `setTrackHeight` 撑开至完整高度。
  3. **点标注图形精简与回车保留选中**：
     - 精简 `LabelItem.qml` 中 `12px` 大圆点叠 `24px` 粗竖线的厚重造型，改为顶部 `8px` 精致菱形/圆头针标 + `1px` 细引线；当点标注靠近视口右边缘放不下标题卡时，将标题卡翻转到针标左侧（`pointAnchorX - width - 8`），避免盖住针标本身。
     - 在 `TrackLabelsContainer.qml#L329` 中，回车完成标题编辑（`onTitleEditAccepted`）后保持当前标注选中，不要调用 `resetSelectedLabels()` 打断连续键盘操作。
  4. **快捷导入入口**：在 `LabelTrackItem.qml` 增加紧凑的 `Import…` 按钮，并在 `ImportDropArea` / `DropController` 中支持将已注册脚本后缀的标注文件直接拖入画布或标注轨导入。

#### D2. 首页与顶部栏去冗余（AppShell 瘦身）
- **涉及文件**：
  - `src/appshell/qml/AGu/AppShell/HomePage/{HomePage,HomeMenu}.qml`
  - `src/project/qml/AGu/Project/ProjectsPage.qml`
- **开发建议**：
  - 移除 `HomePage.qml` 左侧固定 `260px` 宽、仅含单项 `"Project"` 的 `menuPanel`（`HomeMenu`）；在 `ProjectsPage.qml` 顶部设计简洁的欢迎操作栏，将 `新建空白工程`、`打开音频文件…` 提权为主视觉入口卡片，下方铺开最近工程列表/网格。

---

## 三、推荐分批执行任务单（供 ChatGPT / Codex 逐批落地）

建议按以下 **6 个独立可测的 Task** 顺序执行并分别提交：

- [x] **Task 1（去闪烁核心批）**：完成 **A1 + A2 + A4 + A5**（关闭空移鼠标磁吸线频闪、移除折叠轨悬停弹顶栏、`TrackLabelsContainer` 改为同步加载消除抓放空闪、替换为 macOS 原生光标并缓存自定义光标且改用 `changeOverrideCursor`）。
  - *验证*：编译运行 `projectscene_tests`；实机检查空移鼠标、跨折叠轨移动、拖动标注与片段抓放、跨边缘移动鼠标均无任何闪烁。
- [x] **Task 2（选区 GPU 化与频谱绘制提速）**：完成 **A3**（`ClipItem` 根底色改为深色画布底、固定 `WaveView::backgroundColor`、固定 `selectionRectangle` 层级并清理 `Timeline` 重复选区层、`au3spectrogramclipchannelpainter.cpp` 改用 `scanLine` 并解耦纯时间选区重绘）。
  - *验证*：运行 `projectscene_tests` 与频谱相关测试；实机开启双视图频谱后快速拖选时间区间、缩放、拉分割线，确认无亮蓝漏底且帧率顺滑。
- [x] **Task 3（关键手势修复与概览条升级）**：完成 **B1 + B2 + B4（第 5~7 项）**（单击轨道头保留全局时间选区、修复 `OverviewNavigator` 窄视口无法平移并补齐选区投影/双击 Fit/右侧对齐、修复 `VolumeSlider` 0dB 刻度磁吸与 Shift 钳制、拆分工具栏 `Automation` 开关状态、修复 `ImportDropArea` `44px` 偏移）。
  - *验证*：新增/更新 `projectscene_tests`（含轨道点击保留选区、概览条窄视口平移测试、`playbacktoolbarmodel_tests`）。
- [x] **Task 4（库乐队式阻尼动效与分割把手打磨）**：完成 **B3 + B4（第 1~4 项）**（双视图展开/收起 `170ms OutCubic` 缓动 + 手动拉边框 `0ms` 跟手 + 上方自动折叠滚动锚定、隔离 `ProjectOverview` 无关重绘、拓宽轨道底边与波形/频谱分割线热区并增加胶囊 Grip 把手、轨道排序 `6px` 起拖阈值、基础按钮悬停颜色过渡）。
  - *验证*：运行 `ProjectViewStateTests.*` 全量回归；实机测试单双视图切换动画、跨轨切换锚定及手动拖拽分割线跟手度。
- [x] **Task 5（沉稳深色视觉重构与左右几何对齐）**：完成 **C1 + C2 + C3**（收敛 `dark.cfg` 配色与分割线可见度、深色半透明染色 `ClipItem` 顶栏、轨道头左侧 `4px` 主题色标条、左上 `TracksTitleBar` `44+40px` 双层对齐、折叠轨两行紧凑布局、`M/S` 语义色、工具栏走带胶囊与分段按钮）。
  - *验证*：深浅两套主题（重点深色主题）实机走查与全量 CTest `40/40` 通过。
- [x] **Task 6（标注轨泳道闭环与首页瘦身）**：完成 **D1 + D2**（标注轨 `owner` 悬浮泳道徽标、多行截断展开提示、点标注边缘翻转与回车保选中、标注文件快捷拖入、移除 `HomePage` 冗余 `260px` 侧边栏）。
  - *验证*：运行 `tracklabelslayoutmanager_tests`、`au3labelsinteractions_tests`、`appshell_tests`、`workbench_*` 测试集并实机回归。

## 四、落实记录（2026-11-27）

| 批次 | 实现提交 |
| :--- | :--- |
| Task 1 | `de3c831baa` |
| Task 2 | `abdb1a2299` |
| Task 3 | `4b62a2074f` |
| Task 4 | `d91cf9dcb3` |
| Task 5 | `316b75cbbf` |
| Task 6 | `5bdf20bd92` |

六个批次均已完成。批次外的独立修复记录为 `5924fe993e`、`989174ad68` 和 `0c3423f5d2`。
