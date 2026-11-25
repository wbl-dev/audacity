/*
* Audacity: A Digital Audio Editor
*/
#include "projectscenemodule.h"

#include <QtQml>

#include "types/projectscenetypes.h"

#include "framework/ui/iuiactionsregister.h"
#include "framework/interactive/iinteractiveuriregister.h"
#include "framework/rcommand/icommandsregister.h"
#include "framework/rcommand/icommandsstate.h"

#include "internal/projectsceneuiactions.h"
#include "internal/projectsceneactionscontroller.h"
#include "internal/projectscenecommandsregister.h"
#include "internal/projectscenecommandsstate.h"
#include "internal/projectsceneuistate.h"
#include "internal/projectsceneconfiguration.h"
#include "internal/projectviewstatecreator.h"
#include "internal/realtimeeffectpaneltrackselection.h"

#include "view/common/tracksviewstatemodel.h"
#include "view/common/trackviewstatemodel.h"
#include "view/common/customcursor.h"

#include "view/toolbars/audiosetupcontextmenumodel.h"
#include "view/toolbars/projecttoolbarmodel.h"
#include "view/toolbars/undoredotoolbarmodel.h"

#include "view/trackspanel/addeffectmenumodel.h"
#include "view/trackspanel/paneltrackslistmodel.h"
#include "view/trackspanel/realtimeeffectlistitemmenumodel.h"
#include "view/trackspanel/realtimeeffectlistmodel.h"
#include "view/trackspanel/realtimeeffectrowactionscontroller.h"
#include "view/trackspanel/realtimeeffectsectionmodel.h"
#include "view/trackspanel/trackcontextmenumodel.h"
#include "view/trackspanel/trackitem.h"
#include "view/trackspanel/volumepressuremeteritem.h"

#include "view/tracksitemsview/viewtrackslistmodel.h"
#include "view/tracksitemsview/trackclipslistmodel.h"
#include "view/tracksitemsview/trackitemsmovecontroller.h"
#include "view/tracksitemsview/trackclipitem.h"
#include "view/tracksitemsview/tracklabelslistmodel.h"
#include "view/tracksitemsview/tracklabelslayoutmanager.h"
#include "view/tracksitemsview/tracklabelitem.h"
#include "view/tracksitemsview/waveview.h"
#include "view/tracksitemsview/clipcontextmenumodel.h"
#include "view/tracksitemsview/multiclipcontextmenumodel.h"
#include "view/tracksitemsview/labelcontextmenumodel.h"
#include "view/tracksitemsview/canvascontextmenumodel.h"
#include "view/tracksitemsview/selectioncontextmenumodel.h"
#include "view/tracksitemsview/selectionviewcontroller.h"
#include "view/tracksitemsview/splittoolcontroller.h"
#include "view/tracksitemsview/pitchandspeedchangemodel.h"
#include "view/tracksitemsview/wavepainterproxy.h"
#include "view/tracksitemsview/au3/connectingdotspainter.h"
#include "view/tracksitemsview/au3/minmaxrmspainter.h"
#include "view/tracksitemsview/au3/samplespainter.h"
#include "view/tracksitemsview/mousehelper.h"
#include "view/tracksitemsview/dropcontroller.h"
#include "view/tracksitemsview/labeleditor/labelstableviewmodel.h"
#include "view/tracksitemsview/labeleditor/addnewlabeltrackmodel.h"

#include "view/timeline/timelinecontext.h"
#include "view/timeline/projectoverview.h"
#include "view/timeline/timelineruler.h"
#include "view/timeline/timelinecontextmenumodel.h"
#include "view/timeline/playregioncontroller.h"
#include "view/timeline/playregionmodel.h"

#include "view/timeline/gridlines.h"

#include "view/playcursor/playcursorcontroller.h"

#include "view/statusbar/selectionstatusmodel.h"

#include "view/toolbars/playbacktoolbarmodel.h"
#include "view/toolbars/playbacktoolbarcustomisemodel.h"
#include "view/toolbars/playbacktoolbarcustomiseitem.h"

