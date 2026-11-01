/*
 * Audacity: A Digital Audio Editor
 */

#include <gtest/gtest.h>

#include "appshellcommands.h"
#include "qml/Audacity/AppShell/appmenumodel.h"
#include "shared/productprofile.h"

#include "effects/effects_base/effectscommands.h"
#include "project/projectcommands.h"
#include "framework/diagnostics/diagnosticscommands.h"
#include "framework/update/updatecommands.h"

namespace au::appshell {
namespace {

TEST(workbench_menu, excludes_online_and_marketplace_commands)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    EXPECT_FALSE(isMenuCommandAllowed(project::PROJECT_SAVE_TO_CLOUD_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(project::PROJECT_UPDATE_CLOUD_AUDIO_PREVIEW_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(project::PROJECT_SHARE_AUDIO_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(APP_ONLINE_HANDBOOK_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(muse::update::UPDATE_CHECK_COMMAND));
    EXPECT_FALSE(isMenuCommandAllowed(effects::EFFECTS_GET_EFFECTS_COMMAND));
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

} // namespace
} // namespace au::appshell
