/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "framework/global/modularity/imoduleinterface.h"
#include "framework/global/async/notification.h"

#include "project/iaguproject.h"
#include "playback/iplayer.h"
#include "iplaybackstate.h"

namespace agu::context {
class IGlobalContext : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(agu::context::IGlobalContext)

public:
    virtual ~IGlobalContext() = default;

    virtual void setCurrentProject(const agu::project::IAGuProjectPtr& project) = 0;
    virtual agu::project::IAGuProjectPtr currentProject() const = 0;
    virtual muse::async::Notification currentProjectChanged() const = 0;

    virtual agu::trackedit::ITrackeditProjectPtr currentTrackeditProject() const = 0;
    virtual muse::async::Notification currentTrackeditProjectChanged() const = 0;

    virtual void setPlayer(const agu::playback::IPlayerPtr& player) = 0;
    virtual IPlaybackStatePtr playbackState() const = 0;

    virtual bool isRecording() const = 0;
    virtual muse::async::Notification isRecordingChanged() const = 0;
};
}
