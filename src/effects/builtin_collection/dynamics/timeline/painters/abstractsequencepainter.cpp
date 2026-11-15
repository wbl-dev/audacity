/*
 * Audacity: A Digital Audio Editor
 */
#include "abstractsequencepainter.h"

namespace agu::effects {
AbstractSequencePainter::AbstractSequencePainter(const std::atomic<double>& viewportX)
    : m_viewportX{viewportX},
    m_geometry{QSGGeometry::defaultAttributes_Point2D(), 0} {}
} // namespace agu::effects
