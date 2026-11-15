/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "au3-builtin-effects/ToneGenBase.h"
#include "../common/generatoreffect.h"
#include "toneeffect.h"

namespace agu::effects {
class ChirpEffect : public ToneEffect
{
public:
    ChirpEffect();

    static const ComponentInterfaceSymbol Symbol;

    ComponentInterfaceSymbol GetSymbol() const override;
};
}
