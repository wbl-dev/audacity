#pragma once

#include "project/iaguproject.h"

#include "../iwavepainter.h"
#include "WaveMetrics.h"

namespace agu::projectscene::wavepainterutils {
IWavePainter::PlotType getPlotType(std::shared_ptr<agu::project::IAGuProject> project, const trackedit::ClipKey& clipKey, double zoom);
WaveMetrics getWaveMetrics(std::shared_ptr<agu::project::IAGuProject> project, const trackedit::ClipKey& clipKey,
                           const IWavePainter::Params& params, bool snapToSamples = false);
}
