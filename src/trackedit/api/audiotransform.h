/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <cstdint>

#include <QObject>
#include <QString>

namespace agu::trackedit::api {
class AudioTransformTask
{
public:
    virtual ~AudioTransformTask() = default;

    virtual bool cancelled() const = 0;
    virtual bool report(double fraction, const QString& message) = 0;
};

class AudioTransform : public QObject
{
public:
    using QObject::QObject;
    ~AudioTransform() override = default;

    virtual bool apply(float* const channels[], uint32_t channelCount, uint64_t sampleCount) = 0;
};
} // namespace agu::trackedit::api
