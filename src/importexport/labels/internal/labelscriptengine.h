/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "types/retval.h"
#include "labelscriptdata.h"

namespace au::importexport {
class LabelScriptEngine
{
public:
    explicit LabelScriptEngine(QString workerExecutable = {});

    muse::RetVal<ParsedLabelTrack> parse(const QString& source, const LabelScriptInput& input) const;

private:
    QString m_workerExecutable;
    bool m_launchApplicationWorker = false;
};
}
