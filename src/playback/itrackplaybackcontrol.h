/*
* Audacity: A Digital Audio Editor
*/

#pragma once

#include "audio/audiotypes.h"
#include "trackedit/trackedittypes.h"
#include "framework/global/async/channel.h"
#include "framework/global/modularity/imoduleinterface.h"

namespace agu::playback {
class ITrackPlaybackControl : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ITrackPlaybackControl)

public:
    virtual ~ITrackPlaybackControl() = default;

    virtual void setVolume(long trackId, agu::audio::volume_dbfs_t volume, bool completed) = 0;
    virtual agu::audio::volume_dbfs_t volume(long trackId) const = 0;

    virtual void setPan(long trackId, agu::audio::pan_t pan, bool completed) = 0;
    virtual agu::audio::pan_t pan(long trackId) const = 0;

    virtual void setSolo(long trackId, bool solo, bool exclusive = false) = 0;
    virtual bool solo(long trackId) const = 0;

    virtual void setMuted(long trackId, bool mute, bool exclusive = false) = 0;
    virtual void setMuted(const trackedit::TrackIdList& trackIds, bool mute) = 0;
    virtual bool muted(long trackId) const = 0;

    virtual muse::async::Channel<long> muteOrSoloChanged() const = 0;
};
}
