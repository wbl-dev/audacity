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

#ifndef AU_PROJECT_IOPENSAVEPROJECTSCENARIO_H
#define AU_PROJECT_IOPENSAVEPROJECTSCENARIO_H

#include "modularity/imoduleinterface.h"
#include "global/types/retval.h"
#include "project/iaudacityproject.h"

namespace au::project {
class IOpenSaveProjectScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IOpenSaveProjectScenario)

public:
    virtual muse::RetVal<muse::io::path_t> askLocalPath(IAudacityProjectPtr project, SaveMode mode) const = 0;
    virtual muse::RetVal<muse::io::path_t> resolveLegacyProjectFormat(const muse::io::path_t& path) const = 0;
};
}

#endif // AU_PROJECT_IOPENSAVEPROJECTSCENARIO_H
