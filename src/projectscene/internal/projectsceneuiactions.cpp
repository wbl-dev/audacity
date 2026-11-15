/*
* Audacity: A Digital Audio Editor
*/
#include "projectsceneuiactions.h"

#include "ui/view/iconcodes.h"
#include "context/uicontext.h"
#include "context/shortcutcontext.h"
#include "types/translatablestring.h"

#include "log.h"

using namespace agu::projectscene;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

static UiActionList STATIC_ACTIONS = {
    UiAction("clip-gain",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Clip gain"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Clip gain"),
             IconCode::Code::AUTOMATION
             ),
    UiAction("split-tool",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_FOCUSED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Split tool"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Split tool"),
             IconCode::Code::SPLIT_TOOL
             ),
    UiAction("zoom-in",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Zoom in"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Zoom in"),
             IconCode::Code::ZOOM_IN
             ),
    UiAction("zoom-out",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Zoom out"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Zoom out"),
             IconCode::Code::ZOOM_OUT
             ),
    UiAction("zoom-default",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Zoom default"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Zoom default")
             ),
    UiAction("zoom-to-selection",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Zoom to selection"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Zoom to selection"),
             IconCode::Code::FIT_SELECTION
             ),
    UiAction("zoom-to-fit-project",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Zoom to fit project"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Zoom to fit project"),
             IconCode::Code::FIT_PROJECT
             ),
    UiAction("zoom-toggle",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Zoom toggle"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Zoom toggle"),
             IconCode::Code::ZOOM_TOGGLE
             ),
    UiAction("timeline-page-left",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_FOCUSED,
             TranslatableString("action", "Scroll timeline left one page"),
             TranslatableString("action_description", "Scroll the timeline left by one screen width")
             ),
    UiAction("timeline-page-right",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_FOCUSED,
             TranslatableString("action", "Scroll timeline right one page"),
             TranslatableString("action_description", "Scroll the timeline right by one screen width")
             ),
    UiAction("center-view-on-playhead",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Center view on playhead"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Center view on playhead")
             ),
    UiAction("action://trackedit/global-view-spectrogram",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_FOCUSED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Toggle spectral view"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Toggle spectral view"),
             IconCode::Code::SPECTROGRAM
             ),
    UiAction("spectral-box-select",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Spectral box select"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Spectral box select"),
             IconCode::Code::SPECTROGRAM_BOX_SELECTION
             ),
    UiAction("spectral-brush",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Spectral box select"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Spectral box select"),
             IconCode::Code::BRUSH
             ),
    UiAction("snap",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_FOCUSED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Snapping"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Snapping"),
             IconCode::Code::MAGNET
             ),
    UiAction("minutes-seconds-ruler",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Minutes && seconds"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Minutes && seconds"),
             Checkable::Yes
             ),
    UiAction("beats-measures-ruler",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Beats && measures"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Beats && measures"),
             Checkable::Yes
             ),
    UiAction("toggle-vertical-rulers",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Show vertical rulers"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show vertical rulers"),
             Checkable::Yes
             ),
    UiAction("show-master-track",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Show master track"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show master track"),
             Checkable::Yes
             ),
    UiAction("toggle-update-display-while-playing",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Update display while playing"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Update display while playing"),
             Checkable::Yes
             ),
    UiAction("toggle-pinned-play-head",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Pinned playhead"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Pinned playhead"),
             Checkable::Yes
             ),
    UiAction("toggle-playback-on-ruler-click-enabled",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Click ruler to start playback"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Click ruler to start playback"),
             Checkable::Yes
             ),
    // clip
    UiAction("clip-properties",
             agu::context::UiCtxUnknown,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Clip properties"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show clip properties")
             ),
    UiAction("action://delete",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Delete"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Delete"),
             IconCode::Code::DELETE_TANK,
             Checkable::Yes
             ),
    UiAction("action://trackedit/clip/change-color-auto",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Follow track color"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Follow track color"),
             Checkable::Yes
             ),
    UiAction("play-position-decrease",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_FOCUSED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Move playhead left"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Move playhead left")
             ),
    UiAction("play-position-increase",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_FOCUSED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Move playhead right"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Move playhead right")
             ),
    UiAction("sel-ext-left",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Extend selection left"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Extend selection left")
             ),
    UiAction("sel-ext-right",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Extend selection right"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Extend selection right")
             ),
    UiAction("sel-cntr-left",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Contract selection from left"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Contract selection from left")
             ),
    UiAction("sel-cntr-right",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Contract selection from right"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Contract selection from right")
             ),
    UiAction("curs-sel-start",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Move playhead to selection start"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Move playhead to selection start")
             ),
    UiAction("curs-sel-end",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Move playhead to selection end"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Move playhead to selection end")
             ),
    UiAction("clip-pitch-speed",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Pitch and speed"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Pitch and speed")
             ),
    UiAction("toggle-rms-in-waveform",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Show RMS in waveform"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show RMS in waveform"),
             Checkable::Yes
             ),
    UiAction("toggle-clipping-in-waveform",
             agu::context::UiCtxProjectOpened,
             muse::shortcuts::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Show clipping in waveform"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show clipping in waveform"),
             Checkable::Yes
             ),
    UiAction("action://projectscene/track-view-half-wave",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Half-wave"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Half-wave"),
             IconCode::Code::WAVEFORM_HALFWAVE,
             Checkable::Yes
             ),
    UiAction("open-label-editor",
             agu::context::UiCtxProjectOpened,
             agu::context::CTX_PROJECT_OPENED,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Show label editor"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Show label editor")
             ),
    UiAction("realtime-effect-move-up",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Move realtime effect up"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Move realtime effect up")
             ),
    UiAction("realtime-effect-move-down",
             agu::context::UiCtxAny,
             agu::context::CTX_ANY,
             //: Action title: shown as a menu item or a button label; keep it short
             TranslatableString("action", "Move realtime effect down"),
             //: Action description: shown as a tooltip; can be a full sentence
             TranslatableString("action_description", "Move realtime effect down")
             )
};

