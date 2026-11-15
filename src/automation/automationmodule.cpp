#include "automationmodule.h"

#include "internal/au3/au3clipgaininteraction.h"

#include "view/clipgainmodel.h"

#include <QtQml/qqml.h>

using namespace agu::automation;

static const std::string mname("automation");

AutomationModule::AutomationModule()
{}

std::string AutomationModule::moduleName() const
{
    return mname;
}

void AutomationModule::registerUiTypes()
{
    qmlRegisterType<ClipGainModel>("AGu.Automation", 1, 0, "ClipGainModel");
}

muse::modularity::IContextSetup* AutomationModule::newContext(
    const muse::modularity::ContextPtr& ctx) const
{
    return new AutomationContext(ctx);
}

// =====================================================
// ProjectSceneContext
// =====================================================

void AutomationContext::registerExports()
{
    ioc()->registerExport<IClipGainInteraction>(mname, std::make_shared<Au3ClipGainInteraction>(iocContext()));
}
