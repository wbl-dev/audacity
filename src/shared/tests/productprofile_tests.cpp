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
        EXPECT_FALSE(fullDeveloperToolsAvailable());
    } else {
        EXPECT_TRUE(nonDiagnosticExternalLinksAvailable());
        EXPECT_TRUE(fullDeveloperToolsAvailable());
    }
}

} // namespace
} // namespace au::shared
