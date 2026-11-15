/*
 * Audacity: A Digital Audio Editor
 */

#include <gtest/gtest.h>

#include "effectscommands.h"

namespace agu::effects {
namespace {

TEST(workbench_effect_commands, keeps_local_effect_commands)
{
    EXPECT_TRUE(EFFECTS_PLUGIN_MANAGER_COMMAND.isValid());
    EXPECT_TRUE(EFFECTS_TOGGLE_VENDOR_UI_COMMAND.isValid());
    EXPECT_TRUE(EFFECTS_PRESET_APPLY_COMMAND.isValid());
    EXPECT_TRUE(EFFECTS_PRESET_SAVE_COMMAND.isValid());
    EXPECT_TRUE(makeEffectOpenCommand(u"test.effect").isValid());
    EXPECT_TRUE(makeEffectApplyCommand(u"test.effect").isValid());
}

} // namespace
} // namespace agu::effects
