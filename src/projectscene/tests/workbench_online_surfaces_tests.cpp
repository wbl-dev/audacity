/*
 * Audacity: A Digital Audio Editor
 */

#include <gtest/gtest.h>

#include "shared/productprofile.h"
#include "view/trackspanel/addeffectmenumodel.h"

namespace au::projectscene {
namespace {

TEST(workbench_online_surfaces_projectscene, hides_realtime_effects_external_help)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    const AddEffectMenuModel model;
    EXPECT_FALSE(model.externalLinksAvailable());
}

} // namespace
} // namespace au::projectscene
