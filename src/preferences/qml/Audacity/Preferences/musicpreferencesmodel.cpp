/*
 * Audacity: A Digital Audio Editor
 */
#include "musicpreferencesmodel.h"

namespace au::appshell {
MusicPreferencesModel::MusicPreferencesModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void MusicPreferencesModel::init()
{
    importerConfiguration()->tempoDetectionPrefChanged().onNotify(this, [this] {
        emit tempoDetectionPrefChanged();
    });

    importerConfiguration()->subsequentImportLoopActionChanged().onNotify(this, [this] {
        emit askBeforeSubsequentImportChanged();
    });
}

importexport::TempoDetectionPref::TempoDetection MusicPreferencesModel::tempoDetectionPref() const
{
    return importerConfiguration()->tempoDetectionPref();
}

void MusicPreferencesModel::setTempoDetectionPref(importexport::TempoDetectionPref::TempoDetection pref)
{
    importerConfiguration()->setTempoDetectionPref(pref);
}

bool MusicPreferencesModel::askBeforeSubsequentImport() const
{
    return importerConfiguration()->subsequentImportLoopAction() == importexport::LoopAction::Ask;
}

void MusicPreferencesModel::setAskBeforeSubsequentImport(bool ask)
{
    if (askBeforeSubsequentImport() == ask) {
        return;
    }

    importerConfiguration()->setSubsequentImportLoopAction(
        ask ? importexport::LoopAction::Ask : importexport::LoopAction::MatchLoopToProject);
}
}
