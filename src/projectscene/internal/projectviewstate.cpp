/*
* Audacity: A Digital Audio Editor
*/

#include <memory>
#include <algorithm>
#include <cmath>

#include <QEvent>
#include <QKeyEvent>

#include "framework/global/types/retval.h"
#include "framework/global/async/async.h"

#include "au3wrap/iau3project.h"
#include "au3wrap/internal/projectsnap.h"
#include "au3wrap/internal/domaccessor.h"
#include "au3wrap/internal/domconverter.h"
#include "au3wrap/au3types.h"
#include "trackedit/dom/track.h"

#include "au3/waveformscale.h"
#include "au3/viewinfo.h"
#include "au3/trackheightattachment.h"
#include "au3/trackrulertypeattachment.h"
#include "au3/trackviewtypeattachment.h"

#include "projectviewstate.h"

using namespace agu::projectscene;

constexpr int TRACK_DEFAULT_HEIGHT = 220;
constexpr int TRACK_LABEL_DEFAULT_HEIGHT = 86;
constexpr int TRACK_MIN_HEIGHT = 100;
constexpr int TRACK_COLLAPSE_HEIGHT = 120;
constexpr int TRACK_MULTIVIEW_MIN_HEIGHT = 280;
constexpr int TRACK_MULTIVIEW_EXTRA_HEIGHT = 240;
constexpr int LABEL_TRACK_MIN_HEIGHT = 44;
constexpr int LABEL_TRACK_COLLAPSE_HEIGHT = 72;

namespace {
constexpr int numDecimals(float value)
{
    constexpr int MAX_DECIMAL_PLACES = 4;
    constexpr float POWER_OF_TEN = 10000.0f;

    int rounded_scaled_value = static_cast<int>((value * POWER_OF_TEN) + 0.5f);
    int integer_part = static_cast<int>(value);
    int decimal_part_scaled = rounded_scaled_value - (integer_part * static_cast<int>(POWER_OF_TEN));

    if (decimal_part_scaled == 0) {
        return 0;
    }

    int count = MAX_DECIMAL_PLACES;
    int power_of_ten = 10;

    while (count > 0 && (decimal_part_scaled % power_of_ten) == 0) {
        count--;
        power_of_ten *= 10;
    }

    return count;
}

static_assert(numDecimals(0.14999999) == 2);
static_assert(numDecimals(1.00) == 0);
static_assert(numDecimals(0.005) == 3);

void saveProjectSnap(std::shared_ptr<agu::au3::IAu3Project> project, const Snap& snap)
{
    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());
    IF_ASSERT_FAILED(au3Project) {
        return;
    }
    auto& projectSnap = agu::au3::ProjectSnap::Get(*au3Project);
    projectSnap.setSnapType(static_cast<unsigned int>(snap.type));
    projectSnap.enableSnap(snap.enabled);
    projectSnap.setSnapTriplets(snap.isSnapTriplets);
}

Snap getProjectSnap(std::shared_ptr<agu::au3::IAu3Project> project)
{
    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());
    IF_ASSERT_FAILED(au3Project) {
        return Snap {};
    }
    auto& projectSnap = agu::au3::ProjectSnap::Get(*au3Project);
    return Snap {
        static_cast<SnapType>(projectSnap.snapType()),
        projectSnap.isSnapEnabled(),
        projectSnap.isSnapTriplets()
    };
}

void saveProjectZoomState(agu::au3::Au3Project* au3Project, const ZoomState& zoomState)
{
    auto& projectZoomState = agu::au3::ViewInfo::Get(*au3Project);
    projectZoomState.setZoom(zoomState.zoom);
    projectZoomState.setVPos(zoomState.tracksVerticalOffset);
    projectZoomState.setHPos(zoomState.frameStart);
}

ZoomState getProjectZoomState(agu::au3::Au3Project* au3Project)
{
    auto& projectZoomState = agu::au3::ViewInfo::Get(*au3Project);
    return ZoomState {
        projectZoomState.zoom(),
        projectZoomState.hPos(),
        projectZoomState.vPos()
    };
}

std::pair<float, float> getVerticalDisplayBounds(const std::shared_ptr<agu::project::IAGuProject>& project,
                                                 const agu::trackedit::TrackId& trackId)
{
    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());

    agu::au3::Au3WaveTrack* waveTrack = agu::au3::DomAccessor::findWaveTrack(*au3Project, agu::au3::Au3TrackId(trackId));
    if (waveTrack == nullptr) {
        return { -1.0f, 1.0f };
    }

    float min = 0.0f;
    float max = 0.0f;
    auto& cache = WaveformScale::Get(*waveTrack);
    cache.GetDisplayBounds(min, max);
    return { min, max };
}

void setVerticalDisplayBounds(const std::shared_ptr<agu::project::IAGuProject>& project, const agu::trackedit::TrackId& trackId,
                              const std::pair<float, float>& bounds)
{
    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());

    agu::au3::Au3WaveTrack* waveTrack = agu::au3::DomAccessor::findWaveTrack(*au3Project, agu::au3::Au3TrackId(trackId));
    if (waveTrack == nullptr) {
        return;
    }

    auto& cache = WaveformScale::Get(*waveTrack);
    cache.SetDisplayBounds(bounds.first, bounds.second);
}

agu::trackedit::TrackViewType getTrackViewType(const std::shared_ptr<agu::project::IAGuProject>& project,
                                              const agu::trackedit::TrackId& trackId)
{
    const agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());

    const agu::au3::Au3WaveTrack* waveTrack = agu::au3::DomAccessor::findWaveTrack(*au3Project, agu::au3::Au3TrackId(trackId));
    if (waveTrack == nullptr) {
        return agu::trackedit::TrackViewType::Undefined;
    }

    const auto& cache = agu::au3::TrackViewTypeAttachment::Get(waveTrack);
    const auto viewType = cache.GetTrackViewType();
    return viewType == agu::trackedit::TrackViewType::Undefined ? agu::trackedit::TrackViewType::Waveform : viewType;
}

