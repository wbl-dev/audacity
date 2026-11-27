/*
* Audacity: A Digital Audio Editor
*/
#include "trackviewstatemodel.h"

using namespace agu::projectscene;
using namespace agu::project;

TrackViewStateModel::TrackViewStateModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

IProjectViewStatePtr TrackViewStateModel::viewState() const
{
    IAGuProjectPtr prj = globalContext()->currentProject();
    return prj ? prj->viewState() : nullptr;
}

void TrackViewStateModel::init()
{
    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        init();
    }, muse::async::Asyncable::Mode::SetReplace);

    IProjectViewStatePtr vs = viewState();
    if (!vs) {
        return;
    }

    if (m_trackId != -1) {
        m_trackHeight = vs->trackHeight(m_trackId);
        m_trackHeight.ch.onReceive(this, [this](int h) {
            if (m_trackHeight.val == h) {
                return;
            }

            auto prj = globalContext()->currentTrackeditProject();
            if (prj && !prj->track(m_trackId)) {
                return;
            }

            m_trackHeight.val = h;
            emit trackHeightChanged();
        }, muse::async::Asyncable::Mode::SetReplace);

        m_isTrackCollapsed = vs->isTrackCollapsed(m_trackId);
        m_isTrackCollapsed.ch.onReceive(this, [this](bool v) {
            if (m_isTrackCollapsed.val == v) {
                return;
            }

            m_isTrackCollapsed.val = v;
            emit isTrackCollapsedChanged();
        }, muse::async::Asyncable::Mode::SetReplace);

        m_interactiveResize = vs->interactiveResize(m_trackId);
        m_interactiveResize.ch.onReceive(this, [this](bool active) {
            m_interactiveResize.val = active;
            emit isInteractiveResizeChanged();
        }, muse::async::Asyncable::Mode::SetReplace);

        m_channelHeightRatio = vs->channelHeightRatio(m_trackId);
        m_channelHeightRatio.ch.onReceive(this, [this](double ratio) {
            if (m_channelHeightRatio.val == ratio) {
                return;
            }
            m_channelHeightRatio.val = ratio;
            emit channelHeightRatioChanged();
        }, muse::async::Asyncable::Mode::SetReplace);

        m_multiViewHeightRatio = vs->multiViewHeightRatio(m_trackId);
        m_multiViewHeightRatio.ch.onReceive(this, [this](double ratio) {
            m_multiViewHeightRatio.val = ratio;
            emit multiViewHeightRatioChanged();
        }, muse::async::Asyncable::Mode::SetReplace);
        m_viewType = vs->trackViewType(m_trackId);
        m_viewType.ch.onReceive(this, [this](trackedit::TrackViewType type) {
            m_viewType.val = type;
            emit isMultiViewChanged();
        }, muse::async::Asyncable::Mode::SetReplace);

        m_displayBounds = vs->verticalDisplayBounds(m_trackId);
        m_displayBounds.ch.onReceive(this, [this](std::pair<float, float> bounds) {
            if (m_displayBounds.val == bounds) {
                return;
            }
            m_displayBounds.val = bounds;
            emit displayBoundsChanged();
        }, muse::async::Asyncable::Mode::SetReplace);

        m_rulerType = vs->trackRulerType(m_trackId);
        m_rulerType.ch.onReceive(this, [this](int type) {
            if (m_rulerType.val == type) {
                return;
            }
            m_rulerType.val = type;
            emit isLinearChanged();
        }, muse::async::Asyncable::Mode::SetReplace);

        emit trackHeightChanged();
        emit isTrackCollapsedChanged();
        emit isInteractiveResizeChanged();
        emit channelHeightRatioChanged();
        emit displayBoundsChanged();
        emit multiViewHeightRatioChanged();
        emit isMultiViewChanged();
    }

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        emit isPlayingChanged();
    }, muse::async::Asyncable::Mode::SetReplace);

    recordController()->isRecordingChanged().onNotify(this, [this]() {
        emit isRecordingChanged();
    }, muse::async::Asyncable::Mode::SetReplace);

    m_meterModel = new playback::PlaybackMeterModel(this);
    emit meterModelChanged();
}

void TrackViewStateModel::changeTrackHeight(int deltaY)
{
    IProjectViewStatePtr vs = viewState();
    if (vs) {
        vs->changeTrackHeight(m_trackId, deltaY);
    }
}

void TrackViewStateModel::resetTrackHeight()
{
    if (auto vs = viewState()) {
        vs->resetTrackHeight(m_trackId);
    }
}

void TrackViewStateModel::setLabelRowCount(int rows)
{
    if (auto vs = viewState()) {
        vs->setLabelRowCount(m_trackId, rows);
    }
}

bool TrackViewStateModel::isInteractiveResize() const
{
    return m_interactiveResize.val;
}

void TrackViewStateModel::setInteractiveResize(bool active)
{
    if (auto vs = viewState()) {
        vs->setInteractiveResize(m_trackId, active);
    }
}

void TrackViewStateModel::changeChannelHeightRatio(double ratio)
{
    IProjectViewStatePtr vs = viewState();
    if (vs) {
        vs->setChannelHeightRatio(m_trackId, ratio);
    }
}

QVariant TrackViewStateModel::trackId() const
{
    return QVariant::fromValue(m_trackId);
}

void TrackViewStateModel::setTrackId(const QVariant& _newTrackId)
{
    trackedit::TrackId newTrackId = _newTrackId.toInt();
    if (m_trackId == newTrackId) {
        return;
    }
    m_trackId = newTrackId;
    emit trackIdChanged();

    init();
}

int TrackViewStateModel::trackHeight() const
{
    return m_trackHeight.val;
}

bool TrackViewStateModel::isTrackCollapsed() const
{
    return m_isTrackCollapsed.val;
}

double TrackViewStateModel::channelHeightRatio() const
{
    return m_channelHeightRatio.val;
}

QVariant TrackViewStateModel::displayBounds() const
{
    const auto& [min, max] = m_displayBounds.val;
    return QVariant::fromValue(QMap<QString, QVariant> {
        { "min", static_cast<double>(min) },
        { "max", static_cast<double>(max) }
    });
}

bool TrackViewStateModel::isLinear() const
{
    const auto prjViewState = viewState();
    if (!prjViewState) {
        return true;
    }

    const auto rulerType = prjViewState->trackRulerType(m_trackId).val;
    return rulerType != static_cast<int>(agu::trackedit::TrackRulerType::DbLog);
}

bool TrackViewStateModel::isPlaying() const
{
    return playbackController()->isPlaying();
}

bool TrackViewStateModel::isRecording() const
{
    return recordController()->isRecording();
}

agu::playback::PlaybackMeterModel* TrackViewStateModel::meterModel() const
{
    return m_meterModel;
}

double TrackViewStateModel::multiViewHeightRatio() const
{
    return m_multiViewHeightRatio.val;
}

void TrackViewStateModel::changeMultiViewHeightRatio(double ratio)
{
    if (auto vs = viewState()) {
        vs->setMultiViewHeightRatio(m_trackId, ratio);
    }
}

bool TrackViewStateModel::isMultiView() const
{
    return m_viewType.val == trackedit::TrackViewType::WaveformAndSpectrogram;
}

void TrackViewStateModel::toggleMultiView()
{
    if (auto vs = viewState()) {
        const bool expanded = isMultiView();
        vs->setTrackViewType(m_trackId, expanded ? trackedit::TrackViewType::Waveform : trackedit::TrackViewType::WaveformAndSpectrogram);
    }
}
