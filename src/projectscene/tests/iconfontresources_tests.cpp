/*
 * AGu: A Digital Audio Editor
 */
#include <gtest/gtest.h>

#include <QFontDatabase>
#include <QResource>

TEST(IconFontResourcesTests, PackagedMuseIconFontLoadsWithExpectedFamily)
{
    Q_INIT_RESOURCE(muse_ui_resources);

    const int fontId = QFontDatabase::addApplicationFont(":/ui/data/MusescoreIcon.ttf");
    ASSERT_NE(fontId, -1);
    EXPECT_TRUE(QFontDatabase::applicationFontFamilies(fontId).contains("MusescoreIcon"));
}
