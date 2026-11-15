/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "workspacemanager.h"

#include "log.h"

using namespace muse;
using namespace muse::workspace;

static const std::string WORKSPACE_EXT(".mws");

void WorkspaceManager::init()
{
    configuration()->currentWorkspaceNameChanged().onReceive(this, [this](const std::string&) {
        setupCurrentWorkspace();
    });

    load();
}

void WorkspaceManager::deinit()
{
    saveCurrentWorkspace();

    m_currentWorkspace = nullptr;
    m_defaultWorkspace = nullptr;
    m_workspaces.clear();
}

bool WorkspaceManager::isInited() const
{
    return m_defaultWorkspace != nullptr;
}

IWorkspacePtr WorkspaceManager::defaultWorkspace() const
{
    return m_defaultWorkspace;
}

IWorkspacePtr WorkspaceManager::currentWorkspace() const
{
    return m_currentWorkspace;
}

async::Notification WorkspaceManager::currentWorkspaceAboutToBeChanged() const
{
    return m_currentWorkspaceAboutToBeChanged;
}

async::Notification WorkspaceManager::currentWorkspaceChanged() const
{
    return m_currentWorkspaceChanged;
}

IWorkspacePtrList WorkspaceManager::workspaces() const
{
    IWorkspacePtrList iworkspaces;
    iworkspaces.reserve(m_workspaces.size());
    for (const WorkspacePtr& w : m_workspaces) {
        iworkspaces.push_back(w);
    }
    return iworkspaces;
}

WorkspacePtr WorkspaceManager::doNewWorkspace(const std::string& workspaceName) const
{
    return std::make_shared<Workspace>(makeNewWorkspacePath(workspaceName), iocContext());
}

io::path_t WorkspaceManager::makeNewWorkspacePath(const std::string& workspaceName) const
{
    return configuration()->userWorkspacesPath() + "/" + workspaceName + WORKSPACE_EXT;
}

void WorkspaceManager::appendNewWorkspace(WorkspacePtr workspace)
{
    setupConnectionsToNewWorkspace(workspace);
    m_workspaces.push_back(workspace);
}

void WorkspaceManager::setupConnectionsToNewWorkspace(const IWorkspacePtr workspace)
{
    std::string newWorkspaceName = workspace->name();
    workspace->reloadNotification().onNotify(this, [this, newWorkspaceName](){
        if (m_currentWorkspace->name() == newWorkspaceName) {
            m_currentWorkspaceChanged.notify();
        }
    });
}

void WorkspaceManager::load()
{
    reloadWorkspaceFiles();

    setupDefaultWorkspace();
    setupCurrentWorkspace();
}

void WorkspaceManager::reloadWorkspaceFiles()
{
    m_workspaces.clear();

    io::paths_t files = findWorkspaceFiles();
    for (const io::path_t& file : files) {
        auto workspace = std::make_shared<Workspace>(file, iocContext());
        appendNewWorkspace(workspace);
    }

}

io::paths_t WorkspaceManager::findWorkspaceFiles() const
{
    const std::string defaultWorkspaceName = configuration()->defaultWorkspaceName();
    const auto isNotDefaultWorkspace = [&defaultWorkspaceName](const io::path_t& path) {
        return io::completeBasename(path).toStdString() != defaultWorkspaceName;
    };

    auto findFiles = [this](const io::path_t& dirPath) {
        std::string filter = "*" + WORKSPACE_EXT;
        RetVal<io::paths_t> files = fileSystem()->scanFiles(dirPath, { filter });
        if (!files.ret) {
            LOGE() << files.ret.toString();
        }

        return files.val;
    };

    io::paths_t builtinWorkspacesPaths = configuration()->builtinWorkspacesFilePaths();
    muse::remove_if(builtinWorkspacesPaths, isNotDefaultWorkspace);

    io::paths_t userWorkspacesPaths = findFiles(configuration()->userWorkspacesPath());
    muse::remove_if(userWorkspacesPaths, isNotDefaultWorkspace);
    std::set<io::path_t> userWorkspacesFileNames;
    for (const io::path_t& dirPath : userWorkspacesPaths) {
        userWorkspacesFileNames.insert(io::filename(dirPath));
    }

    muse::remove_if(builtinWorkspacesPaths, [=](const io::path_t& path) -> bool {
        io::path_t fileName = io::filename(path);
        return muse::contains(userWorkspacesFileNames, fileName);
    });

    io::paths_t result;
    muse::join(result, builtinWorkspacesPaths);
    muse::join(result, userWorkspacesPaths);

    return result;
}

void WorkspaceManager::setupDefaultWorkspace()
{
    std::string defaultWorkspaceName = configuration()->defaultWorkspaceName();
    WorkspacePtr workspace = findAndInit(defaultWorkspaceName);
    if (workspace) {
        m_defaultWorkspace = workspace;
        return;
    }

    LOGW() << "not found default workspace, will be created new";

    m_defaultWorkspace = doNewWorkspace(defaultWorkspaceName);
    m_workspaces.push_back(m_defaultWorkspace);

    Ret ret = fileSystem()->makePath(configuration()->userWorkspacesPath());
    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    ret = m_defaultWorkspace->load();
    if (!ret) {
        LOGE() << ret.toString();
        return;
    }
}

void WorkspaceManager::setupCurrentWorkspace()
{
    std::string workspaceName = configuration()->currentWorkspaceName();
    if (m_currentWorkspace && m_currentWorkspace->isLoaded()) {
        if (m_currentWorkspace->name() == workspaceName) {
            return;
        }

        saveCurrentWorkspace();

        //! NOTE Perhaps we need to unload the current workspace (clear memory)
    }

    WorkspacePtr workspace = findAndInit(workspaceName);
    if (!workspace) {
        //! NOTE The workspace may have been created by another context/process
        //! and saved to disk. Try reloading the file list before falling back.
        reloadWorkspaceFiles();
        workspace = findAndInit(workspaceName);
    }

    if (!workspace) {
        std::string defaultWorkspaceName = configuration()->defaultWorkspaceName();
        LOGW() << "failed get workspace: " << workspaceName << ", will use " << defaultWorkspaceName;

        //! NOTE Already should be inited
        IF_ASSERT_FAILED(m_defaultWorkspace) {
            setupDefaultWorkspace();
        }

        workspace = m_defaultWorkspace;
        configuration()->setCurrentWorkspaceName(defaultWorkspaceName);
    }

    m_currentWorkspace = workspace;
    m_currentWorkspaceChanged.notify();
}

WorkspacePtr WorkspaceManager::findByName(const std::string& name) const
{
    for (auto workspace : m_workspaces) {
        if (workspace->name() == name) {
            return workspace;
        }
    }

    return nullptr;
}

WorkspacePtr WorkspaceManager::findAndInit(const std::string& name) const
{
    WorkspacePtr workspace = findByName(name);
    if (!workspace) {
        return nullptr;
    }

    if (!workspace->isLoaded()) {
        Ret ret = workspace->load();
        if (!ret) {
            LOGE() << "failed load workspace: " << name;
            return nullptr;
        }
    }

    return workspace;
}

void WorkspaceManager::saveCurrentWorkspace()
{
    if (!m_currentWorkspace) {
        return;
    }

    Ret ret = m_currentWorkspace->save();
    if (!ret) {
        LOGE() << "failed save current workspace, err: " << ret.toString();
    }
}

