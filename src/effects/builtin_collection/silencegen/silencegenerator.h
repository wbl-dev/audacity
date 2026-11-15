#pragma once

#include "../common/generatoreffect.h"
#include "au3-builtin-effects/SilenceBase.h"

namespace agu::effects {
class SilenceGenerator : public SilenceBase, public GeneratorEffect
{
public:
    SilenceGenerator();
};
}