int getTrackRulerType(const std::shared_ptr<agu::project::IAGuProject>& project, const agu::trackedit::TrackId& trackId)
{
    const agu::au3::Au3Project* au3Project = reinterpret_cast<const agu::au3::Au3Project*>(project->au3ProjectPtr());

    const agu::au3::Au3Track* track = agu::au3::DomAccessor::findTrack(*au3Project, agu::au3::Au3TrackId(trackId));
    if (track == nullptr) {
        return static_cast<int>(agu::trackedit::TrackRulerType::Linear);
    }

    const auto& cache = TrackRulerTypeAttachment::Get(track);
    return static_cast<int>(cache.GetRulerType());
}

void setTrackRulerType(const std::shared_ptr<agu::project::IAGuProject>& project, const agu::trackedit::TrackId& trackId, int rulerType)
{
    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());

    agu::au3::Au3Track* track = agu::au3::DomAccessor::findTrack(*au3Project, agu::au3::Au3TrackId(trackId));
    if (track == nullptr) {
        return;
    }

    auto& cache = TrackRulerTypeAttachment::Get(track);
    cache.SetRulerType(static_cast<agu::trackedit::TrackRulerType>(rulerType));
}
}

ProjectViewState::ProjectViewState(const muse::modularity::ContextPtr& ctx)
    : muse::Contextable(ctx)
{
    m_scrollAnchorAnimation.setDuration(170);
    m_scrollAnchorAnimation.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_scrollAnchorAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        m_tracksVerticalOffset.set(value.toInt());
    });
}

void ProjectViewState::init(const std::shared_ptr<au3::IAu3Project>& project)
{
    configuration()->setIsEffectsPanelVisible(false);
    qApp->installEventFilter(this);

    m_snap.set(getProjectSnap(project));
    m_snap.ch.onReceive(this, [project = project](const Snap& s) {
        saveProjectSnap(project, s);
    });

    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());
    IF_ASSERT_FAILED(au3Project) {
        return;
    }

    au3::setViewStateRestorerNotification(*au3Project, m_rolledBack);

    m_tracksVerticalOffset.set(getProjectZoomState(au3Project).tracksVerticalOffset);
    m_tracksVerticalOffset.ch.onReceive(this, [au3Project](const int y) {
        ZoomState zoomState = getProjectZoomState(au3Project);
        zoomState.tracksVerticalOffset = y;
        saveProjectZoomState(au3Project, zoomState);
    });

    globalContext()->currentTrackeditProjectChanged().onNotify(this, [this](){
        auto prj = globalContext()->currentTrackeditProject();
        if (!prj) {
            return;
        }

        prj->trackRemoved().onReceive(this, [this](const trackedit::Track& track) {
            recomputeTotalTrackHeight();
            m_verticalRulerWidth.set(calculateVerticalRulerWidth());
            m_tracks.erase(track.id);
        });

        updateItemsBoundaries(false);
        prj->trackChanged().onReceive(this, [this](const trackedit::Track&) {
            updateItemsBoundaries(false);
        });

        prj->trackAdded().onReceive(this, [this](const trackedit::Track& track) {
            collapseInsertedMultiView(track.id);
            recomputeTotalTrackHeight();
            updateItemsBoundaries(false);
        });

        prj->trackInserted().onReceive(this, [this](const trackedit::Track& track, int) {
            collapseInsertedMultiView(track.id);
            recomputeTotalTrackHeight();
            updateItemsBoundaries(false);
        });

        for (const auto& trackId : prj->trackIdList()) {
            const auto prj = globalContext()->currentProject();
            const auto viewType = getTrackViewType(prj, trackId);
            const auto hasSpectrogram = viewType == agu::trackedit::TrackViewType::Spectrogram
                                        || viewType == agu::trackedit::TrackViewType::WaveformAndSpectrogram;
            frequencySelectionController()->setShowsSpectrogram(trackId, hasSpectrogram);
        }

        // Tracks already present on open don't trigger trackAdded / trackInserted
        // So the total height must be computed here
        recomputeTotalTrackHeight();
    });

    projectHistory()->historyChanged().onReceive(this, [this](auto) {
        updateItemsBoundaries(false);
    });
}

muse::ValCh<int> ProjectViewState::totalTrackHeight() const
{
    return m_totalTracksHeight;
}

muse::ValCh<int> ProjectViewState::tracksVerticalOffset() const
{
    return m_tracksVerticalOffset;
}

void ProjectViewState::changeTracksVerticalOffset(int deltaY)
{
    if (deltaY != m_tracksVerticalOffset.val) {
        m_scrollAnchorAnimation.stop();
    }
    m_tracksVerticalOffset.set(deltaY);
}

double ProjectViewState::mousePositionY() const
{
    return m_mouseYPosition.val;
}

void ProjectViewState::setMousePositionY(double y)
{
    m_mouseYPosition.set(y);
}

muse::ValCh<bool> ProjectViewState::tracksVerticalScrollLocked() const
{
    return m_tracksVerticalScrollLocked;
}

void ProjectViewState::setTracksVerticalScrollLocked(bool lock)
{
    m_tracksVerticalScrollLocked.set(lock);
}

int ProjectViewState::trackVerticalPosition(const trackedit::TrackId& trackId) const
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        return -1;
    }

    trackedit::TrackIdList tracks = prj->trackIdList();

    int tracksVerticalOffset = this->tracksVerticalOffset().val;
    int trackTop = -tracksVerticalOffset;
    int trackBottom = trackTop;

    for (trackedit::TrackId id : tracks) {
        trackTop = trackBottom;
        trackBottom = trackTop + trackHeight(id).val;

        if (trackId == id) {
            return trackTop;
        }
    }

    return -1;
}