#include "view/historypanel/historypanelmodel.h"

#include "view/trackruler/trackrulermodel.h"

using namespace agu::projectscene;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::interactive;

static const std::string mname("projectscene");

static void projectscene_init_qrc()
{
    Q_INIT_RESOURCE(projectscene);
}

std::string ProjectSceneModule::moduleName() const
{
    return mname;
}

void ProjectSceneModule::registerResources()
{
    projectscene_init_qrc();
}

void ProjectSceneModule::registerExports()
{
    m_configuration = std::make_shared<ProjectSceneConfiguration>();

    globalIoc()->registerExport<IProjectSceneConfiguration>(mname, m_configuration);
    globalIoc()->registerExport<IProjectViewStateCreator>(mname, std::make_shared<ProjectViewStateCreator>());
}

void ProjectSceneModule::resolveImports()
{
    auto ir = globalIoc()->resolve<IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(muse::Uri("agu://projectscene/editpitchandspeed"),
                           "AGu/ProjectScene/tracksitemsview/pitchandspeed/PitchAndSpeedChangeDialog.qml");
        ir->registerQmlUri(muse::Uri("agu://projectscene/insertsilence"),
                           "AGu/ProjectScene/common/InsertSilence.qml");
        ir->registerQmlUri(muse::Uri("agu://projectscene/openlabeleditor"),
                           "AGu/ProjectScene/tracksitemsview/labeleditor/LabelEditorDialog.qml");
        ir->registerQmlUri(muse::Uri("agu://projectscene/addnewlabeltrack"),
                           "AGu/ProjectScene/tracksitemsview/labeleditor/AddNewLabelTrackDialog.qml");
    }

    auto cr = globalIoc()->resolve<muse::rcommand::ICommandsRegister>(mname);
    if (cr) {
        cr->reg(std::make_shared<ProjectSceneCommandsRegister>());
    }
}

