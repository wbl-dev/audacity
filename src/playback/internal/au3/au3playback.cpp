#include "au3playback.h"

#include "au3player.h"
#include "au3audiooutput.h"

using namespace agu::playback;

agu::playback::IPlayerPtr Au3Playback::player(TrackSequenceId) const
{
    if (!m_player) {
        m_player = std::make_shared<Au3Player>(iocContext());
    }
    return m_player;
}

std::shared_ptr<agu::playback::IAudioOutput> Au3Playback::audioOutput() const
{
    if (!m_audioOutput) {
        m_audioOutput = std::make_shared<Au3AudioOutput>(iocContext());
    }
    return m_audioOutput;
}
