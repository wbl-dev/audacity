/*
 * Audacity: A Digital Audio Editor
 */
#include <gtest/gtest.h>

#include "global/settings.h"
#include "importexport/import/internal/importerconfiguration.h"

using namespace au::importexport;

namespace {
const muse::Settings::Key PREF("importer", "importer/tempoDetectionPref");
const muse::Settings::Key WORKSPACES("importer", "importer/tempoDetectionWorkspaces");

class ImporterConfigurationMigrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        muse::settings()->setDefaultValue(PREF, muse::Val(static_cast<int>(TempoDetectionPref::TempoDetection::NEVER)));
        muse::settings()->setDefaultValue(WORKSPACES, muse::Val("Music"));
    }

    void TearDown() override
    {
        muse::settings()->setLocalValue(PREF, muse::Val(static_cast<int>(TempoDetectionPref::TempoDetection::NEVER)));
        muse::settings()->setLocalValue(WORKSPACES, muse::Val("Music"));
    }
};

TEST_F(ImporterConfigurationMigrationTest, ModernInLegacyListEnablesTempoDetection)
{
    muse::settings()->setLocalValue(PREF, muse::Val(1));
    muse::settings()->setLocalValue(WORKSPACES, muse::Val("Classic|Modern"));

    ImporterConfiguration configuration;
    EXPECT_EQ(configuration.tempoDetectionPref(), TempoDetectionPref::TempoDetection::ALWAYS);
    EXPECT_EQ(muse::settings()->value(PREF).toInt(), 0);
}

TEST_F(ImporterConfigurationMigrationTest, LegacyMusicOnlyDisablesTempoDetection)
{
    muse::settings()->setLocalValue(PREF, muse::Val(1));
    muse::settings()->setLocalValue(WORKSPACES, muse::Val("Music"));

    ImporterConfiguration configuration;
    EXPECT_EQ(configuration.tempoDetectionPref(), TempoDetectionPref::TempoDetection::NEVER);
    EXPECT_EQ(muse::settings()->value(PREF).toInt(), 2);
}
}
