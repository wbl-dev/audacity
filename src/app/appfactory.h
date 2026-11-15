/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <memory>

#include "app_config.h"
#include "muse_framework_config.h"

#include "global/iapplication.h"

#include "cmdoptions.h"

namespace agu::app {
class AppFactory
{
public:
    AppFactory() = default;

    std::shared_ptr<muse::IApplication> newApp(const std::shared_ptr<AGuCmdOptions>& options) const;

private:
    std::shared_ptr<muse::IApplication> newGuiApp(const std::shared_ptr<AGuCmdOptions>& options) const;
    std::shared_ptr<muse::IApplication> newPluginRegistrationApp(const std::shared_ptr<AGuCmdOptions>& options) const;
};
}
