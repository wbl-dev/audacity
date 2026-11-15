/*
 * Audacity: A Digital Audio Editor
 */
#include "lv2effectsstubmodule.h"

static void lv2_init_qrc()
{
    Q_INIT_RESOURCE(lv2);
}

namespace agu::effects {
std::string Lv2EffectsModule::moduleName() const
{
    return "effects_lv2_stub";
}

void Lv2EffectsModule::registerResources()
{
    lv2_init_qrc();
}
}
