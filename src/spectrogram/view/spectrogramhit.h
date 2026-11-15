/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <QObject>

namespace agu::spectrogram {
class SpectrogramHit
{
public:
    int trackId = -1;
    int channel = 0;
    double spectrogramY = 0; //! relative to the tracks container
    double spectrogramHeight = 0;
};

class SpectrogramHitFactory : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE SpectrogramHit createSpectrogramHit(int trackId, int channel, double spectrogramY, double spectrogramHeight);
    Q_INVOKABLE SpectrogramHit createNullSpectrogramHit()
    {
        return SpectrogramHit{};
    }
};
}
