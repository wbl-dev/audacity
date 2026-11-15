/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "../iprojectviewstatecreator.h"

namespace agu::projectscene {
class ProjectViewStateCreator : public IProjectViewStateCreator
{
public:
    std::shared_ptr<IProjectViewState> createViewState(std::shared_ptr<agu::au3::IAu3Project> project) const override;
};
}
