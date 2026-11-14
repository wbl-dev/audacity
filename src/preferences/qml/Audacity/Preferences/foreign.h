/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include <QtQml/qqmlregistration.h>

#include "importexport/import/types/importtypes.h"

namespace au::preferences {
struct TempoDetectionPrefForeign
{
    Q_GADGET
    QML_FOREIGN(au::importexport::TempoDetectionPref)
    QML_NAMED_ELEMENT(TempoDetection)
    QML_UNCREATABLE("Not creatable from QML")
};

}
