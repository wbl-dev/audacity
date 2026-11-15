/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "spectrogramtypes.h"

namespace agu::spectrogram {
class IFrequencySelectionRestorer
{
public:
    virtual ~IFrequencySelectionRestorer() = default;
    virtual void storeFrequencySelectionState(const FrequencySelection& selection) const = 0;
    virtual FrequencySelection loadFrequencySelectionState() const = 0;
};
}
