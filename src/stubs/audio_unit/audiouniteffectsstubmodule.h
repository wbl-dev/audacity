/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "modularity/imodulesetup.h"

namespace agu::effects {
class AudioUnitEffectsModule : public muse::modularity::IModuleSetup
{
public:
    AudioUnitEffectsModule() = default;

    std::string moduleName() const override;
};
}
