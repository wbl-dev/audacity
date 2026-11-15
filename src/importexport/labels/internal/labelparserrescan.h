#pragma once

#include <QStringList>

#include "../ilabelsconfiguration.h"

namespace agu::importexport {
struct LabelParserRescanResult {
    muse::Ret ret;
    QStringList supported;
    QStringList added;
    QStringList removed;
};

LabelParserRescanResult rescanLabelParsers(ILabelsConfiguration& configuration);
}
