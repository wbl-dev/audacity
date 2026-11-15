/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

namespace agu::shared {
enum class AxisScale {
    Undefined = -1,
    Linear,
    Logarithmic,
    Mel,
    Bark,
    ERB,
    Period,
    _count
};
}
