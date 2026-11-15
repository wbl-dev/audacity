/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "ispectrogramconfiguration.h"

#include <memory>

namespace agu::spectrogram {
class ITrackSpectrogramConfiguration : public ISpectrogramConfiguration
{
public:
    virtual ~ITrackSpectrogramConfiguration() = default;

    virtual bool useGlobalSettings() = 0;
    virtual void setUseGlobalSettings(bool value) = 0;
};

using ITrackSpectrogramConfigurationPtr = std::shared_ptr<ITrackSpectrogramConfiguration>;
} // namespace agu::spectrogram
