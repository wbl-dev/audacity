#include "labelscommandsregister.h"

#include "framework/global/types/translatablestring.h"
#include "../labelscommands.h"

using namespace au::importexport;
using namespace muse::rcommand;

namespace {
const std::vector<CommandInfo> infos {
    CommandInfo { RESCAN_LABEL_PARSERS_COMMAND,
                  muse::TranslatableString("command", "Rescan label parser scripts"),
                  muse::TranslatableString("command_description", "Reload JavaScript label parsers from the scripts folder"),
                  InputSchema(), Decoration() }
};
const std::vector<Command> commands { RESCAN_LABEL_PARSERS_COMMAND };
}

std::string LabelsCommandsRegister::moduleName() const { return "iex_labels"; }
const std::vector<Command>& LabelsCommandsRegister::commandList() const { return commands; }
const std::vector<CommandInfo>& LabelsCommandsRegister::commandInfoList() const { return infos; }
