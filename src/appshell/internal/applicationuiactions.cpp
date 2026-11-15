/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Audacity-CLA-applies
 *
 * Audacity
 * A Digital Audio Editor
 *
 * Copyright (C) 2024 Audacity BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "applicationuiactions.h"

#include <algorithm>

#include "ui/view/iconcodes.h"
#include "context/uicontext.h"
#include "context/shortcutcontext.h"

#include "dockwindow/idockwindow.h"
#include "async/notification.h"

#include "log.h"
#include "../appshellcommands.h"

using namespace agu::appshell;

using namespace muse;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::dock;

static const ActionCode FULL_SCREEN_CODE("fullscreen");

const UiActionList ApplicationUiActions::m_actions = {
    UiAction("quit",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Exit"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Exit")
             ),
    UiAction("restart",
             agu::context::UiCtxAny,
             agu::context::CTX_DISABLED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Restart")
             ),
    UiAction(FULL_SCREEN_CODE,
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Full screen"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Full screen"),
             Checkable::Yes
             ),
    UiAction("about-agu",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&About AGu…")
             ),
    UiAction("about-qt",
             agu::context::UiCtxAny,
             agu::context::CTX_DISABLED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "About &Qt…")
             ),
    UiAction("revert-factory",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Revert to &factory settings"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Revert to factory settings")
             ),

    // Docking
    UiAction("dock-restore-default-layout",
             agu::context::UiCtxAny,
             agu::context::CTX_DISABLED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Restore the &default layout"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Restore the default layout")
             ),

    // Toolbars
    UiAction("toggle-transport",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Playback controls"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show/hide playback controls"),
             Checkable::Yes
             ),
    // Vertical panels
    UiAction("toggle-tracks",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Tracks"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show/hide tracks"),
             Checkable::Yes
             ),
    UiAction("toggle-macro-manager",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Macro manager"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show/hide macro manager"),
             Checkable::Yes
             ),
    // Status bar
    UiAction("toggle-statusbar",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Status bar"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show/hide status bar"),
             Checkable::Yes
             ),

    UiAction("preference-dialog",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Preferences"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Preferences…")
             ),
    UiAction("action://copy",
             { "action://trackedit/copy" },
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Copy"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Copy"),
             IconCode::Code::COPY
             ),
    UiAction("action://cut",
             { "action://trackedit/cut" },
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Cut"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Cut"),
             IconCode::Code::CUT
             ),
    UiAction("action://paste",
             { "action://trackedit/paste-default" },
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Paste"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Paste"),
             IconCode::Code::PASTE
             ),
    UiAction("action://undo",
             { "action://trackedit/undo" },
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Undo"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Undo"),
             IconCode::Code::UNDO
             ),
    UiAction("action://redo",
             { "action://trackedit/redo" },
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Redo"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Redo"),
             IconCode::Code::REDO
             ),
    UiAction("action://delete",
             { "action://trackedit/delete" },
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "De&lete"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Delete"),
             IconCode::Code::DELETE_TANK
             ),
    UiAction("action://cancel",
             agu::context::UiCtxAny,
             agu::context::CTX_DISABLED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Cancel"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Cancel"),
             IconCode::Code::DELETE_TANK
             ),
    UiAction("action://trigger",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Trigger"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Trigger")
             ),
    UiAction("action://enter",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "&Enter"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Trigger the focused control or select the focused track item")
             ),
    UiAction("action://shift-enter",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             TranslatableString("action", "Shift+&Enter"),
             TranslatableString("action", "Trigger the focused control or make a range selection of track items")
             ),
    UiAction("action://context-menu",
             agu::context::UiCtxProjectFocused,
             agu::context::CTX_PROJECT_FOCUSED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Open item context menu"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Open the context menu of the focused item")
             ),
};

ApplicationUiActions::ApplicationUiActions(const muse::modularity::ContextPtr& ctx, std::shared_ptr<ApplicationActionController> controller)
    : muse::Contextable(ctx), m_controller(controller)
{
}

void ApplicationUiActions::init()
{
    mainWindow()->isFullScreenChanged().onNotify(this, [this]() {
        m_actionCheckedChanged.send({ FULL_SCREEN_CODE });
    });

    dockWindowProvider()->windowChanged().onNotify(this, [this]() {
        listenOpenedDocksChanged(dockWindowProvider()->window());
    });

    recordController()->isRecordingChanged().onNotify(this, [this]() {
        m_actionEnabledChanged.send(m_controller->prohibitedActionsWhileRecording());
    });
}

void ApplicationUiActions::listenOpenedDocksChanged(IDockWindow* window)
{
    if (!window) {
        return;
    }

    window->docksOpenStatusChanged().onReceive(this, [this](const QStringList& dockNames) {
        ActionCodeList actions;

        for (const ActionCode& toggleDockAction : toggleDockActions().keys()) {
            const DockName& dockName = toggleDockActions()[toggleDockAction];

            if (dockNames.contains(dockName)) {
                actions.push_back(toggleDockAction);
            }
        }

        if (!actions.empty()) {
            m_actionCheckedChanged.send(actions);
        }
    });
}

const muse::ui::UiActionList& ApplicationUiActions::actionsList() const
{
    return m_actions;
}

bool ApplicationUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller) {
        return true;
    }
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool ApplicationUiActions::actionChecked(const UiAction& act) const
{
    if (act.code == FULL_SCREEN_CODE) {
        return mainWindow()->isFullScreen();
    }

    QMap<ActionCode, DockName> toggleDockActions = ApplicationUiActions::toggleDockActions();
    DockName dockName = toggleDockActions.value(act.code, DockName());

    if (dockName.isEmpty()) {
        return false;
    }

    const IDockWindow* window = dockWindowProvider()->window();
    return window ? window->isDockOpen(dockName) : false;
}

muse::async::Channel<muse::actions::ActionCodeList> ApplicationUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

muse::async::Channel<muse::actions::ActionCodeList> ApplicationUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

const QMap<muse::actions::ActionCode, DockName>& ApplicationUiActions::toggleDockActions()
{
    static const QMap<muse::actions::ActionCode, DockName> actionsMap {
        { "toggle-transport", PLAYBACK_TOOLBAR_NAME },

        { "toggle-tracks", TRACKS_PANEL_NAME },
        { "toggle-history", HISTORY_PANEL_NAME },
        { "toggle-macro-manager", MACRO_MANAGER_PANEL_NAME },

        { "toggle-statusbar", PROJECT_STATUSBAR_NAME },
    };

    return actionsMap;
}
