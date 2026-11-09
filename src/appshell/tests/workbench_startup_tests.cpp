/*
 * Audacity: A Digital Audio Editor
 */

#include <algorithm>

#include <gtest/gtest.h>

#include "appshellmodule.h"
#include "internal/startupscenario.h"
#include "qml/Audacity/AppShell/FirstLaunchSetup/firstlaunchsetupmodel.h"
#include "shared/productprofile.h"

namespace au::appshell {
namespace {

class TestableStartupScenario : public StartupScenario
{
public:
    TestableStartupScenario()
        : StartupScenario(muse::modularity::globalCtx()) {}

    using StartupScenario::hasExplicitStartupTarget;
};

bool contains(const std::vector<muse::Uri>& uris, const char* uri)
{
    return std::find(uris.cbegin(), uris.cend(), muse::Uri(uri)) != uris.cend();
}

TEST(workbench_page_manifest, contains_only_local_product_destinations)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    const AppShellPageManifest manifest = appShellPageManifest();

    EXPECT_TRUE(contains(manifest.pages, "audacity://home"));
    EXPECT_TRUE(contains(manifest.pages, "audacity://project"));
    EXPECT_TRUE(contains(manifest.dialogs, "audacity://about/audacity"));
    EXPECT_TRUE(contains(manifest.dialogs, "audacity://firstLaunchSetup"));

    EXPECT_FALSE(contains(manifest.pages, "audacity://publish"));
    EXPECT_FALSE(contains(manifest.dialogs, "audacity://signin/audiocom"));
    EXPECT_FALSE(contains(manifest.dialogs, "audacity://welcomedialog"));
}

TEST(workbench_startup, preserves_local_startup_and_disables_online_flows)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    const StartupPolicy policy = startupPolicy();

    EXPECT_TRUE(policy.showFirstLaunchSetup);
    EXPECT_TRUE(policy.createFirstLaunchProject);
    EXPECT_TRUE(policy.allowSessionRecovery);
    EXPECT_TRUE(policy.allowExplicitLocalProject);
    EXPECT_FALSE(policy.showWelcomeDialog);
    EXPECT_FALSE(policy.checkForUpdates);
    EXPECT_FALSE(policy.dispatchCloudUrls);
}

TEST(workbench_startup, ignores_cloud_deep_links_and_skips_update_dependencies)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    TestableStartupScenario scenario;
    scenario.setStartupUrl("audacity://open?audioId=remote");
    EXPECT_FALSE(scenario.hasExplicitStartupTarget());

    EXPECT_NO_THROW({
        auto splashResult = scenario.runOnSplashScreen();
        (void)splashResult;
    });
}

TEST(workbench_startup, first_launch_setup_contains_only_local_configuration_pages)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    FirstLaunchSetupModel model;
    ASSERT_EQ(model.numberOfPages(), 3);

    const QString expectedPages[] {
        "ThemesPage.qml",
        "ClipVisualizationPage.qml",
        "WorkspaceLayoutPage.qml",
    };

    for (int index = 0; index < model.numberOfPages(); ++index) {
        model.setCurrentPageIndex(index);
        EXPECT_EQ(model.currentPage().value("url").toString(), expectedPages[index]);
    }
}

} // namespace
} // namespace au::appshell
