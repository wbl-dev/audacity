/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "effects/builtin/qml/AGu/BuiltinEffects/builtineffectmodel.h"

namespace agu::effects {
class LimiterViewModel : public BuiltinEffectModel
{
    Q_OBJECT

public:
    LimiterViewModel(QObject* parent, int instanceId);

private:
    void doReload() override;
};

class LimiterViewModelFactory : public EffectViewModelFactory<LimiterViewModel>
{
};
}
