/*
* Audacity: A Digital Audio Editor
*/
#include "addnewlabeltrackmodel.h"

using namespace agu::projectscene;
using namespace agu::trackedit;

AddNewLabelTrackModel::AddNewLabelTrackModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

agu::trackedit::TrackId AddNewLabelTrackModel::createLabelTrack(const QString& trackName)
{
    muse::RetVal<TrackId> retVal = trackeditInteraction()->newLabelTrack(trackName);
    if (!retVal.ret) {
        return trackedit::INVALID_TRACK;
    }

    return retVal.val;
}
