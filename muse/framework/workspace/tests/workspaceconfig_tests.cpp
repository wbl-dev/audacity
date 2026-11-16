/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <set>

#include "io/fileinfo.h"
#include "workspace/internal/workspacefile.h"
#include "workspace/tests/workspace_test_helpers.h"

using namespace muse;
using namespace muse::workspace;

struct ConfigTestParam {
    std::string configFile;
    std::string workspacesDir;
};

static void PrintTo(const ConfigTestParam& param, std::ostream* os)
{
    *os << param.configFile;
}

static std::vector<ConfigTestParam> configTestParams()
{
    std::vector<ConfigTestParam> params = {
        { WORKSPACE_CONFIG_FILE, BUILTIN_WORKSPACES_DIR },
    };
#if defined(APP_WORKSPACE_CONFIG_FILE) && defined(APP_BUILTIN_WORKSPACES_DIR)
    params.push_back({ APP_WORKSPACE_CONFIG_FILE, APP_BUILTIN_WORKSPACES_DIR });
#endif
    return params;
}

class Workspace_ConfigTests : public ::testing::TestWithParam<ConfigTestParam>
{
};

TEST_P(Workspace_ConfigTests, ConfigIsValid)
{
    const auto& param = GetParam();
    WorkspaceTestConfig cfg;
    ASSERT_TRUE(cfg.load(io::path_t(param.configFile), param.workspacesDir))
        << "failed to load: " << param.configFile;

    //! [THEN] default_workspace_name is a non-empty string
    EXPECT_FALSE(cfg.defaultWorkspaceName().empty());

    //! [THEN] builtin_workspace_files is a non-empty list
    EXPECT_FALSE(cfg.builtinFiles().empty());

    //! [THEN] Each builtin file exists on disk
    bool defaultFoundInBuiltins = false;
    for (const auto& filename : cfg.builtinFiles()) {
        io::path_t fullPath = io::path_t(cfg.builtinWorkspacesDir() + "/" + filename);
        EXPECT_TRUE(io::FileInfo::exists(fullPath)) << "missing: " << fullPath.toStdString();

        std::string baseName = io::completeBasename(fullPath).toStdString();
        if (baseName == cfg.defaultWorkspaceName()) {
            defaultFoundInBuiltins = true;
        }
    }

    EXPECT_TRUE(defaultFoundInBuiltins)
        << "default workspace \"" << cfg.defaultWorkspaceName() << "\" not found in builtin files";
}

TEST_P(Workspace_ConfigTests, AppShipsOnlyModern)
{
#if defined(APP_WORKSPACE_CONFIG_FILE)
    const auto& param = GetParam();
    if (param.configFile != APP_WORKSPACE_CONFIG_FILE) {
        return;
    }

    WorkspaceTestConfig cfg;
    ASSERT_TRUE(cfg.load(io::path_t(param.configFile), param.workspacesDir));
    EXPECT_EQ(cfg.defaultWorkspaceName(), "Modern");
    EXPECT_EQ(cfg.builtinFiles(), std::vector<std::string>{ "Modern.mws" });
#endif
}

TEST_P(Workspace_ConfigTests, AppProjectLayoutMatchesCurrentDockNames)
{
#if defined(APP_WORKSPACE_CONFIG_FILE)
    const auto& param = GetParam();
    if (param.configFile != APP_WORKSPACE_CONFIG_FILE) {
        return;
    }

    WorkspaceFile file(io::path_t(param.workspacesDir + "/Modern.mws"));
    ASSERT_TRUE(file.load());

    const QJsonObject states = QJsonDocument::fromJson(file.data("ui_states")).object();
    const QByteArray projectLayout = states.value("ProjectPage").toString().toUtf8();
    ASSERT_FALSE(projectLayout.isEmpty());

    const QJsonObject project = QJsonDocument::fromJson(projectLayout).object();
    const QJsonArray windows = project.value("mainWindows").toArray();
    ASSERT_EQ(windows.size(), 1);
    EXPECT_EQ(windows.first().toObject().value("uniqueName").toString(), "mainWindow");

    const QJsonArray docks = project.value("allDockWidgets").toArray();
    bool hasCentralDock = false;
    for (const QJsonValue& dock : docks) {
        const QString name = dock.toObject().value("uniqueName").toString();
        EXPECT_FALSE(name.endsWith("_0")) << name.toStdString();
        hasCentralDock |= name == "ProjectPage_central";
    }
    EXPECT_TRUE(hasCentralDock);
#endif
}

//! If this test fails, a new field was added to the config.
//! Please add validation for it in ConfigIsValid above, then update EXPECTED_KEYS.
TEST_P(Workspace_ConfigTests, NoUntestedConfigKeys)
{
    const auto& param = GetParam();
    WorkspaceTestConfig cfg;
    ASSERT_TRUE(cfg.load(io::path_t(param.configFile), param.workspacesDir))
        << "failed to load: " << param.configFile;

    const std::set<std::string> EXPECTED_KEYS = {
        "default_workspace_name",
        "builtin_workspace_files",
    };

    std::set<std::string> actualKeys;
    for (const auto& [key, val] : cfg.config().data()) {
        actualKeys.insert(key);
    }

    EXPECT_EQ(actualKeys, EXPECTED_KEYS)
        << "config has changed — add validation in ConfigIsValid and update EXPECTED_KEYS";
}

INSTANTIATE_TEST_SUITE_P(
    Configs,
    Workspace_ConfigTests,
    ::testing::ValuesIn(configTestParams())
    );
