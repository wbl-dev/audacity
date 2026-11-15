/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

namespace agu::spectrogram {
class Au3SpectrogramSettings;
struct SpectrogramTrackContext {
    Au3SpectrogramSettings& settings;
    const bool trackIsSelected;
    const float minFreq;
    const float maxFreq;
};
}
