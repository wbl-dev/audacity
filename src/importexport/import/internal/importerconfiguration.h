/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "../iimporterconfiguration.h"

namespace au::importexport {
class ImporterConfiguration : public IImporterConfiguration
{
public:
    ImporterConfiguration() = default;

    void init();

    TempoDetectionPref::TempoDetection tempoDetectionPref() const override;
    void setTempoDetectionPref(TempoDetectionPref::TempoDetection pref) override;
    muse::async::Notification tempoDetectionPrefChanged() const override;

    LoopAction subsequentImportLoopAction() const override;
    void setSubsequentImportLoopAction(LoopAction action) override;
    muse::async::Notification subsequentImportLoopActionChanged() const override;

private:
    muse::async::Notification m_tempoDetectionPrefChanged;
    muse::async::Notification m_subsequentImportLoopActionChanged;
};
}
