/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "importexport/import/iimporterconfiguration.h"

namespace au::appshell {
class MusicPreferencesModel : public QObject, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    muse::GlobalInject<au::importexport::IImporterConfiguration> importerConfiguration;

    Q_PROPERTY(importexport::TempoDetectionPref::TempoDetection tempoDetectionPref READ tempoDetectionPref NOTIFY tempoDetectionPrefChanged)
    Q_PROPERTY(
        bool askBeforeSubsequentImport READ askBeforeSubsequentImport WRITE setAskBeforeSubsequentImport NOTIFY askBeforeSubsequentImportChanged)

public:
    explicit MusicPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    importexport::TempoDetectionPref::TempoDetection tempoDetectionPref() const;
    Q_INVOKABLE void setTempoDetectionPref(importexport::TempoDetectionPref::TempoDetection pref);

    bool askBeforeSubsequentImport() const;
    void setAskBeforeSubsequentImport(bool ask);

signals:
    void tempoDetectionPrefChanged();
    void askBeforeSubsequentImportChanged();
};
}
