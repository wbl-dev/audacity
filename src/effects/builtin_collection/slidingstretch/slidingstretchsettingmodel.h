/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "../common/effectsettingmodelimpl.h"
#include "slidingstretcheffect.h"

namespace agu::effects {
class SlidingStretchSettingModel : public EffectSettingModelImpl<SlidingStretchEffect>
{
public:
    SlidingStretchSettingModel(QObject* parent, int instanceId);
};

class SlidingStretchSettingModelFactory : public BuiltinEffectSettingModelFactory<SlidingStretchSettingModel>
{
};
}
