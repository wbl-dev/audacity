/*
* Audacity: A Digital Audio Editor
*/
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "projectscene/view/timeline/timelinecontext.h"

#include "snaptestaccess.h"

#include "context/tests/mocks/globalcontextmock.h"
#include "context/tests/mocks/playbackstatemock.h"
#include "project/tests/mocks/audacityprojectmock.h"
#include "trackedit/tests/mocks/selectioncontrollermock.h"
#include "trackedit/tests/mocks/trackeditprojectmock.h"
#include "trackedit/dom/label.h"

using namespace ::testing;

namespace au::projectscene {
class TimelineContextLabelZoomTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SnapTestAccess::wireTimelineSelection(&m_context, m_globalContext, m_selection);

        ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_audacityProject));
        ON_CALL(*m_globalContext, currentTrackeditProject()).WillByDefault(Return(m_project));
        ON_CALL(*m_globalContext, playbackState()).WillByDefault(Return(m_playbackState));
        ON_CALL(*m_audacityProject, trackeditProject()).WillByDefault(Return(m_project));
        ON_CALL(*m_project, totalTime()).WillByDefault(Return(muse::secs_t(120.0)));
        ON_CALL(*m_playbackState, playbackPosition()).WillByDefault(Return(muse::secs_t(75.0)));

        m_context.onResizeFrameWidth(1000.0);
        m_context.setZoom(20.0, 0.0);
        m_context.moveToFrameTime(40.0);

        trackedit::Label point;
        point.key = m_pointKey;
        point.kind = trackedit::LabelKind::Point;
        point.startTime = 60.0;
        point.endTime = 60.0;
        ON_CALL(*m_project, label(m_pointKey)).WillByDefault(Return(point));
        ON_CALL(*m_selection, selectedLabels()).WillByDefault(Return(trackedit::LabelKeyList { m_pointKey }));
    }

    const trackedit::LabelKey m_pointKey { 1, 1 };
    std::shared_ptr<NiceMock<context::GlobalContextMock> > m_globalContext = std::make_shared<NiceMock<context::GlobalContextMock> >();
    std::shared_ptr<NiceMock<project::AudacityProjectMock> > m_audacityProject = std::make_shared<NiceMock<project::AudacityProjectMock> >();
    std::shared_ptr<NiceMock<context::PlaybackStateMock> > m_playbackState = std::make_shared<NiceMock<context::PlaybackStateMock> >();
    std::shared_ptr<NiceMock<trackedit::SelectionControllerMock> > m_selection = std::make_shared<NiceMock<trackedit::SelectionControllerMock> >();
    std::shared_ptr<NiceMock<trackedit::TrackeditProjectMock> > m_project = std::make_shared<NiceMock<trackedit::TrackeditProjectMock> >();
    TimelineContext m_context;
};

TEST_F(TimelineContextLabelZoomTests, SelectedPointStaysCenteredAfterToolbarZoom)
{
    ASSERT_DOUBLE_EQ(m_context.timeToPosition(60.0), 400.0);

    m_context.zoomIn();

    EXPECT_NEAR(m_context.timeToPosition(60.0), 500.0, 0.001);
}

TEST_F(TimelineContextLabelZoomTests, SelectedPointStaysCenteredAfterPinchZoom)
{
    ASSERT_DOUBLE_EQ(m_context.timeToPosition(60.0), 400.0);

    m_context.pinchToZoom(2.0, QPointF(700.0, 0.0));

    EXPECT_NEAR(m_context.timeToPosition(60.0), 500.0, 0.001);
}
}
