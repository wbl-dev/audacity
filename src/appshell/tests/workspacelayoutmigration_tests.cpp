/*
 * AGu: A Digital Audio Editor
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QMap>

#include "internal/workspacelayoutmigration.h"
#include "ui/tests/mocks/uistatemock.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;

namespace {
class WorkspaceLayoutMigrationTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ON_CALL(state, pageState(_)).WillByDefault(Invoke([this](const QString& key) {
            muse::ValNt<QByteArray> result;
            result.val = pages.value(key);
            return result;
        }));
        ON_CALL(state, setPageState(_, _)).WillByDefault(Invoke([this](const QString& key, const QByteArray& value) {
            pages[key] = value;
        }));
        ON_CALL(state, windowGeometry()).WillByDefault(Invoke([this] { return window; }));
        ON_CALL(state, setWindowGeometry(_)).WillByDefault(Invoke([this](const QByteArray& value) { window = value; }));
        ON_CALL(state, uiItemState(_)).WillByDefault(Invoke([this](const QString& key) { return settings.value(key); }));
        ON_CALL(state, setUiItemState(_, _)).WillByDefault(Invoke([this](const QString& key, const QString& value) {
            settings[key] = value;
        }));
    }

    NiceMock<muse::ui::UiStateMock> state;
    QMap<QString, QByteArray> pages;
    QMap<QString, QString> settings;
    QByteArray window;
};
}

TEST_F(WorkspaceLayoutMigrationTests, PreservesOldLayoutBeforeResettingItOnce)
{
    pages["Home"] = "old-home";
    pages["ProjectPage"] = "old-project";
    window = "old-window";

    agu::appshell::migrateLegacyWorkspaceLayout(state);

    EXPECT_TRUE(pages.value("Home").isEmpty());
    EXPECT_TRUE(pages.value("ProjectPage").isEmpty());
    EXPECT_TRUE(window.isEmpty());
    EXPECT_EQ(pages.value("agu/layoutBackup/Home"), "old-home");
    EXPECT_EQ(pages.value("agu/layoutBackup/ProjectPage"), "old-project");
    EXPECT_EQ(pages.value("agu/layoutBackup/window"), "old-window");
    EXPECT_EQ(settings.value("agu/layoutRevision"), "1");
}

TEST_F(WorkspaceLayoutMigrationTests, KeepsLayoutAfterRevisionIsCurrent)
{
    settings["agu/layoutRevision"] = "1";
    pages["ProjectPage"] = "custom-project";
    window = "custom-window";

    agu::appshell::migrateLegacyWorkspaceLayout(state);

    EXPECT_EQ(pages.value("ProjectPage"), "custom-project");
    EXPECT_EQ(window, "custom-window");
    EXPECT_FALSE(pages.contains("agu/layoutBackup/ProjectPage"));
}