ProjectViewState::TrackData& ProjectViewState::makeTrackData(const trackedit::TrackId& trackId) const
{
    auto existing = m_tracks.find(trackId);
    if (existing != m_tracks.end()) {
        return existing->second;
    }

    const project::IAGuProjectPtr prj = globalContext()->currentProject();

    const std::pair<float, float> defaultBounds = getVerticalDisplayBounds(prj, trackId);
    const int defaultRulerType = getTrackRulerType(prj, trackId);
    const trackedit::TrackViewType defaultViewType = getTrackViewType(prj, trackId);

    TrackData d;
    d.collapsed.val = false;
    d.interactiveResize.val = false;
    d.channelHeightRatio.val = 1.0;
    d.multiViewHeightRatio.val = 0.5;
    d.verticalDisplayBounds.val = defaultBounds;
    d.isHalfWave.val = (defaultBounds.first == 0.0f);
    d.rulerType.val = defaultRulerType;
    d.viewType.val = defaultViewType;

    d.viewType.ch.onReceive(this, [this, trackId](auto) {
        const auto prj = globalContext()->currentTrackeditProject();
        if (prj) {
            const auto track = prj->track(trackId);
            if (track) {
                prj->notifyAboutTrackChanged(*track);
            }
        }
    });

    trackedit::ITrackeditProjectPtr trackeditPrj = globalContext()->currentTrackeditProject();
    if (trackeditPrj) {
        std::optional<trackedit::Track> track = trackeditPrj->track(trackId);
        if (track) {
            d.isLabel = track->type == trackedit::TrackType::Label;
            d.channelHeightRatio.val = track->type == trackedit::TrackType::Stereo ? 0.5 : 1.0;
            int defaultHeight = track->type == trackedit::TrackType::Label ? TRACK_LABEL_DEFAULT_HEIGHT : TRACK_DEFAULT_HEIGHT;

            int height = defaultHeight;

            if (prj) {
                auto track = agu::au3::DomAccessor::findTrack(
                    *reinterpret_cast<agu::au3::Au3Project*>(prj->au3ProjectPtr()), agu::au3::Au3TrackId(trackId));
                if (track) {
                    const auto& attachment = au3::TrackHeightAttachment::Get(track);
                    d.multiViewHeightRatio.val = attachment.GetMultiViewHeightRatio();
                    int savedHeight = attachment.GetHeight();
                    savedHeight = savedHeight <= 0 ? defaultHeight : savedHeight;
                    const int minHeight = d.isLabel ? LABEL_TRACK_MIN_HEIGHT
                                          : defaultViewType == trackedit::TrackViewType::WaveformAndSpectrogram
                                          ? TRACK_MULTIVIEW_MIN_HEIGHT : TRACK_MIN_HEIGHT;
                    height = std::max(savedHeight, minHeight);
                }
            }
            d.collapsed.val = height < (d.isLabel ? LABEL_TRACK_COLLAPSE_HEIGHT : TRACK_COLLAPSE_HEIGHT);
            d.height.val = height;
        }
    }

    m_verticalRulerWidth.set(calculateVerticalRulerWidth());

    return m_tracks.insert({ trackId, d }).first->second;
}

void ProjectViewState::recomputeTotalTrackHeight()
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        m_totalTracksHeight.set(0);
        return;
    }

    int total = 0;
    for (const trackedit::TrackId& id : prj->trackIdList()) {
        total += trackHeight(id).val;
    }
    m_totalTracksHeight.set(total);
}

void ProjectViewState::collapseInsertedMultiView(const trackedit::TrackId& trackId)
{
    if (trackViewType(trackId).val != trackedit::TrackViewType::WaveformAndSpectrogram) {
        return;
    }
    const auto prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        return;
    }
    // Duplication and stereo splitting copy view attachments. Keep the
    // existing expanded track, while retaining the copy's expanded dimensions.
    for (const auto otherId : prj->trackIdList()) {
        if (otherId != trackId && trackViewType(otherId).val == trackedit::TrackViewType::WaveformAndSpectrogram) {
            doSetTrackViewType(trackId, trackedit::TrackViewType::Waveform);
            return;
        }
    }
}

bool ProjectViewState::doSetTrackViewType(const trackedit::TrackId& trackId, trackedit::TrackViewType viewType)
{
    const auto it = m_tracks.find(trackId);
    if (it == m_tracks.end()) {
        return false;
    }

    const auto project = globalContext()->currentProject();
    IF_ASSERT_FAILED(project) {
        return false;
    }

    agu::au3::Au3Project* au3Project = reinterpret_cast<agu::au3::Au3Project*>(project->au3ProjectPtr());

    agu::au3::Au3WaveTrack* waveTrack = agu::au3::DomAccessor::findWaveTrack(*au3Project, agu::au3::Au3TrackId(trackId));
    if (waveTrack == nullptr) {
        return false;
    }

    auto& d = it->second;
    if (d.viewType.val == viewType) {
        return false;
    }
    const bool wasExpanded = d.viewType.val == trackedit::TrackViewType::WaveformAndSpectrogram;
    const bool expanded = viewType == trackedit::TrackViewType::WaveformAndSpectrogram;
    int height = d.height.val;
    auto& heights = au3::TrackHeightAttachment::Get(waveTrack);
    if (wasExpanded != expanded) {
        if (expanded) {
            heights.SetSingleViewHeight(height);
            height = heights.GetMultiViewHeight() > 0 ? heights.GetMultiViewHeight() : height + TRACK_MULTIVIEW_EXTRA_HEIGHT;
        } else {
            heights.SetMultiViewHeight(height);
            height = heights.GetSingleViewHeight() > 0 ? heights.GetSingleViewHeight() : TRACK_DEFAULT_HEIGHT;
        }
    }

    auto& cache = agu::au3::TrackViewTypeAttachment::Get(waveTrack);
    cache.SetTrackViewType(viewType);

    const auto hasSpectrogram = viewType == agu::trackedit::TrackViewType::Spectrogram
                                || viewType == agu::trackedit::TrackViewType::WaveformAndSpectrogram;
    frequencySelectionController()->setShowsSpectrogram(trackId, hasSpectrogram);

    d.viewType.set(viewType);
    if (wasExpanded != expanded) {
        setTrackHeight(trackId, height);
    }
    return true;
}

