/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "modularity/imodulesetup.h"

namespace agu::effects::extensions {
class ExtensionEffectsModule final : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
};
} // namespace agu::effects::extensions
