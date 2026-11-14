/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Audacity-CLA-applies
 *
 * Audacity
 * A Digital Audio Editor
 *
 * Copyright (C) 2024 Audacity BVBA and others
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

#include "projectspagemodel.h"

#include <QString>

#include "types/projecttypes.h"
#include "../projectcommands.h"

using namespace au::project;
using namespace muse;

ProjectsPageModel::ProjectsPageModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void ProjectsPageModel::createNewProject()
{
    commandDispatcher()->dispatch(PROJECT_NEW_COMMAND);
}

void ProjectsPageModel::openOther()
{
    commandDispatcher()->dispatch(PROJECT_OPEN_COMMAND);
}

void ProjectsPageModel::openProject(const QString& scorePath, const QString& displayNameOverride)
{
    commandDispatcher()->dispatch(PROJECT_OPEN_COMMAND, {
        { PROJECT_URL_PARAM, Val(QUrl::fromLocalFile(scorePath).toString()) },
        { PROJECT_DISPLAY_NAME_PARAM, Val(displayNameOverride) },
    });
}

int ProjectsPageModel::tabIndex() const
{
    return configuration()->homeProjectsPageTabIndex();
}

void ProjectsPageModel::setTabIndex(int index)
{
    if (index == tabIndex()) {
        return;
    }

    configuration()->setHomeProjectsPageTabIndex(index);
    emit tabIndexChanged();
}

ProjectsPageModel::ViewType ProjectsPageModel::viewType() const
{
    return static_cast<ViewType>(configuration()->homeProjectsPageViewType());
}

void ProjectsPageModel::setViewType(ViewType type)
{
    if (viewType() == type) {
        return;
    }

    configuration()->setHomeProjectsPageViewType(IProjectConfiguration::HomeProjectsPageViewType(type));
    emit viewTypeChanged();
}
