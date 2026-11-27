/*
* Audacity: A Digital Audio Editor
*/
#include "playbacktoolbarmodel.h"

#include "framework/global/containers.h"
#include "framework/global/log.h"

#include "playback/view/toolbars/playbacktoolbarlevelitem.h"
#include "playback/view/toolbars/playbacktoolbarcontrolitem.h"
#include "playback/view/toolbars/playbacktoolbartimeitem.h"
#include "playback/view/toolbars/playbacktoolbarbpmitem.h"
#include "playback/view/toolbars/playbacktoolbartimesignatureitem.h"
#include "record/qml/AGu/Record/playbacktoolbarrecordlevelitem.h"
#include "projectscene/view/toolbars/snaptoolbaritem.h"

#include "playback/playbackcommands.h"
#include "record/recordcommands.h"
#include "projectscene/projectscenecommands.h"
#include "trackedit/trackeditcommands.h"

using namespace muse::uicomponents;
using namespace muse::ui;
using namespace muse::rcommand;
using namespace agu::projectscene;
using namespace agu::playback;

static PlaybackToolBarModel::ItemType itemType(const Command& command)
{
    static const std::map<Command, PlaybackToolBarModel::ItemType> types = {
        { PLAYBACK_LEVEL_COMMAND, PlaybackToolBarModel::PLAYBACK_LEVEL },
        { PLAYBACK_TIME_COMMAND, PlaybackToolBarModel::PLAYBACK_TIME },
        { PLAYBACK_BPM_COMMAND, PlaybackToolBarModel::PLAYBACK_BPM },
        { PLAYBACK_TIME_SIGNATURE_COMMAND, PlaybackToolBarModel::PLAYBACK_TIME_SIGNATURE },
        { agu::record::RECORD_LEVEL_COMMAND, PlaybackToolBarModel::RECORD_LEVEL },
        { PLAYBACK_TOGGLE_PLAY_PAUSE_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { PLAYBACK_STOP_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { agu::record::RECORD_START_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { PLAYBACK_REWIND_START_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { PLAYBACK_REWIND_END_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { PROJECTSCENE_TIME_SELECTION_TOOL_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { PROJECTSCENE_TOGGLE_CLIP_GAIN_AUTOMATION_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { PROJECTSCENE_TOGGLE_SPLIT_TOOL_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { agu::trackedit::TRACKEDIT_GLOBAL_VIEW_SPECTROGRAM_COMMAND, PlaybackToolBarModel::PLAYBACK_CONTROL },
        { PROJECTSCENE_SNAP_COMMAND, PlaybackToolBarModel::SNAP }
    };

    return muse::value(types, command, PlaybackToolBarModel::PROJECT_CONTROL);
}

PlaybackToolBarModel::PlaybackToolBarModel(QObject* parent)
    : AbstractToolBarModel(parent)
{
}

void PlaybackToolBarModel::load()
{
    if (!m_inited) {
        uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
            reload();
        });

        context()->currentProjectChanged().onNotify(this, [this]() {
            reload();
            emit isEnabledChanged();
            if (context()->currentProject()) {
                setupProjectConnections(*context()->currentProject());
            }
        });
        if (context()->currentProject()) {
            setupProjectConnections(*context()->currentProject());
        }

        playbackController()->isPlayingChanged().onNotify(this, [this]() {
            updatePlayState();
        });

        recordController()->isRecordingChanged().onNotify(this, [this]() {
            updatePlayState();
            updateRecordState();
        });

        m_inited = true;
    }

    updateActions();

    AbstractToolBarModel::load();
    updateStates();
}

void PlaybackToolBarModel::reload()
{
    load();
}

void PlaybackToolBarModel::setupProjectConnections(project::IAGuProject& project)
{
    const auto vs = project.viewState();
    vs->clipGainAutomationEnabled().ch.onReceive(this, [this](bool){ updateClipGainAutomationState(); });
    vs->splitToolEnabled().ch.onReceive(this, [this](bool){ updateSplitState(); });
    vs->globalSpectrogramToggleIsOnChanged().onNotify(this, [this] { updateGlobalSpectrogramViewState(); });
}

void PlaybackToolBarModel::onCommandStateChanged(const Command& command, const CommandState& state)
{
    if (uicontextResolver()->currentUiContext() == context::UiCtxDialogOpened) {
        return;
    }

    AbstractToolBarModel::onCommandStateChanged(command, state);
}

void PlaybackToolBarModel::updateStates()
{
    updatePlayState();
    updateStopState();
    updateRecordState();
    updateTimeSelectionState();
    updateClipGainAutomationState();
    updateSplitState();
    updateGlobalSpectrogramViewState();
}

void PlaybackToolBarModel::updatePlayState()
{
    PlaybackToolBarControlItem* item = dynamic_cast<PlaybackToolBarControlItem*>(findItemPtr(PLAYBACK_TOGGLE_PLAY_PAUSE_COMMAND));

    if (item == nullptr) {
        return;
    }

    bool isPlaying = playbackController()->isPlaying();
    bool isPaused = playbackController()->isPaused();
    bool isRecording = recordController()->isRecording();
    bool isLeadIn = recordController()->isLeadInRecording();

    // During recording it pauses the recorder
    CommandInfo info = commandsRegister()->commandInfo(PLAYBACK_TOGGLE_PLAY_PAUSE_COMMAND);

    // During lead-in, show as playing (green background) since audio is playing back
    bool showAsPlaying = isPlaying || isLeadIn;

    bool leadInPaused = isLeadIn && isPaused;
    if ((showAsPlaying && !leadInPaused) || (isRecording && !isLeadIn)) {
        info.decoration.iconCode = IconCode::Code::PAUSE_FILL;
    }
    item->setCommandInfo(info);
    item->setSelected(showAsPlaying);

    QColor iconColor = themeColor(PLAY_COLOR);
    QColor backgroundColor = themeColor(BUTTON_COLOR);
    if (showAsPlaying) {
        iconColor = themeColor(FONT_PRIMARY_COLOR);
        backgroundColor = themeColor(PLAY_COLOR);
    } else if (isRecording) {
        iconColor = themeColor(FONT_PRIMARY_COLOR);
    }

    item->setIconColor(iconColor);
    item->setBackgroundColor(backgroundColor);
}

void PlaybackToolBarModel::updateStopState()
{
    PlaybackToolBarControlItem* item = dynamic_cast<PlaybackToolBarControlItem*>(findItemPtr(PLAYBACK_STOP_COMMAND));

    if (item == nullptr) {
        return;
    }

    item->setCommandInfo(commandsRegister()->commandInfo(PLAYBACK_STOP_COMMAND));
    const QColor iconColor = themeColor(FONT_PRIMARY_COLOR);
    item->setIconColor(iconColor);
}

void PlaybackToolBarModel::updateRecordState()
{
    PlaybackToolBarControlItem* item = dynamic_cast<PlaybackToolBarControlItem*>(findItemPtr(record::RECORD_START_COMMAND));

    if (item == nullptr) {
        return;
    }

    bool isRecording = recordController()->isRecording();
    bool isLeadIn = recordController()->isLeadInRecording();

    // During lead-in pre-roll, the button should not appear as actively recording
    item->setSelected(isRecording && !isLeadIn);

    QColor iconColor = themeColor(RECORD_COLOR);
    QColor backgroundColor = themeColor(BUTTON_COLOR);
    if (isRecording && !isLeadIn) {
        iconColor = themeColor(BACKGROUND_PRIMARY_COLOR);
        backgroundColor = themeColor(RECORD_COLOR);
    }

    item->setIconColor(iconColor);
    item->setBackgroundColor(backgroundColor);
}

void PlaybackToolBarModel::updateTimeSelectionState()
{
    const auto project = context()->currentProject();
    if (!project) {
        return;
    }

    const auto viewState = project->viewState();
    updateToggleState(PROJECTSCENE_TIME_SELECTION_TOOL_COMMAND,
                      !viewState->splitToolEnabled().val);
}

void PlaybackToolBarModel::updateToggleState(const Command& command, bool isOn)
{
    PlaybackToolBarControlItem* item = dynamic_cast<PlaybackToolBarControlItem*>(findItemPtr(command));

    if (item == nullptr) {
        return;
    }

    item->setSelected(isOn);
    auto state = item->commandState();
    state.checked = isOn;
    item->setCommandState(state);
    item->setBackgroundColor(themeColor(isOn ? ACCENT_COLOR : BUTTON_COLOR));
}

void PlaybackToolBarModel::updateClipGainAutomationState()
{
    auto prj = context()->currentProject();

    if (!prj) {
        return;
    }

    updateToggleState(PROJECTSCENE_TOGGLE_CLIP_GAIN_AUTOMATION_COMMAND, prj->viewState()->clipGainAutomationEnabled().val);
    updateTimeSelectionState();
}

void PlaybackToolBarModel::updateSplitState()
{
    auto prj = context()->currentProject();

    if (!prj) {
        return;
    }

    updateToggleState(PROJECTSCENE_TOGGLE_SPLIT_TOOL_COMMAND, prj->viewState()->splitToolEnabled().val);
    updateTimeSelectionState();
}

void PlaybackToolBarModel::updateGlobalSpectrogramViewState()
{
    auto prj = context()->currentProject();

    if (!prj) {
        return;
    }

    updateToggleState(trackedit::TRACKEDIT_GLOBAL_VIEW_SPECTROGRAM_COMMAND, prj->viewState()->globalSpectrogramToggleIsOn());
}

QColor PlaybackToolBarModel::themeColor(muse::ui::ThemeStyleKey key) const
{
    return uiConfiguration()->currentTheme().values.value(key).value<QColor>();
}

void PlaybackToolBarModel::updateActions()
{
    ToolBarItemList items;

    // The workspace has a fixed set of controls. Legacy toolbar and meter
    // layout preferences must not hide transport or the master volume.
    static const std::vector<Command> commands {
        PLAYBACK_REWIND_START_COMMAND,
        PLAYBACK_TOGGLE_PLAY_PAUSE_COMMAND,
        PLAYBACK_STOP_COMMAND,
        record::RECORD_START_COMMAND,
        {},
        PLAYBACK_TIME_COMMAND,
        {},
        PROJECTSCENE_TIME_SELECTION_TOOL_COMMAND,
        PROJECTSCENE_TOGGLE_CLIP_GAIN_AUTOMATION_COMMAND,
        PROJECTSCENE_TOGGLE_SPLIT_TOOL_COMMAND,
        {},
        PROJECTSCENE_ZOOM_OUT_COMMAND,
        PROJECTSCENE_ZOOM_IN_COMMAND,
        PROJECTSCENE_ZOOM_TO_FIT_PROJECT_COMMAND,
        {},
        PROJECTSCENE_SNAP_COMMAND,
        {},
        PLAYBACK_LEVEL_COMMAND
    };

    for (const Command& command : commands) {
        if (!command.isValid()) {
            items << AbstractToolBarModel::makeSeparator();
            continue;
        }

        ToolBarItem* item = makeLocalItem(command);
        if (!item) {
            continue;
        }

        item->setIsTransparent(true);

        items << item;
    }

    setItems(items);
}

ToolBarItem* PlaybackToolBarModel::makeLocalItem(const Command& command)
{
    PlaybackToolBarModel::ItemType type = itemType(command);

    if (type == PlaybackToolBarModel::PROJECT_CONTROL) {
        ToolBarItem* item = AbstractToolBarModel::makeItem(command);
        if (item) {
            item->setType(static_cast<ToolBarItemType::Type>(type));
        }
        return item;
    }

    const CommandInfo& info = commandsRegister()->commandInfo(command);
    if (!info.isValid()) {
        LOGW() << "not found command: " << command;
        return nullptr;
    }

    ToolBarItem* result = nullptr;

    switch (type) {
    case PlaybackToolBarModel::PLAYBACK_LEVEL:
        result = new PlaybackToolBarLevelItem(info, static_cast<ToolBarItemType::Type>(type), this);
        break;
    case PlaybackToolBarModel::PLAYBACK_TIME:
        result = new PlaybackToolBarTimeItem(info, static_cast<ToolBarItemType::Type>(type), this);
        break;
    case PlaybackToolBarModel::PLAYBACK_BPM:
        result = new PlaybackToolBarBPMItem(info, static_cast<ToolBarItemType::Type>(type), this);
        break;
    case PlaybackToolBarModel::PLAYBACK_TIME_SIGNATURE:
        result = new PlaybackToolBarTimeSignatureItem(info, static_cast<ToolBarItemType::Type>(type), this);
        break;
    case PlaybackToolBarModel::RECORD_LEVEL:
        result = new record::PlaybackToolBarRecordLevelItem(info, static_cast<ToolBarItemType::Type>(type), this);
        break;
    case PlaybackToolBarModel::PLAYBACK_CONTROL: {
        PlaybackToolBarControlItem* item = new PlaybackToolBarControlItem(info, static_cast<ToolBarItemType::Type>(type), this);
        item->setIconColor(themeColor(FONT_PRIMARY_COLOR));
        item->setBackgroundColor(themeColor(BUTTON_COLOR));
        result = std::move(item);
        break;
    }
    case PlaybackToolBarModel::SNAP:
        result = new projectscene::SnapToolBarItem(info, static_cast<ToolBarItemType::Type>(type), this);
        break;
    default:
        break;
    }

    if (result) {
        result->setCommandState(commandsState()->commandState(command));
    }

    return result;
}

bool PlaybackToolBarModel::isEnabled() const
{
    return context()->currentProject() != nullptr;
}