void ProjectSceneModule::registerUiTypes()
{
    // types
    qmlRegisterUncreatableType<TrackTypes>("AGu.ProjectScene", 1, 0, "TrackType", "Not creatable from QML");
    qmlRegisterUncreatableType<ClipKey>("AGu.ProjectScene", 1, 0, "ClipKey", "Not creatable from QML");
    qmlRegisterUncreatableType<ClipStyles>("AGu.ProjectScene", 1, 0, "ClipStyle", "Not creatable from QML");
    qmlRegisterUncreatableType<StereoHeightsPref>("AGu.ProjectScene", 1, 0, "AsymmetricStereoHeights", "Not creatable from QML");
    qmlRegisterUncreatableType<ClipBoundary>("AGu.ProjectScene", 1, 0, "ClipBoundaryAction", "Not creatable from QML");
    qmlRegisterUncreatableType<DirectionType>("AGu.ProjectScene", 1, 0, "Direction", "Not creatable from QML");

    // common
    qmlRegisterType<TracksViewStateModel>("AGu.ProjectScene", 1, 0, "TracksViewStateModel");
    qmlRegisterType<TrackViewStateModel>("AGu.ProjectScene", 1, 0, "TrackViewStateModel");
    qmlRegisterType<CustomCursor>("AGu.ProjectScene", 1, 0, "CustomCursor");
    qmlRegisterSingletonType<CustomCursorProvider>("AGu.ProjectScene", 1, 0, "CustomCursorProvider",
                                                   [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new CustomCursorProvider();
    });

    // toolbars
    qmlRegisterType<ProjectToolBarModel>("AGu.ProjectScene", 1, 0, "ProjectToolBarModel");
    qmlRegisterType<UndoRedoToolBarModel>("AGu.ProjectScene", 1, 0, "UndoRedoToolBarModel");
    qmlRegisterType<AudioSetupContextMenuModel>("AGu.ProjectScene", 1, 0, "AudioSetupContextMenuModel");

    qmlRegisterType<PlaybackToolBarModel>("AGu.ProjectScene", 1, 0, "PlaybackToolBarModel");
    qmlRegisterType<PlaybackToolBarCustomiseModel>("AGu.ProjectScene", 1, 0, "PlaybackToolBarCustomiseModel");
    qmlRegisterUncreatableType<PlaybackToolBarCustomiseItem>("AGu.ProjectScene", 1, 0, "PlaybackToolBarCustomiseItem",
                                                             "Cannot create");

    // tracks panel
    qmlRegisterType<PanelTracksListModel>("AGu.ProjectScene", 1, 0, "PanelTracksListModel");
    qmlRegisterType<AddEffectMenuModel>("AGu.ProjectScene", 1, 0, "AddEffectMenuModel");
    qmlRegisterType<RealtimeEffectListModel>("AGu.ProjectScene", 1, 0, "RealtimeEffectListModel");
    qmlRegisterType<RealtimeEffectListItemMenuModel>("AGu.ProjectScene", 1, 0, "RealtimeEffectListItemMenuModel");
    qmlRegisterType<RealtimeEffectRowActionsController>("AGu.ProjectScene", 1, 0, "RealtimeEffectRowActionsController");
    qmlRegisterType<RealtimeEffectSectionModel>("AGu.ProjectScene", 1, 0, "RealtimeEffectSectionModel");
    qmlRegisterType<TrackContextMenuModel>("AGu.ProjectScene", 1, 0, "TrackContextMenuModel");
    qmlRegisterType<VolumePressureMeterItem>("AGu.ProjectScene", 1, 0, "VolumePressureMeterItem");

    // clips view
    qmlRegisterType<ViewTracksListModel>("AGu.ProjectScene", 1, 0, "ViewTracksListModel");
    qmlRegisterType<TrackItemsMoveController>("AGu.ProjectScene", 1, 0, "TrackItemsMoveController");
    qmlRegisterType<TrackClipsListModel>("AGu.ProjectScene", 1, 0, "TrackClipsListModel");
    qmlRegisterUncreatableType<TrackClipItem>("AGu.ProjectScene", 1, 0, "TrackClipItem", "Not creatable from QML");
    qmlRegisterType<MouseHelper>("AGu.ProjectScene", 1, 0, "MouseHelper");
    qmlRegisterType<TrackLabelsListModel>("AGu.ProjectScene", 1, 0, "TrackLabelsListModel");
    qmlRegisterType<TrackLabelsLayoutManager>("AGu.ProjectScene", 1, 0, "TrackLabelsLayoutManager");
    qmlRegisterUncreatableType<TrackLabelItem>("AGu.ProjectScene", 1, 0, "TrackLabelItem", "Not creatable from QML");
    qmlRegisterType<WaveView>("AGu.ProjectScene", 1, 0, "WaveView");
    qmlRegisterType<ProjectOverview>("AGu.ProjectScene", 1, 0, "ProjectOverview");
    qmlRegisterType<ClipContextMenuModel>("AGu.ProjectScene", 1, 0, "ClipContextMenuModel");
    qmlRegisterType<MultiClipContextMenuModel>("AGu.ProjectScene", 1, 0, "MultiClipContextMenuModel");
    qmlRegisterType<LabelContextMenuModel>("AGu.ProjectScene", 1, 0, "LabelContextMenuModel");
    qmlRegisterType<SelectionContextMenuModel>("AGu.ProjectScene", 1, 0, "SelectionContextMenuModel");
    qmlRegisterType<CanvasContextMenuModel>("AGu.ProjectScene", 1, 0, "CanvasContextMenuModel");
    qmlRegisterType<SelectionViewController>("AGu.ProjectScene", 1, 0, "SelectionViewController");
    qmlRegisterType<PitchAndSpeedChangeModel>("AGu.ProjectScene", 1, 0, "PitchAndSpeedChangeModel");
    qmlRegisterType<SplitToolController>("AGu.ProjectScene", 1, 0, "SplitToolController");
    qmlRegisterType<DropController>("AGu.ProjectScene", 1, 0, "DropController");
    qmlRegisterType<LabelsTableViewModel>("AGu.ProjectScene", 1, 0, "LabelsTableViewModel");
    qmlRegisterUncreatableMetaObject(LabelsTableViewCellType::staticMetaObject,
                                     "AGu.ProjectScene", 1, 0, "LabelsTableViewCellType", "");
    qmlRegisterType<AddNewLabelTrackModel>("AGu.ProjectScene", 1, 0, "AddNewLabelTrackModel");

    // timeline
    qmlRegisterType<TimelineContext>("AGu.ProjectScene", 1, 0, "TimelineContext");
    qmlRegisterType<TimelineRuler>("AGu.ProjectScene", 1, 0, "TimelineRuler");
    qmlRegisterType<TimelineContextMenuModel>("AGu.ProjectScene", 1, 0, "TimelineContextMenuModel");
    qmlRegisterType<PlayRegionController>("AGu.ProjectScene", 1, 0, "PlayRegionController");
    qmlRegisterType<PlayRegionModel>("AGu.ProjectScene", 1, 0, "PlayRegionModel");

    // gridlines
    qmlRegisterType<GridLines>("AGu.ProjectScene", 1, 0, "GridLines");

    // play cursor
    qmlRegisterType<PlayCursorController>("AGu.ProjectScene", 1, 0, "PlayCursorController");

    // status bar
    qmlRegisterType<SelectionStatusModel>("AGu.ProjectScene", 1, 0, "SelectionStatusModel");

    // history panel
    qmlRegisterType<HistoryPanelModel>("AGu.ProjectScene", 1, 0, "HistoryPanelModel");

    //track ruler
    qmlRegisterType<TrackRulerModel>("AGu.ProjectScene", 1, 0, "TrackRulerModel");
}

