/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "au3audiometer.h"
#include <memory>

namespace agu::au3 {
std::shared_ptr<Au3AudioMeter> createAudioMeter();
}
