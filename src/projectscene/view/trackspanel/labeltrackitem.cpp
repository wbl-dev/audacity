/*
* Audacity: A Digital Audio Editor
*/

#include "labeltrackitem.h"

using namespace agu::projectscene;
using namespace agu::trackedit;

LabelTrackItem::LabelTrackItem(QObject* parent)
    : TrackItem(parent)
{
}

bool LabelTrackItem::isAudible() const
{
    return false;
}
