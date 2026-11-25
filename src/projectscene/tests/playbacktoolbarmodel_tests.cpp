/*
 * Audacity: A Digital Audio Editor
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "view/toolbars/playbacktoolbarmodel.h"
#include "uicomponents/qml/Muse/UiComponents/toolbaritem.h"
#include "rcommand/tests/mocks/commandsregistermock.h"
#ifdef MUSE_MODULE_SHORTCUTS_V2
#include "shortcuts_v2/tests/mocks/commandshortcutsregistermock.h"
#endif
#include "ui/tests/mocks/uiconfigurationmock.h"
#include "ui/tests/mocks/uistatemock.h"

#include "playback/playbackcommands.h"
#include "record/recordcommands.h"

using namespace ::testing;

namespace agu::projectscene {
struct PlaybackToolBarTestAccess {
    static void configure(PlaybackToolBarModel& model,
                          const std::shared_ptr<muse::ui::IUiState>& state,
                          const std::shared_ptr<muse::ui::IUiConfiguration>& configuration)
    {
        model.uiState.set(state);
        model.uiConfiguration.set(configuration);
    }

    static void updateActions(PlaybackToolBarModel& model)
    {
        model.updateActions();
    }
};

namespace {
#ifndef MUSE_MODULE_SHORTCUTS_V2
class ShortcutsRegisterMock : public muse::shortcuts::IShortcutsRegister
{
public:
    MOCK_METHOD(const muse::shortcuts::ShortcutList&, shortcuts, (), (const, override));
    MOCK_METHOD(muse::Ret, setShortcuts, (const muse::shortcuts::ShortcutList&), (override));
    MOCK_METHOD(void, resetShortcuts, (), (override));
    MOCK_METHOD(muse::async::Notification, shortcutsChanged, (), (const, override));
    MOCK_METHOD(muse::Ret, setAdditionalShortcuts, (const std::string&, const muse::shortcuts::ShortcutList&), (override));
    MOCK_METHOD(const muse::shortcuts::Shortcut&, shortcut, (const std::string&), (const, override));
    MOCK_METHOD(const muse::shortcuts::Shortcut&, defaultShortcut, (const std::string&), (const, override));
    MOCK_METHOD(bool, isRegistered, (const std::string&), (const, override));
    MOCK_METHOD(muse::shortcuts::ShortcutList, shortcutsForSequence, (const std::string&), (const, override));
    MOCK_METHOD(muse::Ret, importFromFile, (const muse::io::path_t&), (override));
    MOCK_METHOD(muse::Ret, exportToFile, (const muse::io::path_t&), (const, override));
    MOCK_METHOD(bool, active, (), (override));
    MOCK_METHOD(void, setActive, (bool), (override));
    MOCK_METHOD(muse::async::Notification, activeChanged, (), (const, override));
    MOCK_METHOD(void, reload, (bool), (override));
};
#endif

class CommandsStateStub : public muse::rcommand::ICommandsState
{
public:
    void reg(const muse::rcommand::IModuleCommandsStatePtr&) override {}
    void unreg(const muse::rcommand::IModuleCommandsStatePtr&) override {}
    muse::rcommand::CommandState commandState(const muse::rcommand::Command&) const override { return { true }; }
    muse::async::Channel<muse::rcommand::Command, muse::rcommand::CommandState> commandStateChanged() const override { return {}; }
};

// Only register transport commands: the test exercises the real toolbar model
// without starting the timecode, meters or project-specific editing items.
TEST(PlaybackToolBarModelTests, LegacyCustomizationCannotHideTransportOrRestoreLoop)
{
    auto savedState = std::make_shared<NiceMock<muse::ui::UiStateMock> >();
    muse::ui::ToolConfig legacyConfig;
    legacyConfig.items = { { playback::PLAYBACK_TOGGLE_LOOP_REGION_COMMAND, true } };
    ON_CALL(*savedState, toolConfig(_, _)).WillByDefault(Return(legacyConfig));

    auto configuration = std::make_shared<NiceMock<muse::ui::UiConfigurationMock> >();
    muse::ui::ThemeInfo theme;
    ON_CALL(*configuration, currentTheme()).WillByDefault(ReturnRef(theme));

    auto commands = std::make_shared<NiceMock<muse::rcommand::CommandsRegisterMock> >();
    std::map<muse::rcommand::Command, muse::rcommand::CommandInfo> infos;
    for (const auto& command : { playback::PLAYBACK_REWIND_START_COMMAND, playback::PLAYBACK_TOGGLE_PLAY_PAUSE_COMMAND,
                                playback::PLAYBACK_STOP_COMMAND, record::RECORD_START_COMMAND,
                                playback::PLAYBACK_TOGGLE_LOOP_REGION_COMMAND }) {
        infos[command].command = command;
    }
    muse::rcommand::CommandInfo unavailable;
    ON_CALL(*commands, commandInfo(_)).WillByDefault(Invoke([&](const auto& command) -> const auto& {
        auto it = infos.find(command);
        return it != infos.end() ? it->second : unavailable;
    }));

#ifdef MUSE_MODULE_SHORTCUTS_V2
    auto shortcuts = std::make_shared<NiceMock<muse::shortcuts::CommandShortcutsRegisterMock> >();
#else
    auto shortcuts = std::make_shared<NiceMock<ShortcutsRegisterMock> >();
#endif
    muse::shortcuts::Shortcut shortcut;
    ON_CALL(*shortcuts, shortcut(_)).WillByDefault(ReturnRef(shortcut));

    PlaybackToolBarModel model;
    model.commandsRegister.set(commands);
    model.commandsState.set(std::make_shared<CommandsStateStub>());
#ifdef MUSE_MODULE_SHORTCUTS_V2
    model.commandShortcutsRegister.set(shortcuts);
#else
    model.shortcutsRegister.set(shortcuts);
#endif
    PlaybackToolBarTestAccess::configure(model, savedState, configuration);
    PlaybackToolBarTestAccess::updateActions(model);

    std::vector<muse::rcommand::Command> visibleCommands;
    for (const auto* item : model.items()) {
        if (item->command().isValid()) {
            visibleCommands.push_back(item->command());
            EXPECT_TRUE(item->enabled());
        }
    }
    EXPECT_THAT(visibleCommands, UnorderedElementsAre(playback::PLAYBACK_REWIND_START_COMMAND,
                                                     playback::PLAYBACK_TOGGLE_PLAY_PAUSE_COMMAND,
                                                     playback::PLAYBACK_STOP_COMMAND,
                                                     record::RECORD_START_COMMAND));
}
}
}
