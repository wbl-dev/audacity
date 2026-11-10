/*
 * Audacity: A Digital Audio Editor
 */

#include <algorithm>

#include <QFile>

#include <gtest/gtest.h>

#include "appshellcommands.h"
#include "internal/appshellcommandsregister.h"
#include "internal/applicationactioncontroller.h"
#include "internal/applicationuiactions.h"
#include "qml/Audacity/AppShell/appmenumodel.h"
#include "qml/Audacity/AppShell/aboutmodel.h"
#include "shared/productprofile.h"

#include "effects/effects_base/effectscommands.h"
#include "project/projectcommands.h"
#include "framework/diagnostics/diagnosticscommands.h"
#include "framework/update/updatecommands.h"

namespace au::appshell {
namespace {

bool contains(const std::vector<muse::rcommand::Command>& commands, const muse::rcommand::Command& command)
{
    return std::find(commands.cbegin(), commands.cend(), command) != commands.cend();
}

bool contains(const muse::ui::UiActionList& actions, const muse::actions::ActionCode& actionCode)
{
    return std::find_if(actions.cbegin(), actions.cend(), [&actionCode](const muse::ui::UiAction& action) {
        return action.code == actionCode;
    }) != actions.cend();
}

TEST(workbench_menu, excludes_online_and_marketplace_commands)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    EXPECT_FALSE(isMenuCommandAllowed(project::PROJECT_SAVE_TO_CLOUD_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(project::PROJECT_UPDATE_CLOUD_AUDIO_PREVIEW_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(project::PROJECT_SHARE_AUDIO_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(APP_ONLINE_HANDBOOK_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(muse::update::UPDATE_CHECK_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(effects::EFFECTS_GET_EFFECTS_COMMAND));

    const AppShellCommandsRegister commandRegistry;
    EXPECT_FALSE(contains(commandRegistry.commandList(), APP_ONLINE_HANDBOOK_COMMAND));
    EXPECT_FALSE(contains(commandRegistry.commandList(), APP_ASK_HELP_COMMAND));

    const auto controller = std::make_shared<ApplicationActionController>(muse::modularity::globalCtx());
    const ApplicationUiActions uiActions(muse::modularity::globalCtx(), controller);
    EXPECT_FALSE(contains(uiActions.actionsList(), "online-handbook"));
    EXPECT_FALSE(contains(uiActions.actionsList(), "ask-help"));
}

TEST(workbench_menu, keeps_local_workbench_commands_and_diagnostics)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    EXPECT_TRUE(isMenuCommandAllowed(project::PROJECT_NEW_COMMAND));
    EXPECT_TRUE(isMenuCommandAllowed(project::PROJECT_OPEN_COMMAND));
    EXPECT_TRUE(isMenuCommandAllowed(project::PROJECT_SAVE_COMMAND));
    EXPECT_TRUE(isMenuCommandAllowed(project::PROJECT_EXPORT_AUDIO_COMMAND));
    EXPECT_TRUE(isMenuCommandAllowed(APP_ABOUT_COMMAND));
    EXPECT_TRUE(isMenuCommandAllowed(APP_PREFERENCES_COMMAND));
    EXPECT_TRUE(isMenuCommandAllowed(effects::EFFECTS_PLUGIN_MANAGER_COMMAND));
    EXPECT_TRUE(isMenuCommandAllowed(muse::diagnostics::DIAGNOSTICS_SAVE_FILES_COMMAND));
}

TEST(workbench_online_surfaces_appshell, removes_about_dialog_external_links)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    const AboutModel model;
    EXPECT_FALSE(model.externalLinksAvailable());

    const QVariantList sections = model.creditList();
    for (const QVariant& sectionValue : sections) {
        const QVariantList credits = sectionValue.toMap().value("credits").toList();
        for (const QVariant& creditValue : credits) {
            EXPECT_TRUE(creditValue.toMap().value("url").toString().isEmpty());
        }
    }

    EXPECT_FALSE(model.gplText().contains("<a href", Qt::CaseInsensitive));
}

TEST(workbench_online_surfaces_appshell, guards_devtools_external_url_test)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    QFile qmlFile(QString::fromUtf8(appshell_tests_DATA_ROOT)
                  + "/../qml/Audacity/AppShell/DevTools/Interactive/InteractiveTests.qml");
    ASSERT_TRUE(qmlFile.open(QIODevice::ReadOnly));

    const QByteArray source = qmlFile.readAll();
    EXPECT_TRUE(source.contains("AboutModel {"));
    EXPECT_TRUE(source.contains("visible: productPolicyModel.externalLinksAvailable"));
}

} // namespace
} // namespace au::appshell
