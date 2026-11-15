/*
* Audacity: A Digital Audio Editor
*/
#include "projectcreator.h"

#include "aguproject.h"

using namespace agu::project;

IAGuProjectPtr ProjectCreator::newProject() const
{
    return std::make_shared<AGuProject>(iocContext());
}

muse::RetVal<IAGuProjectPtr> ProjectCreator::loadProject(const muse::io::path_t& path) const
{
    IAGuProjectPtr project = std::make_shared<AGuProject>(iocContext());

    const muse::Ret ret = project->load(path);
    if (!ret) {
        return muse::RetVal<IAGuProjectPtr>::make_ret(ret);
    }

    return muse::RetVal<IAGuProjectPtr>::make_ok(project);
}
