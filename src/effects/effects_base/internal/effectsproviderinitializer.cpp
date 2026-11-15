/*
 * Audacity: A Digital Audio Editor
 */
#include "effectsproviderinitializer.h"

namespace agu::effects {
void EffectsProviderInitializer::setStartupPluginValidationPolicy(StartupPluginValidationPolicy policy)
{
    m_validationPolicy = policy;
}

void EffectsProviderInitializer::callAfterSplashScreen()
{
    static bool effectsProviderInitialized = false;
    if (effectsProviderInitialized) {
        return;
    }
    effectsProvider()->initOnce(iocContext(), *interactive(), *registerAudioPluginsScenario(), m_validationPolicy);
    effectsProviderInitialized = true;
}
} // namespace agu::effects
