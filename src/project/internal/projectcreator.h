/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "framework/global/modularity/ioc.h"

#include "../iprojectcreator.h"

namespace agu::project {
class ProjectCreator : public IProjectCreator, public muse::Contextable
{
public:
    ProjectCreator(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    IAGuProjectPtr newProject() const override;
    muse::RetVal<IAGuProjectPtr> loadProject(const muse::io::path_t& path) const override;
};
}
