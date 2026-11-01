/*
 * Audacity: A Digital Audio Editor
 */

#include <gtest/gtest.h>

#include "effectscommands.h"
#include "shared/productprofile.h"

namespace au::effects {
namespace {

TEST(workbench_effect_commands, excludes_marketplace_and_keeps_local_effect_workflow)
{
    ASSERT_TRUE(shared::isWorkbenchProfile());

    EXPECT_FALSE(isEffectsCommandAllowed(EFFECTS_GET_EFFECTS_COMMAND));
    EXPECT_FALSE(isEffectsActionAllowed("get-effects"));

    EXPECT_TRUE(isEffectsCommandAllowed(EFFECTS_PLUGIN_MANAGER_COMMAND));
    EXPECT_TRUE(isEffectsCommandAllowed(EFFECTS_TOGGLE_VENDOR_UI_COMMAND));
    EXPECT_TRUE(isEffectsCommandAllowed(EFFECTS_PRESET_APPLY_COMMAND));
    EXPECT_TRUE(isEffectsCommandAllowed(EFFECTS_PRESET_SAVE_COMMAND));
    EXPECT_TRUE(isEffectsCommandAllowed(makeEffectOpenCommand(u"test.effect")));
    EXPECT_TRUE(isEffectsCommandAllowed(makeEffectApplyCommand(u"test.effect")));
    EXPECT_TRUE(isEffectsActionAllowed("plugin-manager"));
    EXPECT_TRUE(isEffectsActionAllowed("action://effects/presets/apply"));
}

} // namespace
} // namespace au::effects
