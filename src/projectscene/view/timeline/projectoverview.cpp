/*
 * Audacity: A Digital Audio Editor
 */
#include "projectoverview.h"

#include <algorithm>
#include <cmath>

using namespace agu::projectscene;

ProjectOverview::ProjectOverview(QQuickItem* parent)
    : QQuickPaintedItem(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
    setClip(true);
}

void ProjectOverview::init()
{
    reload();
}

double ProjectOverview::duration() const
{
    return m_duration;
}

void ProjectOverview::reload()
{
    // Disconnect before releasing the old project/list notification objects.
    async_disconnectAll();
    m_clipLists.clear();
    m_trackColors.clear();
    m_project = globalContext()->currentTrackeditProject();
    globalContext()->currentProjectChanged().onNotify(this, [this]() { reload(); });
    if (m_project) {
        const auto reloadTracks = [this](auto...) { reload(); };
        m_project->tracksChanged().onReceive(this, reloadTracks);
        m_project->trackAdded().onReceive(this, reloadTracks);
        m_project->trackInserted().onReceive(this, reloadTracks);
        m_project->trackRemoved().onReceive(this, reloadTracks);
        m_project->trackMoved().onReceive(this, reloadTracks);
        m_project->trackChanged().onReceive(this, [this](const trackedit::Track& track) {
            if (updateTrackColor(track)) {
                refresh();
            }
        });
        m_project->trackClipListChanged().onReceive(this, reloadTracks);

        for (const auto& track : m_project->trackList()) {
            if (track.type != trackedit::TrackType::Mono && track.type != trackedit::TrackType::Stereo) {
                continue;
            }
            m_trackColors.insert(track.id, track.colorIndex);
            auto clips = m_project->clipList(track.id);
            clips.onChanged(this, [this]() { refresh(); });
            clips.onItemAdded(this, [this](auto) { refresh(); });
            clips.onItemRemoved(this, [this](auto) { refresh(); });
            clips.onItemChanged(this, [this](auto) { refresh(); });
            m_clipLists.push_back(std::move(clips));
        }
    }
    refresh();
}

bool ProjectOverview::updateTrackColor(const trackedit::Track& track)
{
    if (track.type != trackedit::TrackType::Mono && track.type != trackedit::TrackType::Stereo) {
        return false;
    }
    const auto it = m_trackColors.constFind(track.id);
    if (it != m_trackColors.cend() && *it == track.colorIndex) {
        return false;
    }
    m_trackColors.insert(track.id, track.colorIndex);
    return true;
}

void ProjectOverview::refresh()
{
    const double duration = m_project ? std::max(1.0, m_project->totalTime().to_double()) : 1.0;
    if (!qFuzzyCompare(duration, m_duration)) {
        m_duration = duration;
        emit durationChanged();
    }
    update();
}

void ProjectOverview::paint(QPainter* painter)
{
    if (!m_project || width() <= 0 || height() <= 0 || !globalContext()->currentProject()) {
        return;
    }
    auto tracks = m_project->trackList();
    tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [](const auto& track) {
        return track.type != trackedit::TrackType::Mono && track.type != trackedit::TrackType::Stereo;
    }), tracks.end());
    if (tracks.empty()) {
        return;
    }

    const double zoom = width() / m_duration;
    // Overlay additional tracks in a few lanes. Dividing a 24px strip by every
    // track would truncate stereo channel heights to zero in the native cache.
    const size_t lanes = std::min(tracks.size(), static_cast<size_t>(std::max(1.0, std::floor(height() / 6.0))));
    const double bandHeight = std::max(2.0, height() / lanes);
    for (size_t i = 0; i < tracks.size(); ++i) {
        const auto& track = tracks[i];
        for (const auto& clip : m_project->clipList(track.id)) {
            const double start = std::max(0.0, clip.startTime);
            const double end = std::min(m_duration, clip.endTime);
            if (end <= start) {
                continue;
            }
            IWavePainter::Params params;
            params.zoom = zoom;
            params.geometry = { bandHeight, (end - start) * zoom, 0.0 };
            params.fromTime = start - clip.startTime;
            params.toTime = end - clip.startTime;
            params.channelHeightRatio = clip.stereo ? 0.5 : 1.0;
            const auto colorIndex = clip.isAutoColor ? track.colorIndex : clip.colorIndex;
            const QColor color = configuration()->clipColor(colorIndex).toQColor();
            params.style.samplePen = color;
            params.style.selectedSamplePen = color;
            params.style.rmsPen = color;
            params.style.rmsSelectedPen = color;
            params.style.centerLine = color;
            params.style.blankBrush = Qt::transparent;
            params.style.normalBackground = Qt::transparent;
            params.style.selectedBackground = Qt::transparent;
            params.style.envelopeBackground = Qt::transparent;
            params.style.selectedEnvelopeBackground = Qt::transparent;
            painter->save();
            painter->translate(start * zoom, (i % lanes) * bandHeight);
            // The native cache uses RGB images, so a transparent background is
            // rendered black. Lighten keeps earlier signals when lanes overlap.
            painter->setCompositionMode(QPainter::CompositionMode_Lighten);
            wavePainter()->paint(*painter, clip.key, params, IWavePainter::PlotType::MinMaxRMS);
            painter->restore();
        }
    }
}
