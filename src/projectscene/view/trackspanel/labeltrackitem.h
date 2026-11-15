/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "trackitem.h"

namespace agu::projectscene {
class LabelTrackItem : public TrackItem
{
    Q_OBJECT

public:
    explicit LabelTrackItem(QObject* parent = nullptr);

protected:
    bool isAudible() const override;
};
}
