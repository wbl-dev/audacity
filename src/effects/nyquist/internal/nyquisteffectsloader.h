/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "effects/effects_base/internal/au3effectloader.h"
#include "au3-nyquist-effects/LoadNyquist.h"

namespace agu::effects {
class NyquistEffectsLoader final : public Au3EffectLoader
{
public:
    NyquistEffectsLoader()
        : Au3EffectLoader(m_module, EffectFamily::Nyquist) {}

private:
    ::NyquistEffectsModule m_module;
};
} // namespace agu::effects
