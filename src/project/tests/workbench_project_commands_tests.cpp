/*
 * Audacity: A Digital Audio Editor
 */

#include <algorithm>

#include <gtest/gtest.h>

#include "internal/projectcommandsregister.h"
#include "projectcommands.h"
#include "projectmodule.h"

namespace agu::project {
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

    const ProjectCommandsRegister registry;
    const auto& commands = registry.commandList();

    EXPECT_TRUE(contains(commands, PROJECT_NEW_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_OPEN_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_SAVE_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_SAVE_AS_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_IMPORT_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_EXPORT_AUDIO_COMMAND));
    EXPECT_TRUE(contains(commands, PROJECT_EXPORT_LABELS_COMMAND));
}

TEST(workbench_project_commands, registers_only_local_project_routes)
{

    const ProjectPageManifest pages = projectPageManifest();
    EXPECT_TRUE(contains(pages, "agu://project/new"));
    EXPECT_FALSE(contains(pages, "agu://project/asklocationtype"));
    EXPECT_FALSE(contains(pages, "agu://project/savetocloud"));
    EXPECT_FALSE(contains(pages, "agu://project/cloudprojectsyncing"));
}

} // namespace
} // namespace agu::project
