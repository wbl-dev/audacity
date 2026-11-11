#pragma once

#include "framework/rcommand/commandable.h"
#include "framework/rcommand/icommanddispatcher.h"
#include "framework/interactive/iinteractive.h"
#include "framework/global/modularity/ioc.h"
#include "../ilabelsconfiguration.h"

namespace au::importexport {
class LabelsCommandsController : public muse::rcommand::Commandable, public muse::Contextable
{
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher { this };
    muse::GlobalInject<ILabelsConfiguration> configuration;
    muse::ContextInject<muse::IInteractive> interactive { this };

public:
    explicit LabelsCommandsController(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}
    void init();
};
}