muse::ValCh<int> ProjectViewState::trackHeight(const trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.height;
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj || !prj->track(trackId)) {
        return {};
    }

    const ProjectViewState::TrackData& d = makeTrackData(trackId);
    return d.height;
}

muse::ValCh<bool> ProjectViewState::isTrackCollapsed(const trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.collapsed;
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj || !prj->track(trackId)) {
        return {};
    }

    const ProjectViewState::TrackData& d = makeTrackData(trackId);
    return d.collapsed;
}

muse::ValCh<bool> ProjectViewState::interactiveResize(const trackedit::TrackId& trackId) const
{
    return makeTrackData(trackId).interactiveResize;
}

void ProjectViewState::setInteractiveResize(const trackedit::TrackId& trackId, bool active)
{
    makeTrackData(trackId).interactiveResize.set(active);
}

muse::ValCh<double> ProjectViewState::channelHeightRatio(const trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.channelHeightRatio;
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj || !prj->track(trackId)) {
        return {};
    }

    const ProjectViewState::TrackData& d = makeTrackData(trackId);
    return d.channelHeightRatio;
}

muse::ValCh<double> ProjectViewState::multiViewHeightRatio(const trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.multiViewHeightRatio;
    }
    return makeTrackData(trackId).multiViewHeightRatio;
}

void ProjectViewState::setMultiViewHeightRatio(const trackedit::TrackId& trackId, double ratio)
{
    if (!std::isfinite(ratio)) {
        return;
    }
    const double bounded = std::clamp(ratio, 0.1, 0.9);
    auto& d = makeTrackData(trackId);
    d.multiViewHeightRatio.set(bounded);
    const auto prj = globalContext()->currentProject();
    if (prj && prj->au3ProjectPtr()) {
        auto track = au3::DomAccessor::findTrack(*reinterpret_cast<au3::Au3Project*>(prj->au3ProjectPtr()), au3::Au3TrackId(trackId));
        if (track) {
            au3::TrackHeightAttachment::Get(track).SetMultiViewHeightRatio(bounded);
        }
    }
}

void ProjectViewState::changeTrackHeight(const trackedit::TrackId& trackId, int delta)
{
    setTrackHeight(trackId, makeTrackData(trackId).height.val + delta);
}

void ProjectViewState::setTrackHeight(const trackedit::TrackId& trackId, int height)
{
    TrackData* d = nullptr;
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        d = &it->second;
    } else {
        d = &makeTrackData(trackId);
    }

    const bool expanded = d->viewType.val == trackedit::TrackViewType::WaveformAndSpectrogram;
    const int minHeight = d->isLabel ? LABEL_TRACK_MIN_HEIGHT : expanded ? TRACK_MULTIVIEW_MIN_HEIGHT : TRACK_MIN_HEIGHT;
    int newHeight = std::max(height, minHeight);
    d->height.set(newHeight);
    d->collapsed.set(newHeight < (d->isLabel ? LABEL_TRACK_COLLAPSE_HEIGHT : TRACK_COLLAPSE_HEIGHT));

    recomputeTotalTrackHeight();

    const project::IAGuProjectPtr prj = globalContext()->currentProject();
    if (prj) {
        au3::Au3Project* au3Project = reinterpret_cast<au3::Au3Project*>(prj->au3ProjectPtr());
        if (au3Project) {
            auto track = agu::au3::DomAccessor::findTrack(*au3Project, agu::au3::Au3TrackId(trackId));
            if (track) {
                auto& heights = au3::TrackHeightAttachment::Get(track);
                heights.SetHeight(newHeight);
                if (expanded) {
                    heights.SetMultiViewHeight(newHeight);
                } else {
                    heights.SetSingleViewHeight(newHeight);
                }
            }
        }
    }
}

void ProjectViewState::resetTrackHeight(const trackedit::TrackId& trackId)
{
    const auto& data = makeTrackData(trackId);
    const int defaultHeight = data.isLabel ? std::max(TRACK_LABEL_DEFAULT_HEIGHT, data.labelRowCount * 24 + 16)
                              : data.viewType.val == trackedit::TrackViewType::WaveformAndSpectrogram
                              ? TRACK_DEFAULT_HEIGHT + TRACK_MULTIVIEW_EXTRA_HEIGHT : TRACK_DEFAULT_HEIGHT;
    setTrackHeight(trackId, defaultHeight);
}

void ProjectViewState::setLabelRowCount(const trackedit::TrackId& trackId, int rows)
{
    makeTrackData(trackId).labelRowCount = std::max(0, rows);
}

