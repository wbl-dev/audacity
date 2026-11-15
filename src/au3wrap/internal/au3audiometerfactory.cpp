/*
 * Audacity: A Digital Audio Editor
 */
#include "au3audiometerfactory.h"
#include "audio/iaudiometer.h"

std::shared_ptr<agu::au3::Au3AudioMeter> agu::au3::createAudioMeter()
{
    return std::make_shared<Au3AudioMeter>(audio::IAudioMeter::create());
}