ProjectSceneUiActions::ProjectSceneUiActions(const muse::modularity::ContextPtr& ctx,
                                             std::shared_ptr<ProjectSceneActionsController> controller)
    : muse::Contextable(ctx), m_controller(controller)
{
    m_actions = STATIC_ACTIONS;
}

void ProjectSceneUiActions::init()
{
    const auto& colorInfos = configuration()->clipColorInfos();
    m_actions.clear();
    m_actions.reserve(2 * colorInfos.size() + STATIC_ACTIONS.size());

    for (const auto& info : colorInfos) {
        muse::Color resolved = configuration()->clipColor(info.index);

        UiAction clipColorAction;
        clipColorAction.code = muse::actions::ActionQuery(makeClipColorChangeAction(info.index)).toString();
        clipColorAction.uiCtx = context::UiCtxProjectOpened;
        clipColorAction.scCtx = context::CTX_PROJECT_FOCUSED;
        clipColorAction.description = muse::TranslatableString("action_description", "Change clip color");
        clipColorAction.title = muse::TranslatableString("action", "Change clip color");
        clipColorAction.iconCode = IconCode::Code::FRETBOARD_MARKER_CIRCLE_FILLED;
        clipColorAction.iconColor = QString::fromStdString(resolved.toString());
        clipColorAction.checkable = Checkable::Yes;

        m_actions.push_back(std::move(clipColorAction));

        UiAction trackColorAction;
        trackColorAction.code = muse::actions::ActionQuery(makeTrackColorChangeAction(info.index)).toString();
        trackColorAction.uiCtx = context::UiCtxProjectOpened;
        trackColorAction.scCtx = context::CTX_DISABLED;
        trackColorAction.description = muse::TranslatableString("action_description", "Change track color");
        trackColorAction.title = muse::TranslatableString("action", "Change track color");
        trackColorAction.iconCode = IconCode::Code::FRETBOARD_MARKER_CIRCLE_FILLED;
        trackColorAction.iconColor = QString::fromStdString(resolved.toString());
        trackColorAction.checkable = Checkable::Yes;

        m_actions.push_back(std::move(trackColorAction));
    }

    m_actions.insert(m_actions.end(), STATIC_ACTIONS.begin(), STATIC_ACTIONS.end());

    m_controller->actionCheckedChanged().onReceive(this, [this](const ActionCode& code) {
        m_actionCheckedChanged.send({ code });
    });
    m_controller->actionEnabledChanged().onReceive(this, [this](const ActionCode& code) {
        m_actionEnabledChanged.send({ code });
    });
}

const UiActionList& ProjectSceneUiActions::actionsList() const
{
    return m_actions;
}

bool ProjectSceneUiActions::actionEnabled(const muse::ui::UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

muse::async::Channel<ActionCodeList> ProjectSceneUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

bool ProjectSceneUiActions::actionChecked(const muse::ui::UiAction& act) const
{
    return m_controller->actionChecked(act.code);
}

muse::async::Channel<ActionCodeList> ProjectSceneUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
