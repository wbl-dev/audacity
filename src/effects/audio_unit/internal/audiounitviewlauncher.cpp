/*
* Audacity: A Digital Audio Editor
*/
#include "audiounitviewlauncher.h"

muse::Ret agu::effects::AudioUnitViewLauncher::showEffect(const EffectInstanceId& instanceId) const
{
    return doShowEffect(instanceId, EffectFamily::AudioUnit);
}

void agu::effects::AudioUnitViewLauncher::showRealtimeEffect(const RealtimeEffectStatePtr& state) const
{
    doShowRealtimeEffect(state);
}
