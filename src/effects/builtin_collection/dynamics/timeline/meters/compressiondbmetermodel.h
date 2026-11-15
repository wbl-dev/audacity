/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "abstractdbmetermodel.h"

namespace agu::effects {
class CompressionDbMeterModel : public AbstractDbMeterModel
{
    Q_OBJECT

public:
    explicit CompressionDbMeterModel(QObject* parent = nullptr);

    float latestValue() override;
};
} // namespace agu::effects
