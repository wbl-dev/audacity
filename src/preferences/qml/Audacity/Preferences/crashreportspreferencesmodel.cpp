/*
 * Audacity: A Digital Audio Editor
 */
#include "crashreportspreferencesmodel.h"

#include "muse_framework_config.h"

using namespace au::appshell;

CrashReportsPreferencesModel::CrashReportsPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

bool CrashReportsPreferencesModel::isCrashReportingAvailable() const
{
#ifdef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
    return true;
#else
    return false;
#endif
}

bool CrashReportsPreferencesModel::sendCrashReports() const
{
    return diagnosticsConfiguration()->isDumpUploadAllowed();
}

void CrashReportsPreferencesModel::setSendCrashReports(bool send)
{
    if (send == sendCrashReports()) {
        return;
    }

    diagnosticsConfiguration()->setIsDumpUploadAllowed(send);
    emit sendCrashReportsChanged(send);
}
