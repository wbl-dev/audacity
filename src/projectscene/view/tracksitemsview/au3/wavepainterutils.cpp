#include "wavepainterutils.h"

#include "global/realfn.h"

#include "au3wrap/internal/domaccessor.h"

static constexpr auto PIXELS_PER_SAMPLE_WHEN_CONNECTING_POINTS = 0.5;
static constexpr auto PIXELS_PER_SAMPLE_WHEN_INDIVIDUAL_POINTS = 4;

namespace agu::projectscene::wavepainterutils {
IWavePainter::PlotType getPlotType(std::shared_ptr<agu::project::IAGuProject> project, const trackedit::ClipKey& clipKey, double zoom)
{
    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());
    WaveTrack* track = agu::au3::DomAccessor::findWaveTrack(*au3Project, TrackId(clipKey.trackId));
    if (!track) {
        return IWavePainter::PlotType::MinMaxRMS;
    }

    std::shared_ptr<WaveClip> waveClip = agu::au3::DomAccessor::findWaveClip(track, clipKey.itemId);
    if (!waveClip) {
        return IWavePainter::PlotType::MinMaxRMS;
    }

    const double sampleRate = waveClip->GetRate();
    const double stretchRatio = waveClip->GetStretchRatio();

    const double rate = sampleRate / stretchRatio;

    const double threshold1 = PIXELS_PER_SAMPLE_WHEN_CONNECTING_POINTS * rate;
    if (zoom < threshold1) {
        return IWavePainter::PlotType::MinMaxRMS;
    }

    const double threshold2 = PIXELS_PER_SAMPLE_WHEN_INDIVIDUAL_POINTS * rate;
    if (zoom < threshold2) {
        return IWavePainter::PlotType::ConnectingDots;
    }

    return IWavePainter::PlotType::Stem;
}

WaveMetrics getWaveMetrics(std::shared_ptr<agu::project::IAGuProject> project, const trackedit::ClipKey& clipKey,
                           const IWavePainter::Params& params, bool snapToSamples)
{
    WaveMetrics wm;

    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());
    WaveTrack* track = agu::au3::DomAccessor::findWaveTrack(*au3Project, TrackId(clipKey.trackId));
    IF_ASSERT_FAILED(track) {
        return wm;
    }

    std::shared_ptr<WaveClip> waveClip = agu::au3::DomAccessor::findWaveClip(track, clipKey.itemId);
    IF_ASSERT_FAILED(waveClip) {
        return wm;
    }

    wm.zoom = params.zoom;
    wm.fromTime = params.fromTime;
    wm.toTime = params.toTime;
    wm.selectionStartTime = 0;
    wm.selectionEndTime = 0;
    wm.width = params.geometry.width;
    wm.left = params.geometry.left;

    // calculate selection area relative to the clip itself
    if (!muse::RealIsEqual(params.selectionStartTime, params.selectionEndTime)) {
        const double relativeStartTime = params.selectionStartTime - waveClip->Start();
        const double relativeEndTime = params.selectionEndTime - waveClip->Start();

        const auto snapToSampleTimeConv = [&](double time) {
            const double multiplier = waveClip->GetRate() / waveClip->GetStretchRatio();
            return std::round(time * multiplier) / multiplier;
        };

        wm.selectionStartTime = (snapToSamples ? snapToSampleTimeConv(relativeStartTime) : relativeStartTime) + waveClip->GetTrimLeft();
        wm.selectionEndTime = (snapToSamples ? snapToSampleTimeConv(relativeEndTime) : relativeEndTime) + waveClip->GetTrimLeft();
    }

    return wm;
}
}
