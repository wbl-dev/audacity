/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <memory>

#include "app_config.h"
#include "muse_framework_config.h"

#include "global/iapplication.h"

#include "cmdoptions.h"

namespace au::app {
struct WorkbenchModulePolicy
{
    bool diagnostics;
    bool genericNetwork;
    bool cloud;
    bool museHub;
    bool usageInfo;
    bool update;
};

constexpr WorkbenchModulePolicy modulePolicy() noexcept
{
    return {
#ifdef MUSE_MODULE_DIAGNOSTICS
        true,
#else
        false,
#endif
#ifdef MUSE_MODULE_NETWORK
        true,
#else
        false,
#endif
#ifdef MUSE_MODULE_CLOUD
        true,
#else
        false,
#endif
#ifdef AU_BUILD_MUSEHUB_MODULE
        true,
#else
        false,
#endif
#ifdef AU_BUILD_USAGEINFO_MODULE
        true,
#else
        false,
#endif
#ifdef MUSE_MODULE_UPDATE
        true,
#else
        false,
#endif
    };
}

class AppFactory
{
public:
    AppFactory() = default;

    std::shared_ptr<muse::IApplication> newApp(const std::shared_ptr<AudacityCmdOptions>& options) const;

private:
    std::shared_ptr<muse::IApplication> newGuiApp(const std::shared_ptr<AudacityCmdOptions>& options) const;
    std::shared_ptr<muse::IApplication> newPluginRegistrationApp(const std::shared_ptr<AudacityCmdOptions>& options) const;
};
}