agu::trackedit::TrackId ProjectViewState::trackAtPosition(double y) const
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        return trackedit::INVALID_TRACK;
    }

    trackedit::TrackIdList tracks = prj->trackIdList();

    int tracksVerticalOffset = this->tracksVerticalOffset().val;
    int trackTop = -tracksVerticalOffset;
    int trackBottom = trackTop;

    for (trackedit::TrackId id : tracks) {
        trackTop = trackBottom;
        trackBottom = trackTop + trackHeight(id).val;

        if (muse::RealIsEqualOrMore(y, trackTop) && muse::RealIsEqualOrLess(y, trackBottom)) {
            return id;
        }
    }

    return trackedit::INVALID_TRACK;
}

agu::trackedit::TrackIdList ProjectViewState::tracksInRange(double y1, double y2) const
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        return {};
    }

    if (y1 > y2) {
        std::swap(y1, y2);
    }

    // Tracks are sorted by index, i.e., top to bottom.
    trackedit::TrackIdList tracks = prj->trackIdList();

    int i = 0;
    const auto numTracks = static_cast<int>(tracks.size());
    while (i < numTracks) {
        const trackedit::TrackId& trackId = tracks[i];
        const auto trackTop = trackVerticalPosition(trackId);
        const auto trackBottom = trackTop + trackHeight(trackId).val;
        if (y1 < trackBottom) {
            break;
        }
        ++i;
    }

    auto j = i;
    while (j < numTracks) {
        const trackedit::TrackId& trackId = tracks[j];
        const auto trackTop = trackVerticalPosition(trackId);
        if (y2 <= trackTop) {
            break;
        }
        ++j;
    }

    return trackedit::TrackIdList(tracks.begin() + i, tracks.begin() + j);
}

bool ProjectViewState::isBelowAllTracks(double y) const
{
    const std::optional<int> bottomY = tracksBottomY();
    return bottomY.has_value() && muse::RealIsEqualOrMore(y, *bottomY);
}

std::optional<int> ProjectViewState::tracksBottomY() const
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        return std::nullopt;
    }

    const trackedit::TrackIdList tracks = prj->trackIdList();
    if (tracks.empty()) {
        return std::nullopt;
    }

    int bottomY = -tracksVerticalOffset().val;
    for (const trackedit::TrackId& id : tracks) {
        bottomY += trackHeight(id).val;
    }

    return bottomY;
}

void ProjectViewState::setChannelHeightRatio(const trackedit::TrackId& trackId, double ratio)
{
    TrackData* d = nullptr;
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        d = &it->second;
    } else {
        d = &makeTrackData(trackId);
    }

    d->channelHeightRatio.set(ratio);
}

bool ProjectViewState::isSnapEnabled() const
{
    return m_snap.val.enabled;
}

void ProjectViewState::setIsSnapEnabled(bool enabled)
{
    Snap s = this->snap().val;
    s.enabled = enabled;
    setSnap(s);
}

SnapType ProjectViewState::snapType() const
{
    return m_snap.val.type;
}

void ProjectViewState::setSnapType(SnapType type)
{
    Snap s = this->snap().val;
    s.type = type;
    setSnap(s);
}

bool ProjectViewState::isSnapTripletsEnabled() const
{
    return m_snap.val.isSnapTriplets;
}

void ProjectViewState::setIsSnapTripletsEnabled(bool enabled)
{
    Snap s = this->snap().val;
    s.isSnapTriplets = enabled;
    setSnap(s);
}

void ProjectViewState::setSnap(const Snap& s)
{
    m_snap.set(s);
}

Snap ProjectViewState::getSnap() const
{
    return m_snap.val;
}

muse::ValCh<Snap> ProjectViewState::snap() const
{
    return m_snap;
}

void ProjectViewState::setClipGainAutomationEnabled(bool enabled)
{
    if (m_clipGainAutomationEnabled.val == enabled) {
        return;
    }

    m_clipGainAutomationEnabled.set(enabled);
}

muse::ValCh<bool> ProjectViewState::clipGainAutomationEnabled() const
{
    return m_clipGainAutomationEnabled;
}

void ProjectViewState::setSplitToolEnabled(const bool enabled)
{
    if (m_splitToolEnabled.val == enabled) {
        return;
    }

    m_splitToolEnabled.set(enabled);
}

muse::ValCh<bool> ProjectViewState::splitToolEnabled() const
{
    return m_splitToolEnabled;
}

muse::ValCh<std::pair<float, float> > ProjectViewState::verticalDisplayBounds(const trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.verticalDisplayBounds;
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj || !prj->track(trackId)) {
        return {};
    }

    const ProjectViewState::TrackData& d = makeTrackData(trackId);
    return d.verticalDisplayBounds;
}

float ProjectViewState::maxVerticalZoomLevel(const trackedit::TrackId& trackId) const
{
    constexpr int MIN_DISTANCE_FROM_RANGE = 6;
    constexpr int DB_PER_STEP = 6;
    constexpr float MIN_VERTICAL_RANGE = 1.0f / (1 << 9);

    const auto project = globalContext()->currentProject();
    if (!project) {
        return MIN_VERTICAL_RANGE;
    }

    const agu::au3::Au3Project* au3Project = reinterpret_cast<const agu::au3::Au3Project*>(project->au3ProjectPtr());

    const agu::au3::Au3WaveTrack* waveTrack = agu::au3::DomAccessor::findWaveTrack(*au3Project, ::TrackId(trackId));
    if (waveTrack == nullptr) {
        return MIN_VERTICAL_RANGE;
    }

    trackedit::Track track = agu::au3::DomConverter::track(waveTrack);

    const auto rulerType = static_cast<trackedit::TrackRulerType>(::getTrackRulerType(project, trackId));

    if (rulerType == trackedit::TrackRulerType::DbLog) {
        const int dBRange = static_cast<int>(playback::PlaybackMeterDbRange::toDouble(playbackConfiguration()->playbackMeterDbRange()));
        const int steps = (-dBRange - MIN_DISTANCE_FROM_RANGE) / DB_PER_STEP;
        return std::max(MIN_VERTICAL_RANGE, 1.0f / (1 << steps));
    }

    return MIN_VERTICAL_RANGE;
}

