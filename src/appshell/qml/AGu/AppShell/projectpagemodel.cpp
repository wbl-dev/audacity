/*
* Audacity: A Digital Audio Editor
*/
#include "projectpagemodel.h"

#include "framework/global/log.h"
#include "framework/global/async/async.h"
#include "framework/dockwindow/dockcommands.h"

#include "playback/playbackcommands.h"
#include "projectscene/view/toolbars/playbacktoolbarconfig.h"

using namespace agu::appshell;
using namespace muse;

ProjectPageModel::ProjectPageModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void ProjectPageModel::init()
{
    TRACEFUNC;

    if (!m_inited) {
        applicationActionController()->dockToggleRequested().onReceive(this, [this](const DockName& dockName) {
            toggleDock(dockName);
        });

        m_inited = true;
    }

    updatePlaybackMeterVisibility();
}

void ProjectPageModel::updatePlaybackMeterVisibility()
{
    // The fixed transport includes its own master meter. Old toolbar layout
    // preferences must not restore a second meter beside the tracks.
    muse::async::Async::call(this, [this]() {
        commandDispatcher()->dispatch(dock::DOCK_SET_OPEN_COMMAND, {
            { "dock_name", Val(playbackMeterPanelName().toStdString()) },
            { "open", Val(false) },
        });
    });
}

QString ProjectPageModel::projectToolBarName() const
{
    return PROJECT_TOOLBAR_NAME;
}

QString ProjectPageModel::playbackToolBarName() const
{
    return PLAYBACK_TOOLBAR_NAME;
}

QString ProjectPageModel::undoRedoToolBarName() const
{
    return UNDO_REDO_TOOLBAR_NAME;
}

QString ProjectPageModel::tracksPanelName() const
{
    return TRACKS_PANEL_NAME;
}

QString ProjectPageModel::historyPanelName() const
{
    return HISTORY_PANEL_NAME;
}

QString ProjectPageModel::playbackMeterPanelName() const
{
    return PLAYBACK_METER_PANEL_NAME;
}

QString ProjectPageModel::macroManagerPanelName() const
{
    return MACRO_MANAGER_PANEL_NAME;
}

QString ProjectPageModel::statusBarName() const
{
    return PROJECT_STATUSBAR_NAME;
}

void ProjectPageModel::toggleDock(const QString& name)
{
    commandDispatcher()->dispatch(dock::DOCK_TOGGLE_COMMAND, { { "dock_name", Val(name.toStdString()) } });
}
