/*
* Audacity: A Digital Audio Editor
*/
#include "tracklabelslayoutmanager.h"

#include <algorithm>
#include <cmath>

#include <QAbstractListModel>
#include <QModelIndex>
#include <QVariant>

#include "tracklabelslistmodel.h"
#include "tracklabelitem.h"
#include "viewtrackitem.h"

#include "defer.h"

using namespace au::projectscene;
using namespace au::trackedit;

static const double COMPARE_EPS = 0.001;

TrackLabelsLayoutManager::TrackLabelsLayoutManager(QObject* parent)
    : QObject(parent)
{
}

TrackLabelsLayoutManager::~TrackLabelsLayoutManager()
{
}

void TrackLabelsLayoutManager::init()
{
    m_isInited = true;

    relayout();
    relink();
}

au::projectscene::LabelKey TrackLabelsLayoutManager::leftLinkedLabel(const LabelKey& key) const
{
    if (!m_leftLinkedLabels.contains(key)) {
        return LabelKey();
    }

    return m_leftLinkedLabels[key];
}

void TrackLabelsLayoutManager::activateLeftLinkedLabel(const LabelKey& key)
{
    if (!m_leftLinkedLabels.contains(key)) {
        return;
    }

    QList<LabelInfo> labels = collectLabelsInfo();
    const LabelKey& otherKey = m_leftLinkedLabels[key];
    for (const LabelInfo& label : labels) {
        if (label.key == otherKey) {
            label.item->setIsLinkedActive(true);
            break;
        }
    }
}

au::projectscene::LabelKey TrackLabelsLayoutManager::rightLinkedLabel(const LabelKey& key) const
{
    if (!m_rightLinkedLabels.contains(key)) {
        return LabelKey();
    }

    return m_rightLinkedLabels[key];
}

void TrackLabelsLayoutManager::activateRightLinkedLabel(const LabelKey& key)
{
    if (!m_rightLinkedLabels.contains(key)) {
        return;
    }

    QList<LabelInfo> labels = collectLabelsInfo();
    const LabelKey& otherKey = m_rightLinkedLabels[key];
    for (const LabelInfo& label : labels) {
        if (label.key == otherKey) {
            label.item->setIsLinkedActive(true);
            break;
        }
    }
}

void TrackLabelsLayoutManager::deactivateLinkedLabel(const LabelKey& key)
{
    QList<LabelInfo> labels = collectLabelsInfo();
    const LabelKey& otherLeftLinkedLabelKey = m_leftLinkedLabels[key];
    const LabelKey& otherRightLinkedLabelKey = m_rightLinkedLabels[key];
    for (const LabelInfo& label : labels) {
        if (label.key == otherLeftLinkedLabelKey) {
            label.item->setIsLinkedActive(false);
        } else if (label.key == otherRightLinkedLabelKey) {
            label.item->setIsLinkedActive(false);
        }
    }
}

QObject* TrackLabelsLayoutManager::labelsModel() const
{
    return m_labelsModel;
}

void TrackLabelsLayoutManager::setLabelsModel(QObject* model)
{
    TrackLabelsListModel* labelsListModel = qobject_cast<TrackLabelsListModel*>(model);
    if (m_labelsModel == labelsListModel) {
        return;
    }

    unsubscribeFromLabelsChanges();

    m_labelsModel = labelsListModel;
    emit labelsModelChanged();

    subscribeToLabelsChanges();
}

void TrackLabelsLayoutManager::subscribeToLabelsChanges()
{
    if (!m_labelsModel) {
        return;
    }

    unsubscribeFromLabelsChanges();

    QList<LabelInfo> labels = collectLabelsInfo();

    for (const LabelInfo& label : labels) {
        connect(label.item, &TrackLabelItem::startTimeChanged, this, &TrackLabelsLayoutManager::scheduleRelayout);
        connect(label.item, &TrackLabelItem::endTimeChanged, this, &TrackLabelsLayoutManager::scheduleRelayout);
        connect(label.item, &TrackLabelItem::ownerChanged, this, &TrackLabelsLayoutManager::scheduleRelayout);
        connect(label.item, &TrackLabelItem::draggedChanged, this, &TrackLabelsLayoutManager::scheduleRelayout);
        connect(label.item, &TrackLabelItem::isEditingChanged, this, &TrackLabelsLayoutManager::relink);
    }

    connect(m_labelsModel, &QAbstractListModel::rowsInserted, this, &TrackLabelsLayoutManager::subscribeToLabelsChanges);
    connect(m_labelsModel, &QAbstractListModel::rowsRemoved, this, &TrackLabelsLayoutManager::subscribeToLabelsChanges);

    relayout();
}

void TrackLabelsLayoutManager::unsubscribeFromLabelsChanges()
{
    if (!m_labelsModel) {
        return;
    }

    QList<LabelInfo> labels = collectLabelsInfo();
    for (const LabelInfo& label : labels) {
        label.item->disconnect(this);
    }

    disconnect(m_labelsModel, &QAbstractListModel::rowsInserted, this, &TrackLabelsLayoutManager::subscribeToLabelsChanges);
    disconnect(m_labelsModel, &QAbstractListModel::rowsRemoved, this, &TrackLabelsLayoutManager::subscribeToLabelsChanges);
}

TimelineContext* TrackLabelsLayoutManager::timelineContext() const
{
    return m_context;
}

void TrackLabelsLayoutManager::setTimelineContext(TimelineContext* newContext)
{
    if (m_context == newContext) {
        return;
    }

    if (m_context) {
        disconnect(m_context, nullptr, this, nullptr);
    }

    m_context = newContext;

    if (m_context) {
        connect(m_context, &TimelineContext::zoomChanged, this, &TrackLabelsLayoutManager::scheduleRelayout);
    }

    emit timelineContextChanged();
}

