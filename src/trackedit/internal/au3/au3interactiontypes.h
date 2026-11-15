/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

namespace agu::trackedit {
enum class NeedsDownmixing {
    Yes,
    No,
};

constexpr NeedsDownmixing operator|=(NeedsDownmixing& lhs, NeedsDownmixing rhs)
{
    return lhs = lhs == NeedsDownmixing::Yes || rhs == NeedsDownmixing::Yes ? NeedsDownmixing::Yes : NeedsDownmixing::No;
}
}
