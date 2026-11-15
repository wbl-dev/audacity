/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "playbackstate.h"
#include "record/irecordcontroller.h"

namespace agu::context {
class GlobalContext : public agu::context::IGlobalContext, public muse::Contextable
{
    muse::ContextInject<agu::record::IRecordController> recordController { this };

public:
    GlobalContext(muse::modularity::ContextPtr ctx)
        : muse::Contextable(std::move(ctx))
        , m_playbackState(std::make_shared<PlaybackState>())
    {
    }

    void setCurrentProject(const agu::project::IAGuProjectPtr& project) override;
    agu::project::IAGuProjectPtr currentProject() const override;
    muse::async::Notification currentProjectChanged() const override;

    agu::trackedit::ITrackeditProjectPtr currentTrackeditProject() const override;
    muse::async::Notification currentTrackeditProjectChanged() const override;

    void setPlayer(const agu::playback::IPlayerPtr& player) override;
    IPlaybackStatePtr playbackState() const override;

    bool isRecording() const override;
    muse::async::Notification isRecordingChanged() const override;

private:
    agu::project::IAGuProjectPtr m_currentProject;
    muse::async::Notification m_currentProjectChanged;

    std::shared_ptr<PlaybackState> m_playbackState;
};
}
