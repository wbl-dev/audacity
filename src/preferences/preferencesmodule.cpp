#include "preferencesmodule.h"

#include "framework/global/modularity/ioc.h"

#include "framework/interactive/iinteractiveuriregister.h"

using namespace agu::preferences;

std::string PreferencesModule::moduleName() const
{
    return "preferences";
}

void PreferencesModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(muse::Uri("agu://preferences"), "AGu.Preferences", "PreferencesDialog");
    }
}