void ProjectViewState::zoomInVertically(const trackedit::TrackId& trackId)
{
    const auto project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    const auto& [verticalMin, verticalMax] = getVerticalDisplayBounds(project, trackId);
    const float maxZoom = maxVerticalZoomLevel(trackId);

    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        const std::pair<float, float> newBounds = {
            std::min(verticalMin / 2.0f, -maxZoom),
            std::max(verticalMax / 2.0f, maxZoom)
        };
        setVerticalDisplayBounds(project, trackId, newBounds);
        it->second.verticalDisplayBounds.set(newBounds);
        m_verticalRulerWidth.set(calculateVerticalRulerWidth());
    }
}

void ProjectViewState::zoomOutVertically(const trackedit::TrackId& trackId)
{
    constexpr float MAX_VERTICAL_RANGE = 2.0f;

    const auto project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    const auto& [verticalMin, verticalMax] = getVerticalDisplayBounds(project, trackId);

    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        const std::pair<float, float> newBounds = {
            std::max(verticalMin * 2.0f, -MAX_VERTICAL_RANGE),
            std::min(verticalMax * 2.0f, MAX_VERTICAL_RANGE)
        };
        setVerticalDisplayBounds(project, trackId, newBounds);
        it->second.verticalDisplayBounds.set(newBounds);
        m_verticalRulerWidth.set(calculateVerticalRulerWidth());
    }
}

void ProjectViewState::resetVerticalZoom(const trackedit::TrackId& trackId)
{
    const float DEFAULT_VERTICAL_MIN = -1.0f;
    const float DEFAULT_VERTICAL_MAX = 1.0f;

    const auto project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    const auto& [verticalMin, _] = getVerticalDisplayBounds(project, trackId);
    const bool isHalfWave = (verticalMin == 0.0f);

    const std::pair<float, float> defaultBounds =  { isHalfWave ? 0.0f : DEFAULT_VERTICAL_MIN, DEFAULT_VERTICAL_MAX };

    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        setVerticalDisplayBounds(project, trackId, defaultBounds);
        it->second.verticalDisplayBounds.set(defaultBounds);
        m_verticalRulerWidth.set(calculateVerticalRulerWidth());
    }
}

bool ProjectViewState::isDefaultVerticalZoom(const trackedit::TrackId& trackId) const
{
    constexpr float DEFAULT_VERTICAL_MAX = 1.0f;

    const auto project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    const auto& [_, verticalMax] = getVerticalDisplayBounds(project, trackId);
    return muse::RealIsEqual(verticalMax, DEFAULT_VERTICAL_MAX);
}

bool ProjectViewState::isMaxVerticalZoom(const trackedit::TrackId& trackId) const
{
    const auto project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    const auto& [_, verticalMax] = getVerticalDisplayBounds(project, trackId);
    const float maxZoom = maxVerticalZoomLevel(trackId);
    return muse::RealIsEqualOrLess(std::abs(verticalMax), maxZoom);
}

bool ProjectViewState::isMinVerticalZoom(const trackedit::TrackId& trackId) const
{
    constexpr float MAX_VERTICAL_RANGE = 2.0f;

    const auto project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    const auto& [_, verticalMax] = getVerticalDisplayBounds(project, trackId);
    return muse::RealIsEqualOrMore(std::abs(verticalMax), MAX_VERTICAL_RANGE);
}

muse::ValCh<bool> ProjectViewState::isHalfWave(const trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.isHalfWave;
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj || !prj->track(trackId)) {
        return {};
    }

    const ProjectViewState::TrackData& d = makeTrackData(trackId);
    return d.isHalfWave;
}

void ProjectViewState::toggleHalfWave(const trackedit::TrackId& trackId)
{
    const auto project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    const auto& [verticalMin, verticalMax] = getVerticalDisplayBounds(project, trackId);

    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        bool isHalfWave = (verticalMin == 0.0f);
        setVerticalDisplayBounds(project, trackId,
                                 { isHalfWave ? -verticalMax : 0.0f, verticalMax });
        it->second.verticalDisplayBounds.set(
            { isHalfWave ? -verticalMax : 0.0f, verticalMax });
        it->second.isHalfWave.set(!isHalfWave);
        m_verticalRulerWidth.set(calculateVerticalRulerWidth());
    }
}

muse::ValCh<agu::trackedit::TrackViewType> ProjectViewState::trackViewType(const agu::trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.viewType;
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj || !prj->track(trackId)) {
        return {};
    }

    const ProjectViewState::TrackData& d = makeTrackData(trackId);
    return d.viewType;
}

void ProjectViewState::setTrackViewType(const trackedit::TrackId& trackId, trackedit::TrackViewType viewType)
{
    if (trackViewType(trackId).val == viewType) {
        return;
    }

    const auto project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    const bool changed = doSetTrackViewType(trackId, viewType);
    if (changed && viewType == trackedit::TrackViewType::WaveformAndSpectrogram) {
        const auto prj = globalContext()->currentTrackeditProject();
        if (prj) {
            int heightShiftAbove = 0;
            const auto trackIds = prj->trackIdList();
            const auto expandedPosition = std::find(trackIds.begin(), trackIds.end(), trackId);
            for (auto it = trackIds.begin(); it != trackIds.end(); ++it) {
                const auto otherId = *it;
                if (otherId != trackId && trackViewType(otherId).val == trackedit::TrackViewType::WaveformAndSpectrogram) {
                    const int oldHeight = trackHeight(otherId).val;
                    doSetTrackViewType(otherId, trackedit::TrackViewType::Waveform);
                    if (it < expandedPosition) {
                        heightShiftAbove += trackHeight(otherId).val - oldHeight;
                    }
                }
            }
            if (heightShiftAbove != 0 && m_tracksVerticalOffset.val > 0) {
                const int initialOffset = m_tracksVerticalOffset.val;
                const int targetOffset = std::max(0, initialOffset + heightShiftAbove);
                m_scrollAnchorAnimation.stop();
                m_scrollAnchorAnimation.setStartValue(initialOffset);
                m_scrollAnchorAnimation.setEndValue(targetOffset);
                m_scrollAnchorAnimation.start();
            }
        }
    }

    if (changed) {
        projectHistory()->modifyState();
        projectHistory()->markUnsaved();
    }
}