void ProjectSceneModule::onInit(const muse::IApplication::RunMode&)
{
    m_configuration->init();
}

IContextSetup* ProjectSceneModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new ProjectSceneContext(ctx);
}

// =====================================================
// ProjectSceneContext
// =====================================================

void ProjectSceneContext::registerExports()
{
    m_projectSceneActionsController = std::make_shared<ProjectSceneActionsController>(iocContext());
    m_uiActions = std::make_shared<ProjectSceneUiActions>(iocContext(), m_projectSceneActionsController);
    m_realtimeEffectPanelTrackSelection = std::make_shared<RealtimeEffectPanelTrackSelection>(iocContext());
    m_uiState = std::make_shared<ProjectSceneUiState>(iocContext());

    ioc()->registerExport<IProjectSceneActionsController>(mname, m_projectSceneActionsController);
    ioc()->registerExport<IProjectSceneUiState>(mname, m_uiState);
    ioc()->registerExport<IRealtimeEffectPanelTrackSelection>(mname, m_realtimeEffectPanelTrackSelection);
    ioc()->registerExport<IWavePainter>(mname, std::make_shared<WavePainterProxy>(iocContext()));
    ioc()->registerExport<IConnectingDotsPainter>(mname, std::make_shared<ConnectingDotsPainter>(iocContext()));
    ioc()->registerExport<IMinMaxRMSPainter>(mname, std::make_shared<MinMaxRMSPainter>(iocContext()));
    ioc()->registerExport<ISamplesPainter>(mname, std::make_shared<SamplesPainter>(iocContext()));
}

void ProjectSceneContext::resolveImports()
{
    auto cs = ioc()->resolve<muse::rcommand::ICommandsState>(mname);
    if (cs) {
        cs->reg(std::make_shared<ProjectSceneCommandsState>(iocContext()));
    }
}

void ProjectSceneContext::onInit(const muse::IApplication::RunMode& mode)
{
    if (mode != muse::IApplication::RunMode::GuiApp) {
        return;
    }

    m_uiActions->init();
    m_projectSceneActionsController->init();
    m_realtimeEffectPanelTrackSelection->init();
    m_uiState->init();

    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(m_uiActions);
    }
}

void ProjectSceneContext::onDeinit()
{
}
