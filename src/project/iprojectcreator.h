/*
* Audacity: A Digital Audio Editor
*/

#pragma once
#include "iaguproject.h"

#include "modularity/imoduleinterface.h"
#include "iaguproject.h"

namespace agu::project {
class IProjectCreator : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IProjectCreator)

public:
    virtual ~IProjectCreator() = default;

    virtual IAGuProjectPtr newProject() const = 0;
    virtual muse::RetVal<IAGuProjectPtr> loadProject(const muse::io::path_t& path) const = 0;
};
}
