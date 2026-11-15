/*
 * Audacity: A Digital Audio Editor
 */
#include <gtest/gtest.h>

#include "global/settings.h"
#include "projectscene/internal/projectsceneconfiguration.h"

using namespace au::projectscene;

namespace {
const muse::Settings::Key PREF("projectscene", "projectscene/asymmetricStereoHeights");
const muse::Settings::Key WORKSPACES("projectscene", "projectscene/asymmetricStereoHeightsWorkspaces");

class StereoHeightConfigurationMigrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        muse::settings()->setDefaultValue(PREF, muse::Val(static_cast<int>(StereoHeightsPref::AsymmetricStereoHeights::NEVER)));
        muse::settings()->setDefaultValue(WORKSPACES, muse::Val("Modern"));
    }

    void TearDown() override
    {
        muse::settings()->setLocalValue(PREF, muse::Val(static_cast<int>(StereoHeightsPref::AsymmetricStereoHeights::NEVER)));
        muse::settings()->setLocalValue(WORKSPACES, muse::Val("Modern"));
    }
};

TEST_F(StereoHeightConfigurationMigrationTest, ModernInLegacyListEnablesAsymmetricHeights)
{
    muse::settings()->setLocalValue(PREF, muse::Val(1));
    muse::settings()->setLocalValue(WORKSPACES, muse::Val("Music|Modern"));

    ProjectSceneConfiguration configuration;
    EXPECT_EQ(configuration.stereoHeightsPref(), StereoHeightsPref::AsymmetricStereoHeights::ALWAYS);
    EXPECT_EQ(muse::settings()->value(PREF).toInt(), 0);
}

TEST_F(StereoHeightConfigurationMigrationTest, LegacyMusicOnlyDisablesAsymmetricHeights)
{
    muse::settings()->setLocalValue(PREF, muse::Val(1));
    muse::settings()->setLocalValue(WORKSPACES, muse::Val("Music"));

    ProjectSceneConfiguration configuration;
    EXPECT_EQ(configuration.stereoHeightsPref(), StereoHeightsPref::AsymmetricStereoHeights::NEVER);
    EXPECT_EQ(muse::settings()->value(PREF).toInt(), 2);
}
}
