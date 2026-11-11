#include "labelscommandscontroller.h"

#include "framework/global/translation.h"
#include "../labelscommands.h"
#include "labelparserrescan.h"

#include <QStringList>

using namespace au::importexport;

void LabelsCommandsController::init()
{
    commandDispatcher()->onRequest(this, RESCAN_LABEL_PARSERS_COMMAND, [this]() {
        const auto result = rescanLabelParsers(*configuration());
        const auto suffixes = [](const QStringList& values) {
            return values.isEmpty() ? muse::qtrc("importexport", "None") : "." + values.join(", .");
        };
        QStringList lines;
        if (!result.ret.success()) {
            lines.push_back(QString::fromStdString(result.ret.text()));
        }
        lines.push_back(muse::qtrc("importexport", "Currently supported: %1").arg(suffixes(result.supported)));
        lines.push_back(muse::qtrc("importexport", "Added: %1").arg(suffixes(result.added)));
        lines.push_back(muse::qtrc("importexport", "Removed: %1").arg(suffixes(result.removed)));
        if (result.ret.success()) {
            interactive()->info(muse::trc("importexport", "Label parsers"),
                                lines.join("\n").toStdString());
        } else {
            interactive()->error(muse::trc("importexport", "Label parser scan"),
                                 lines.join("\n").toStdString());
        }
        return result.ret;
    });
}
