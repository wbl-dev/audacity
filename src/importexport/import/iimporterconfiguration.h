/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "async/notification.h"
#include "modularity/imoduleinterface.h"

#include "types/importtypes.h"

#include <string>

namespace agu::importexport {
class IImporterConfiguration : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IImporterConfiguration)

public:
    virtual ~IImporterConfiguration() = default;

    /// Always / Never; the persisted workspace-dependent value is migrated.
    virtual TempoDetectionPref::TempoDetection tempoDetectionPref() const = 0;
    virtual void setTempoDetectionPref(TempoDetectionPref::TempoDetection pref) = 0;
    virtual muse::async::Notification tempoDetectionPrefChanged() const = 0;

    /// "Remember my choice" for the subsequent-import tempo detection dialog.
    virtual LoopAction subsequentImportLoopAction() const = 0;
    virtual void setSubsequentImportLoopAction(LoopAction action) = 0;
    virtual muse::async::Notification subsequentImportLoopActionChanged() const = 0;
};
}
