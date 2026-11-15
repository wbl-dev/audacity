/*
 * Audacity: A Digital Audio Editor
 */
#include "extensioneffectsstubmodule.h"

namespace agu::effects::extensions {
std::string ExtensionEffectsModule::moduleName() const
{
    return "effects_extensions";
}
} // namespace agu::effects::extensions
