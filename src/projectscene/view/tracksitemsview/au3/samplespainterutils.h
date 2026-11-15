#pragma once

#include "../iwavepainter.h"

#include "project/iaguproject.h"

#include "au3wrap/au3types.h"
#include "WaveMetrics.h"
#include "sampledata.h"

namespace agu::projectscene::samplespainterutils {
float FromDB(float value, double dBRange);
float ValueOfPixel(int yy, int height, bool offset, bool dB, double dBRange, float zoomMin, float zoomMax);
int getWaveYPos(float value, float min, float max, int height, bool dB, bool outer, float dBr, bool clip);
void drawBackground(QPainter& painter, const agu::projectscene::WaveMetrics& metrics, const IWavePainter::Style& style,
                    const double trimLeft);
void drawCenterLine(QPainter& painter, const agu::projectscene::WaveMetrics& metrics, const IWavePainter::Style& style, int y);
void drawClippedSamples(const agu::projectscene::SampleData& samples, const agu::projectscene::WaveMetrics& metrics, QPainter& painter,
                        const agu::projectscene::IWavePainter::Style& style);
SampleData getSampleData(const agu::au3::Au3WaveClip& clip, int channelIndex, const agu::projectscene::WaveMetrics& metrics, bool dB,
                         float dBRange, float zoomMax, float zoomMin);
std::optional<int> hitChannelIndex(std::shared_ptr<agu::project::IAGuProject> project, const trackedit::ClipKey& clipKey,
                                   const QPoint& position, const IWavePainter::Params& params);
std::optional<int> hitNearestSampleChannelIndex(std::shared_ptr<agu::project::IAGuProject> project, const trackedit::ClipKey& clipKey,
                                                const QPoint& position, const IWavePainter::Params& params);
void setLastClickPos(const unsigned int currentChannel, std::shared_ptr<agu::project::IAGuProject> project,
                     const trackedit::ClipKey& clipKey, const QPoint& lastPosition, const QPoint& currentPosition,
                     const IWavePainter::Params& params);
void smoothLastClickPos(const unsigned int currentChannel, std::shared_ptr<agu::project::IAGuProject> project,
                        const trackedit::ClipKey& clipKey, const QPoint& currentPosition, const IWavePainter::Params& params);
void setIsolatedPoint(const unsigned int currentChannel, const trackedit::ClipKey& clipKey,
                      std::shared_ptr<agu::project::IAGuProject> project, const QPoint& isolatedPoint, const QPoint& currentPosition,
                      const IWavePainter::Params& params);
}