void ProjectViewState::toggleGlobalSpectrogramView()
{
    const auto prj = globalContext()->currentProject();
    IF_ASSERT_FAILED(prj) {
        return;
    }
    auto changed = false;
    using namespace trackedit;

    if (!m_globalSpectrogramToggleIsOn) {
        // Enable spectrogram view: convert ALL waveform and multi-view tracks to spectrogram
        for (auto& [trackId, trackData] : m_tracks) {
            if (trackData.viewType.val == TrackViewType::Waveform || trackData.viewType.val == TrackViewType::WaveformAndSpectrogram) {
                changed = true;
                doSetTrackViewType(trackId, TrackViewType::Spectrogram);
            }
        }
    } else {
        // Disable spectrogram view: convert ALL spectrogram and multi-view tracks back to waveform
        for (auto& [trackId, trackData] : m_tracks) {
            if (trackData.viewType.val == TrackViewType::Spectrogram || trackData.viewType.val == TrackViewType::WaveformAndSpectrogram) {
                changed = true;
                doSetTrackViewType(trackId, TrackViewType::Waveform);
            }
        }
    }

    m_globalSpectrogramToggleIsOn = !m_globalSpectrogramToggleIsOn;
    m_globalSpectrogramToggleIsOnChanged.notify();

    if (changed) {
        projectHistory()->modifyState();
        projectHistory()->markUnsaved();
    }
}

bool ProjectViewState::globalSpectrogramToggleIsOn() const
{
    return m_globalSpectrogramToggleIsOn;
}

muse::async::Notification ProjectViewState::globalSpectrogramToggleIsOnChanged() const
{
    return m_globalSpectrogramToggleIsOnChanged;
}

muse::ValCh<int> ProjectViewState::trackRulerType(const trackedit::TrackId& trackId) const
{
    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        return it->second.rulerType;
    }

    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj || !prj->track(trackId)) {
        return {};
    }

    const ProjectViewState::TrackData& d = makeTrackData(trackId);
    return d.rulerType;
}

void ProjectViewState::setTrackRulerType(const trackedit::TrackId& trackId, int rulerType)
{
    const auto project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    auto it = m_tracks.find(trackId);
    if (it != m_tracks.end()) {
        it->second.rulerType.set(rulerType);
        ::setTrackRulerType(project, trackId, rulerType);
        m_verticalRulerWidth.set(calculateVerticalRulerWidth());
    }
}

muse::ValCh<int> ProjectViewState::verticalRulerWidth() const
{
    return m_verticalRulerWidth;
}

void ProjectViewState::setItemEditStartTimeOffset(double val)
{
    if (m_itemEditStartTimeOffset == val) {
        return;
    }

    m_itemEditStartTimeOffset = val;
}

double ProjectViewState::itemEditStartTimeOffset() const
{
    return m_itemEditStartTimeOffset;
}

void ProjectViewState::setItemEditEndTimeOffset(double val)
{
    if (m_itemEditEndTimeOffset == val) {
        return;
    }

    m_itemEditEndTimeOffset = val;
}

double ProjectViewState::itemEditEndTimeOffset() const
{
    return m_itemEditEndTimeOffset;
}

void ProjectViewState::setMoveInitiated(bool val)
{
    if (m_moveInitiated == val) {
        return;
    }

    m_moveInitiated = val;
}

bool ProjectViewState::moveInitiated() const
{
    return m_moveInitiated;
}

void ProjectViewState::setMovePreviewEndTime(double time)
{
    m_movePreviewEndTime = time;
}

double ProjectViewState::movePreviewEndTime() const
{
    return m_movePreviewEndTime;
}

void ProjectViewState::setLastEditedClip(const trackedit::ClipKey& clipKey)
{
    if (m_lastEditedClip == clipKey) {
        return;
    }

    m_lastEditedClip = clipKey;
}

agu::trackedit::ClipKey ProjectViewState::lastEditedClip() const
{
    return m_lastEditedClip;
}

void ProjectViewState::setItemsBoundaries(const std::set<muse::secs_t>& boundaries)
{
    m_itemsBoundaries = boundaries;
}

std::set<muse::secs_t> ProjectViewState::itemsBoundaries() const
{
    return m_itemsBoundaries;
}

void ProjectViewState::setEditedItem(const trackedit::TrackItemKey& key)
{
    m_editedItemKey = key;
}

