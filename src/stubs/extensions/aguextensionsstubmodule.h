/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "modularity/imodulesetup.h"

namespace agu::extensions {
class AGuExtensionsModule final : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
};
} // namespace agu::extensions
