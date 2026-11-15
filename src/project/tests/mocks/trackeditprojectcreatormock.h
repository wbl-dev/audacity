/*
* Audacity: A Digital Audio Editor
*/

#pragma once

#include <gmock/gmock.h>

#include "trackedit/itrackeditproject.h"

namespace agu::project {
class TrackeditProjectCreatorMock final : public trackedit::ITrackeditProjectCreator
{
public:
    MOCK_METHOD(trackedit::ITrackeditProjectPtr, create,
                (const std::shared_ptr<agu::au3::IAu3Project>& au3project),
                (const, override));
};
} // namespace agu::project
