#pragma once

#include "modularity/imodulesetup.h"

namespace agu::preferences {
class PreferencesModule : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;

    void resolveImports() override;
};
}
