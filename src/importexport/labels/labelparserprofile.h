#pragma once

#include <QString>
#include <QStringList>

namespace au::importexport {
struct LabelParserProfile {
    QString id;
    QString displayName;
    QStringList suffixes;
    bool enabled = true;
    QString source;
};
}
