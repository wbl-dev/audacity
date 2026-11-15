/*
 * Audacity: A Digital Audio Editor
 */

#include <algorithm>

#include <gtest/gtest.h>

#include "appshellmodule.h"
#include "internal/startupscenario.h"
#include "qml/AGu/AppShell/FirstLaunchSetup/firstlaunchsetupmodel.h"
#include "qml/AGu/AppShell/maintoolbarmodel.h"

namespace agu::appshell {
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

    const AppShellPageManifest manifest = appShellPageManifest();

    EXPECT_TRUE(contains(manifest.pages, "agu://home"));
    EXPECT_TRUE(contains(manifest.pages, "agu://project"));
    EXPECT_TRUE(contains(manifest.dialogs, "agu://about/agu"));
    EXPECT_TRUE(contains(manifest.dialogs, "agu://firstLaunchSetup"));

    EXPECT_FALSE(contains(manifest.pages, "agu://publish"));
    EXPECT_FALSE(contains(manifest.pages, "agu://devtools"));
    EXPECT_FALSE(contains(manifest.dialogs, "agu://signin/audiocom"));
    EXPECT_FALSE(contains(manifest.dialogs, "agu://welcomedialog"));
}

TEST(workbench_startup, preserves_local_startup_and_disables_online_flows)
{

    const StartupPolicy policy = startupPolicy();

    EXPECT_TRUE(policy.showFirstLaunchSetup);
    EXPECT_TRUE(policy.createFirstLaunchProject);
    EXPECT_TRUE(policy.allowSessionRecovery);
    EXPECT_TRUE(policy.allowExplicitLocalProject);
}

TEST(workbench_startup, first_launch_setup_contains_only_local_configuration_pages)
{

    FirstLaunchSetupModel model;
    ASSERT_EQ(model.numberOfPages(), 2);

    const QString expectedPages[] {
        "ThemesPage.qml",
        "ClipVisualizationPage.qml",
    };

    for (int index = 0; index < model.numberOfPages(); ++index) {
        model.setCurrentPageIndex(index);
        EXPECT_EQ(model.currentPage().value("url").toString(), expectedPages[index]);
    }
}

} // namespace
} // namespace agu::appshell
