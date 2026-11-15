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
#ifndef MUSE_WORKSPACE_WORKSPACEMANAGER_H
#define MUSE_WORKSPACE_WORKSPACEMANAGER_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "io/ifilesystem.h"
#include "../iworkspaceconfiguration.h"

#include "workspace.h"

#include "iworkspacemanager.h"

namespace muse::workspace {
class WorkspaceManager : public IWorkspaceManager, public Contextable, public async::Asyncable
{
    GlobalInject<io::IFileSystem> fileSystem;
    GlobalInject<IWorkspaceConfiguration> configuration;

public:

    WorkspaceManager(const modularity::ContextPtr& iocCtx)
        : Contextable(iocCtx) {}

    void init();
    void deinit();
    bool isInited() const;

    IWorkspacePtr defaultWorkspace() const override;

    IWorkspacePtr currentWorkspace() const override;
    async::Notification currentWorkspaceAboutToBeChanged() const override;
    async::Notification currentWorkspaceChanged() const override;

    IWorkspacePtrList workspaces() const override;

private:
    void load();
    void reloadWorkspaceFiles();

    io::paths_t findWorkspaceFiles() const;

    WorkspacePtr doNewWorkspace(const std::string& workspaceName) const;
    io::path_t makeNewWorkspacePath(const std::string& workspaceName) const;

    void appendNewWorkspace(WorkspacePtr workspace);
    void setupConnectionsToNewWorkspace(const IWorkspacePtr workspace);

    void setupDefaultWorkspace();
    void setupCurrentWorkspace();
    void saveCurrentWorkspace();

    WorkspacePtr findByName(const std::string& name) const;
    WorkspacePtr findAndInit(const std::string& name) const;

    WorkspacePtr m_defaultWorkspace;
    WorkspacePtr m_currentWorkspace;
    async::Notification m_currentWorkspaceAboutToBeChanged;
    async::Notification m_currentWorkspaceChanged;

    std::vector<WorkspacePtr> m_workspaces;
};
}

#endif // MUSE_WORKSPACE_WORKSPACEMANAGER_H
