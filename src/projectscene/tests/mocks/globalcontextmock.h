#pragma once

#include "context/iglobalcontext.h"
#include <gmock/gmock.h>

namespace agu::projectscene {
class GlobalContextMock : public context::IGlobalContext
{
public:
    MOCK_METHOD(agu::trackedit::ITrackeditProjectPtr, currentTrackeditProject, (), (const, override));
    MOCK_METHOD(void, setCurrentProject, (const agu::project::IAGuProjectPtr&), (override));
    MOCK_METHOD(agu::project::IAGuProjectPtr, currentProject, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentProjectChanged, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentTrackeditProjectChanged, (), (const, override));
    MOCK_METHOD(void, setPlayer, (const agu::playback::IPlayerPtr&), (override));
    MOCK_METHOD(context::IPlaybackStatePtr, playbackState, (), (const, override));

    MOCK_METHOD(bool, isRecording, (), (const, override));
    MOCK_METHOD(muse::async::Notification, isRecordingChanged, (), (const, override));
};
}
