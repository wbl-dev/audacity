/*
* Audacity: A Digital Audio Editor
*/
#include "selectionstatusmodel.h"
#include <algorithm>
#include <QStringList>

#include "playback/iaudiooutput.h"

using namespace agu::projectscene;

SelectionStatusModel::SelectionStatusModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void SelectionStatusModel::init()
{
    m_currentFormat = configuration()->selectionTimecodeFormat();
    emit currentFormatChanged();

    m_durationFormat = configuration()->durationTimecodeFormat();
    emit durationFormatChanged();

    m_startTime = selectionController()->dataSelectedStartTime();
    selectionController()->dataSelectedStartTimeChanged().onReceive(this, [this](trackedit::secs_t time) {
        m_startTime = !time.is_negative() ? time : trackedit::secs_t(0.0);
        emit startTimeChanged();
    });

    m_endTime = selectionController()->dataSelectedEndTime();
    selectionController()->selectedTracksChanged().onReceive(this, [this](const trackedit::TrackIdList&) {
        emit targetTracksChanged();
    });
    selectionController()->dataSelectedEndTimeChanged().onReceive(this, [this](trackedit::secs_t time) {
        m_endTime = !time.is_negative() ? time : trackedit::secs_t(0.0);
        emit endTimeChanged();
    });

    playback()->audioOutput()->sampleRateChanged().onReceive(this, [this](audio::sample_rate_t) {
        emit sampleRateChanged();
    });

    globalContext()->currentTrackeditProjectChanged().onNotify(this, [this](){
        emit isEnabledChanged();
        onProjectChanged();
    });

    onProjectChanged();

    configuration()->selectionTimecodeFormatChanged().onNotify(this, [this](){
        setCurrentFormat(configuration()->selectionTimecodeFormat());
    });

    configuration()->durationTimecodeFormatChanged().onNotify(this, [this](){
        setDurationFormat(configuration()->durationTimecodeFormat());
    });
}

void SelectionStatusModel::onProjectChanged()
{
    auto project = globalContext()->currentTrackeditProject();
    if (project) {
        project->trackChanged().onReceive(this, [this](const trackedit::Track&) {
            emit targetTracksChanged();
        });
        project->timeSignatureChanged().onReceive(this, [this](const trackedit::TimeSignature&) {
            emit timeSignatureChanged();
        });
    }

    emit timeSignatureChanged();
    emit targetTracksChanged();
}

QString SelectionStatusModel::targetTracks() const
{
    const auto project = globalContext()->currentTrackeditProject();
    if (!project) {
        return {};
    }
    const auto selected = selectionController()->selectedTracks();
    QStringList names;
    for (const auto& track : project->trackList()) {
        if (track.type != trackedit::TrackType::Label && std::find(selected.begin(), selected.end(), track.id) != selected.end()) {
            names << track.title.toQString();
        }
    }
    return names.join(QStringLiteral(", "));
}

double SelectionStatusModel::startTime() const
{
    return m_startTime;
}

void SelectionStatusModel::setStartTime(double time)
{
    if (qFuzzyCompare(m_startTime, time)) {
        return;
    }

    selectionController()->setDataSelectedStartTime(time, true);
}

double SelectionStatusModel::endTime() const
{
    return m_endTime;
}

void SelectionStatusModel::setEndTime(double time)
{
    if (qFuzzyCompare(m_endTime, time)) {
        return;
    }

    selectionController()->setDataSelectedEndTime(time, true);
}

int SelectionStatusModel::currentFormat() const
{
    return m_currentFormat;
}

void SelectionStatusModel::setCurrentFormat(int format)
{
    if (m_currentFormat == format) {
        return;
    }

    m_currentFormat = format;
    emit currentFormatChanged();
    configuration()->setSelectionTimecodeFormat(format);
}

int SelectionStatusModel::durationFormat() const
{
    return m_durationFormat;
}

void SelectionStatusModel::setDurationFormat(int format)
{
    if (m_durationFormat == format) {
        return;
    }

    m_durationFormat = format;
    emit durationFormatChanged();
    configuration()->setDurationTimecodeFormat(format);
}

double SelectionStatusModel::sampleRate() const
{
    return playback()->audioOutput()->sampleRate();
}

double SelectionStatusModel::tempo() const
{
    auto project = globalContext()->currentTrackeditProject();
    if (!project) {
        return 0.0;
    }

    return project->timeSignature().tempo;
}

int SelectionStatusModel::upperTimeSignature() const
{
    auto project = globalContext()->currentTrackeditProject();
    if (!project) {
        return 0;
    }

    return project->timeSignature().upper;
}

int SelectionStatusModel::lowerTimeSignature() const
{
    auto project = globalContext()->currentTrackeditProject();
    if (!project) {
        return 0;
    }

    return project->timeSignature().lower;
}

bool SelectionStatusModel::isEnabled() const
{
    return globalContext()->currentTrackeditProject() != nullptr;
}
