/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

#include "modularity/ioc.h"
#include "diagnostics/idiagnosticsconfiguration.h"

namespace au::appshell {
class CrashReportsPreferencesModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    muse::GlobalInject<muse::diagnostics::IDiagnosticsConfiguration> diagnosticsConfiguration;

    Q_PROPERTY(bool sendCrashReports READ sendCrashReports WRITE setSendCrashReports NOTIFY sendCrashReportsChanged)

public:
    explicit CrashReportsPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE bool isCrashReportingAvailable() const;

    bool sendCrashReports() const;

public slots:
    void setSendCrashReports(bool send);

signals:
    void sendCrashReportsChanged(bool send);
};
}
