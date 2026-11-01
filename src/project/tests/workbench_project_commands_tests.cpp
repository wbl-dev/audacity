/*
 * Audacity: A Digital Audio Editor
 */

#include <algorithm>

#include <gtest/gtest.h>

#include "internal/projectcommandsregister.h"
#include "projectcommands.h"
#include "projectmodule.h"
#include "shared/productprofile.h"

namespace au::project {
namespace {

bool contains(const std::vector<muse::rcommand::Command>& commands, const muse::rcommand::Command& command)
{
    return std::find(commands.cbegin(), commands.cend(), command) != commands.cend();
}

bool contains(const ProjectPageManifest& pages, const char* uri)
{
    return std::find_if(pages.cbegin(), pages.cend(), [uri](const ProjectPageRegistration& page) {
        return page.uri == muse::Uri(uri);
    }) != pages.cend();
}

TEST(workbench_project_commands, excludes_cloud_commands_and_keeps_local_project_workflow)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    const ProjectCommandsRegister registry;
    const auto& commands = registry.commandList();

    EXPECT_FALSE(contains(commands, PROJECT_OPEN_CLOUD_COMMAND));
    EXPECT_FALSE(contains(commands, PROJECT_SAVE_TO_CLOUD_COMMAND));
    EXPECT_FALSE(contains(commands, PROJECT_SHARE_AUDIO_COMMAND));
    EXPECT_FALSE(contains(commands, PROJECT_OPEN_CLOUD_AUDIO_FILE_COMMAND));
    EXPECT_FALSE(contains(commands, PROJECT_UPDATE_CLOUD_AUDIO_PREVIEW_COMMAND));
    EXPECT_FALSE(contains(commands, PROJECT_UPDATE_CLOUD_AUDIO_PREVIEW_FOR_PROJECT_COMMAND));

    EXPECT_FALSE(isProjectActionAllowed("cloud-file-open"));
    EXPECT_FALSE(isProjectActionAllowed("file-save-to-cloud"));
    EXPECT_FALSE(isProjectActionAllowed("file-share-audio"));
    EXPECT_FALSE(isProjectActionAllowed("action://cloud/open-audio-file"));
    EXPECT_FALSE(isProjectActionAllowed("action://cloud/update-audio-preview"));
    EXPECT_FALSE(isProjectActionAllowed("action://cloud/update-audio-preview-for-project"));
    EXPECT_FALSE(isProjectActionAllowed("link-account"));

    EXPECT_TRUE(contains(commands, PROJECT_NEW_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_OPEN_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_SAVE_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_SAVE_AS_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_IMPORT_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_EXPORT_AUDIO_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_EXPORT_LABELS_COMMAND));
    EXPECT_TRUE(isProjectActionAllowed("file-open"));
    EXPECT_TRUE(isProjectActionAllowed("file-save"));
    EXPECT_TRUE(isProjectActionAllowed("export-audio"));
}

TEST(workbench_project_commands, registers_only_local_project_routes)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    const ProjectPageManifest pages = projectPageManifest();
    EXPECT_TRUE(contains(pages, "audacity://project/new"));
    EXPECT_FALSE(contains(pages, "audacity://project/asklocationtype"));
    EXPECT_FALSE(contains(pages, "audacity://project/savetocloud"));
    EXPECT_FALSE(contains(pages, "audacity://project/cloudprojectsyncing"));
}

} // namespace
} // namespace au::project
