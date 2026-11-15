/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "abstractsequencepainter.h"

namespace agu::effects {
class LineSequencePainter : public AbstractSequencePainter
{
public:
    LineSequencePainter(const double& viewportX);

    void append(std::vector<SequenceSample> samples) override;

private:
    int numSamplesToDiscard() const;

    std::vector<QSGGeometry::Point2D> m_buffer;
};
} // namespace agu::effects