void ProjectViewState::updateItemsBoundaries(bool excludeCurrentSelection, const trackedit::TrackItemKey& itemKeyToOmit)
{
    auto prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        return;
    }

    std::set<muse::secs_t> boundaries;
    const auto& selectedClips = selectionController()->selectedClips();
    const auto& selectedLabels = selectionController()->selectedLabels();

    // An item being dragged must never snap to its own edges. The drag may rebuild the
    // boundary set mid-flight (e.g. label moves emit trackChanged every frame, which calls
    // updateItemsBoundaries(false)), so relying on the one-shot itemKeyToOmit argument is not
    // enough; m_editedItemKey persists the exclusion for the whole edit.
    const auto isEditedItem = [this, &itemKeyToOmit](const trackedit::TrackItemKey& key) {
        if (itemKeyToOmit.isValid() && key == itemKeyToOmit) {
            return true;
        }
        return m_editedItemKey.isValid() && key == m_editedItemKey;
    };

    for (const auto& trackId : prj->trackIdList()) {
        // Add clips boundaries
        for (const auto& clip : prj->clipList(trackId)) {
            if ((excludeCurrentSelection || m_moveInitiated) && muse::contains(selectedClips, clip.key)) {
                continue;
            }

            if (isEditedItem(clip.key)) {
                continue;
            }

            boundaries.insert(clip.startTime);
            boundaries.insert(clip.endTime);
        }

        // Add labels boundaries
        for (const auto& label : prj->labelList(trackId)) {
            if ((excludeCurrentSelection || m_moveInitiated) && muse::contains(selectedLabels, label.key)) {
                continue;
            }

            if (isEditedItem(label.key)) {
                continue;
            }

            boundaries.insert(label.startTime);
            boundaries.insert(label.endTime);
        }
    }

    setItemsBoundaries(boundaries);
}

void ProjectViewState::setZoomState(const ZoomState& state)
{
    agu::au3::Au3Project* project = reinterpret_cast<agu::au3::Au3Project*>(globalContext()->currentProject()->au3ProjectPtr());
    saveProjectZoomState(project, state);
}

ZoomState ProjectViewState::zoomState() const
{
    agu::au3::Au3Project* project = reinterpret_cast<agu::au3::Au3Project*>(globalContext()->currentProject()->au3ProjectPtr());
    return getProjectZoomState(project);
}

muse::async::Notification ProjectViewState::rolledBack() const
{
    return m_rolledBack;
}

muse::ValCh<bool> ProjectViewState::altPressed() const
{
    return m_altPressed;
}

muse::ValCh<bool> ProjectViewState::ctrlPressed() const
{
    return m_ctrlPressed;
}

muse::ValCh<bool> ProjectViewState::keyboardMoveActive() const
{
    return m_keyboardMoveActive;
}

void ProjectViewState::setKeyboardMoveActive(bool active)
{
    if (m_keyboardMoveActive.val != active) {
        m_keyboardMoveActive.set(active);
    }
}

muse::async::Notification ProjectViewState::modifiersReleased() const
{
    return m_modifiersReleased;
}

int ProjectViewState::trackDefaultHeight() const
{
    return TRACK_DEFAULT_HEIGHT;
}

int ProjectViewState::calculateVerticalRulerWidth() const
{
    constexpr int MIN_RULER_WIDTH = 56;

    const auto project = globalContext()->currentProject();
    if (!project) {
        return MIN_RULER_WIDTH;
    }

    float smallestBoundValue = 1;
    for (const auto& [trackId, trackData] : m_tracks) {
        const auto rulerType = static_cast<agu::trackedit::TrackRulerType>(::getTrackRulerType(project, trackId));
        if (rulerType != agu::trackedit::TrackRulerType::Linear) {
            continue;
        }

        const auto& [minBound, maxBound] = verticalDisplayBounds(trackId).val;

        // Find the smallest vertical zoom among linear rulers to ajust the width globally.
        smallestBoundValue = std::min(smallestBoundValue, ((maxBound - minBound) / 2.0f));
    }
    smallestBoundValue *= 0.1f;

    const int nDecimals = numDecimals(smallestBoundValue);

    // Adjust the width according to the number of decimal places.
    return MIN_RULER_WIDTH + (nDecimals * 8);
}

bool ProjectViewState::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        if (static_cast<QKeyEvent*>(event)->key() == 0 || static_cast<QKeyEvent*>(event)->key() == Qt::Key_unknown) {
            return QObject::eventFilter(watched, event);
        }

        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Alt
            && static_cast<QKeyEvent*>(event)->modifiers() == Qt::AltModifier) {
            if (!m_altPressed.val) {
                m_altPressed.set(true);
            }
        } else if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Control
                   && static_cast<QKeyEvent*>(event)->modifiers() == Qt::ControlModifier) {
            if (!m_ctrlPressed.val) {
                m_ctrlPressed.set(true);
            }
        } else {
            // We only want to process single ALT and CTRL key presses
            if (m_altPressed.val) {
                m_altPressed.set(false);
            }

            if (m_ctrlPressed.val) {
                m_ctrlPressed.set(false);
            }
        }
    } else if (event->type() == QEvent::KeyRelease) {
        if (static_cast<QKeyEvent*>(event)->key() == 0 || static_cast<QKeyEvent*>(event)->key() == Qt::Key_unknown) {
            return QObject::eventFilter(watched, event);
        }

        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Alt || (static_cast<QKeyEvent*>(event)->modifiers() & Qt::AltModifier)) {
            m_altPressed.set(false);
        }

        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Control
            || (static_cast<QKeyEvent*>(event)->modifiers() & Qt::ControlModifier)) {
            m_ctrlPressed.set(false);
        }

        const QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (!keyEvent->isAutoRepeat()) {
            switch (keyEvent->key()) {
            case Qt::Key_Control:
            case Qt::Key_Meta:
            case Qt::Key_Alt:
            case Qt::Key_Shift:
                m_modifiersReleased.notify();
                break;
            default:
                break;
            }
        }
    } else if (event->type() == QEvent::ApplicationStateChange) {
        if (qApp->applicationState() != Qt::ApplicationState::ApplicationActive) {
            if (m_altPressed.val) {
                m_altPressed.set(false);
            }

            if (m_ctrlPressed.val) {
                m_ctrlPressed.set(false);
            }

            m_modifiersReleased.notify();
        }
    }

    return QObject::eventFilter(watched, event);
}
