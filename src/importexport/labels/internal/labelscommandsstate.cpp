#include "labelscommandsstate.h"

using namespace au::importexport;
using namespace muse::rcommand;

std::string LabelsCommandsState::moduleName() const
{
    return "iex_labels";
}

void LabelsCommandsState::init()
{
}

void LabelsCommandsState::deinit()
{
}

CommandState LabelsCommandsState::commandState(const Command&) const
{
    return CommandState(true, false);
}

muse::async::Channel<Command, CommandState> LabelsCommandsState::commandStateChanged() const
{
    return m_commandStateChanged;
}
