/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "modularity/imodulesetup.h"

namespace agu::effects {
class Lv2EffectsModule : public muse::modularity::IModuleSetup
{
public:
    Lv2EffectsModule() = default;

    std::string moduleName() const override;
    void registerResources() override;
};
}
