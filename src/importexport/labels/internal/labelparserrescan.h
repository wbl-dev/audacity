#pragma once

#include <QStringList>

#include "../ilabelsconfiguration.h"

namespace au::importexport {
struct LabelParserRescanResult {
    muse::Ret ret;
    QStringList supported;
    QStringList added;
    QStringList removed;
};

LabelParserRescanResult rescanLabelParsers(ILabelsConfiguration& configuration);
}