QList<TrackLabelsLayoutManager::LabelInfo> TrackLabelsLayoutManager::collectLabelsInfo() const
{
    QList<LabelInfo> labels;

    if (!m_labelsModel) {
        return labels;
    }

    for (int i = 0; i < m_labelsModel->rowCount(QModelIndex()); ++i) {
        QModelIndex index = m_labelsModel->index(i);
        QVariant itemVariant = m_labelsModel->data(index, Qt::UserRole + 1);
        ViewTrackItem* viewItem = qvariant_cast<ViewTrackItem*>(itemVariant);
        TrackLabelItem* labelItem = qobject_cast<TrackLabelItem*>(viewItem);

        if (labelItem) {
            LabelInfo info;
            info.item = labelItem;
            info.key = labelItem->key();

            TrackItemTime time = labelItem->time();
            info.startTime = time.startTime;
            info.endTime = time.endTime;
            info.isPoint = labelItem->isPoint();
            info.owner = labelItem->owner();

            labels << info;
        }
    }

    return labels;
}

void TrackLabelsLayoutManager::scheduleRelayout()
{
    if (m_relayoutScheduled) {
        return;
    }
    m_relayoutScheduled = true;

    QMetaObject::invokeMethod(this, [this]() {
        m_relayoutScheduled = false;
        relayout();
    }, Qt::QueuedConnection);
}

void TrackLabelsLayoutManager::relayout()
{
    if (m_isBusy || !m_isInited) {
        return;
    }

    m_isBusy = true;
    DEFER {
        m_isBusy = false;
    };

    if (!m_labelsModel) {
        return;
    }

    QList<LabelInfo> labels = collectLabelsInfo();
    labels.removeIf([](const LabelInfo& label) { return label.item->dragged(); });

    QMap<QString, QList<LabelInfo*> > byOwner;
    for (LabelInfo& label : labels) {
        byOwner[label.owner].append(&label);
    }

    int nextLevel = 0;
    bool firstOwner = true;
    for (auto ownerIt = byOwner.begin(); ownerIt != byOwner.end(); ++ownerIt) {
        if (!firstOwner) {
            ++nextLevel; // A blank row separates semantic owner groups.
        }
        firstOwner = false;

        QList<LabelInfo*> ranges;
        QList<LabelInfo*> points;
        for (LabelInfo* label : ownerIt.value()) {
            (label->isPoint ? points : ranges).append(label);
        }
        const auto byStart = [](const LabelInfo* a, const LabelInfo* b) {
            return a->startTime < b->startTime;
        };
        std::stable_sort(ranges.begin(), ranges.end(), byStart);
        std::stable_sort(points.begin(), points.end(), byStart);

        QList<double> rowEndTimes;
        for (LabelInfo* range : ranges) {
            int row = 0;
            while (row < rowEndTimes.size() && rowEndTimes[row] > range->startTime) {
                ++row;
            }
            if (row == rowEndTimes.size()) {
                rowEndTimes.append(range->endTime);
            } else {
                rowEndTimes[row] = range->endTime;
            }
            range->item->setLevel(nextLevel + row);
        }
        nextLevel += rowEndTimes.size();

        int pointRows = 0;
        int sameTimeIndex = 0;
        double previousTime = -1.0;
        for (LabelInfo* point : points) {
            sameTimeIndex = point->startTime == previousTime ? sameTimeIndex + 1 : 0;
            previousTime = point->startTime;
            point->item->setLevel(nextLevel + sameTimeIndex);
            pointRows = std::max(pointRows, sameTimeIndex + 1);
        }
        nextLevel += pointRows;
    }
}

void TrackLabelsLayoutManager::relink()
{
    if (m_isBusy || !m_isInited) {
        return;
    }

    m_isBusy = true;
    DEFER {
        m_isBusy = false;
    };

    if (!m_labelsModel) {
        return;
    }

    m_leftLinkedLabels.clear();
    m_rightLinkedLabels.clear();

    QList<LabelInfo> labels = collectLabelsInfo();
    // Do not link to ghosts
    labels.removeIf([](const LabelInfo& label) { return label.item->isDragGhost(); });
    for (const LabelInfo& label : labels) {
        label.item->setIsLeftLinked(false);
        label.item->setIsRightLinked(false);
    }

    // Update linked labels
    // Label is linked if its right edge matches another label's left edge (or vice versa)
    for (int i = 0; i < labels.size(); ++i) {
        const LabelInfo& current = labels[i];
        if (current.isPoint) {
            continue;
        }

        double currentRightEdge = current.endTime;
        double currentLeftEdge = current.startTime;

        for (int j = 0; j < labels.size(); ++j) {
            if (i == j) {
                continue;
            }

            const LabelInfo& other = labels[j];
            if (other.isPoint) {
                continue;
            }

            double otherRightEdge = other.endTime;
            double otherLeftEdge = other.startTime;

            if (std::abs(currentRightEdge - otherLeftEdge) < COMPARE_EPS) {
                m_rightLinkedLabels[current.key] = other.key;
                current.item->setIsRightLinked(true);

                m_leftLinkedLabels[other.key] = current.key;
                other.item->setIsLeftLinked(true);
            }

            if (std::abs(currentLeftEdge - otherRightEdge) < COMPARE_EPS) {
                m_leftLinkedLabels[current.key] = other.key;
                current.item->setIsLeftLinked(true);

                m_rightLinkedLabels[other.key] = current.key;
                other.item->setIsRightLinked(true);
            }
        }
    }
}
