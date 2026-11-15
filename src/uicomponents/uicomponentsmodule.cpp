/*
* Audacity: A Digital Audio Editor
*/

#include "iapplication.h"

#include "uicomponentsmodule.h"

#include "log.h"

using namespace agu::uicomponents;

std::string UiComponentsModule::moduleName() const
{
    return "agu::uicomponents";
}

void UiComponentsModule::onInit(const muse::IApplication::RunMode& mode)
{
    UNUSED(mode);
}

void UiComponentsModule::onDeinit()
{
}
