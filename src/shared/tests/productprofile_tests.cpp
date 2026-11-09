/*
 * Audacity: A Digital Audio Editor
 */

#include <gtest/gtest.h>

#include "shared/productprofile.h"

namespace au::shared {
namespace {

TEST(ProductProfile, ExposesNonDiagnosticExternalLinksOnlyInDefaultAudacity)
{
    if constexpr (isWorkbenchProfile()) {
        EXPECT_FALSE(nonDiagnosticExternalLinksAvailable());
    } else {
        EXPECT_TRUE(nonDiagnosticExternalLinksAvailable());
    }
}

} // namespace
} // namespace au::shared
