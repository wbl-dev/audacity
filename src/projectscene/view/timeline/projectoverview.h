/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <QQuickPaintedItem>
#include <QHash>

#include "context/iglobalcontext.h"
#include "global/async/asyncable.h"
#include "modularity/ioc.h"
#include "projectscene/iprojectsceneconfiguration.h"
#include "trackedit/itrackeditproject.h"
#include "../tracksitemsview/iwavepainter.h"

namespace agu::projectscene {
// A project-time overview. It deliberately has no selection controller: navigating
// the overview must not modify the audio or the user's editing selection.
class ProjectOverview : public QQuickPaintedItem, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged FINAL)

    muse::GlobalInject<IProjectSceneConfiguration> configuration;
    muse::ContextInject<context::IGlobalContext> globalContext { this };
    muse::ContextInject<IWavePainter> wavePainter { this };

public:
    explicit ProjectOverview(QQuickItem* parent = nullptr);
    Q_INVOKABLE void init();
    double duration() const;
    void paint(QPainter* painter) override;

signals:
    void durationChanged();

private:
    friend class ProjectOverviewTests;
    void reload();
    void refresh();
    bool updateTrackColor(const trackedit::Track& track);
    trackedit::ITrackeditProjectPtr m_project;
    std::vector<muse::async::NotifyList<trackedit::Clip> > m_clipLists;
    QHash<trackedit::TrackId, trackedit::ClipColorIndex> m_trackColors;
    double m_duration = 1.0;
};
}
