/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "modularity/ioc.h"

namespace agu::testutils {
static constexpr muse::modularity::IoCID testContextId = 999;

inline muse::modularity::ContextPtr makeTestContext()
{
    return std::make_shared<muse::modularity::Context>(testContextId);
}
}
