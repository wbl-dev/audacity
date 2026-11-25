/*
 * Audacity: A Digital Audio Editor
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <QImage>

#include "view/timeline/projectoverview.h"
#include "context/tests/mocks/globalcontextmock.h"
#include "project/tests/mocks/aguprojectmock.h"
#include "trackedit/tests/mocks/trackeditprojectmock.h"
#include "mocks/projectsceneconfigurationmock.h"

using namespace ::testing;

namespace agu::projectscene {
class OverviewPainterMock : public IWavePainter
{
public:
    MOCK_METHOD(void, paint, (QPainter&, const trackedit::ClipKey&, const Params&, std::optional<PlotType>), (override));
};

class ProjectOverviewTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_overview.globalContext.set(m_context);
        m_overview.configuration.set(m_configuration);
        m_overview.wavePainter.set(m_painter);
        m_overview.m_project = m_project;
        m_overview.m_duration = 10.0;
        m_overview.setWidth(1000);
        m_overview.setHeight(24);
        ON_CALL(*m_context, currentProject()).WillByDefault(Return(m_audioProject));
    }

    std::shared_ptr<NiceMock<context::GlobalContextMock> > m_context = std::make_shared<NiceMock<context::GlobalContextMock> >();
    std::shared_ptr<NiceMock<project::AGuProjectMock> > m_audioProject = std::make_shared<NiceMock<project::AGuProjectMock> >();
    std::shared_ptr<NiceMock<trackedit::TrackeditProjectMock> > m_project = std::make_shared<NiceMock<trackedit::TrackeditProjectMock> >();
    std::shared_ptr<NiceMock<ProjectSceneConfigurationMock> > m_configuration = std::make_shared<NiceMock<ProjectSceneConfigurationMock> >();
    std::shared_ptr<StrictMock<OverviewPainterMock> > m_painter = std::make_shared<StrictMock<OverviewPainterMock> >();
    ProjectOverview m_overview;
};

TEST_F(ProjectOverviewTests, ManyStereoTracksRetainDrawableChannelHeights)
{
    trackedit::TrackList tracks;
    for (int i = 0; i < 13; ++i) {
        trackedit::Track track;
        track.id = i;
        track.type = trackedit::TrackType::Stereo;
        tracks.push_back(track);
        trackedit::Clip clip;
        clip.key = { i, 1 };
        clip.startTime = 2.0;
        clip.endTime = 8.0;
        clip.stereo = true;
        muse::async::NotifyList<trackedit::Clip> clips;
        clips.push_back(clip);
        ON_CALL(*m_project, clipList(track.id)).WillByDefault(Return(clips));
        EXPECT_CALL(*m_painter, paint(_, clip.key, _, std::optional<IWavePainter::PlotType>(IWavePainter::PlotType::MinMaxRMS)))
        .WillOnce([](QPainter& painter, const trackedit::ClipKey& key, const IWavePainter::Params& params, auto) {
            EXPECT_GE(params.geometry.height * params.channelHeightRatio, 1.0);
            EXPECT_GE(params.geometry.height * (1.0 - params.channelHeightRatio), 1.0);
            EXPECT_DOUBLE_EQ(params.geometry.width, 600.0);
            EXPECT_DOUBLE_EQ(params.fromTime, 0.0);
            EXPECT_DOUBLE_EQ(params.toTime, 6.0);
            // Simulate the opaque RGB native cache, with an earlier signal at a
            // different position from clips sharing the same overview lane.
            painter.fillRect(QRectF(0, 0, params.geometry.width, params.geometry.height), Qt::black);
            painter.fillRect(QRectF(key.trackId == 0 ? 100 : 200, 0, 2, params.geometry.height), Qt::white);
        });
    }
    ON_CALL(*m_project, trackList()).WillByDefault(Return(tracks));
    QImage image(1000, 24, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    m_overview.paint(&painter);
    EXPECT_EQ(image.pixelColor(300, 1), QColor(Qt::white));
}
}
