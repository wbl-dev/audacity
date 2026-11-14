/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Audacity-CLA-applies
 *
 * Audacity
 * A Digital Audio Editor
 *
 * Copyright (C) 2024 Audacity Limited
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

#ifndef AU_PROJECT_OPENSAVEPROJECTSCENARIO_H
#define AU_PROJECT_OPENSAVEPROJECTSCENARIO_H

#include "iopensaveprojectscenario.h"

#include "framework/global/modularity/ioc.h"
#include "framework/global/io/ifilesystem.h"
#include "framework/interactive/iinteractive.h"
#include "framework/multiwindows/imultiwindowsprovider.h"

#include "project/iprojectconfiguration.h"
#include "project/iprojectfilescontroller.h"

namespace au::project {
class OpenSaveProjectScenario : public IOpenSaveProjectScenario, public muse::Contextable
{
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;

    muse::ContextInject<IProjectFilesController> projectFilesController { this };
    muse::ContextInject<muse::IInteractive> interactive { this };

public:
    OpenSaveProjectScenario(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    muse::RetVal<muse::io::path_t> askLocalPath(IAudacityProjectPtr project, SaveMode mode) const override;
    muse::RetVal<muse::io::path_t> resolveLegacyProjectFormat(const muse::io::path_t& path) const override;
};
}

#endif // AU_PROJECT_OPENSAVEPROJECTSCENARIO_H
