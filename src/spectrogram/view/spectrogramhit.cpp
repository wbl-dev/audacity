/*
 * Audacity: A Digital Audio Editor
 */
#include "spectrogramhit.h"

namespace agu::spectrogram {
SpectrogramHit SpectrogramHitFactory::createSpectrogramHit(int trackId, int channel, double spectrogramY, double spectrogramHeight)
{
    return SpectrogramHit{ trackId, channel, spectrogramY, spectrogramHeight };
}
} // namespace agu::spectrogram
