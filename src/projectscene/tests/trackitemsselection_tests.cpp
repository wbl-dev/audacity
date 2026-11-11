/*
 * Audacity: A Digital Audio Editor
 */
#include <gtest/gtest.h>

#include "projectscene/view/tracksitemsview/trackclipslistmodel.h"
#include "projectscene/view/tracksitemsview/tracklabelslistmodel.h"
#include "projectscene/view/timeline/timelinecontext.h"

#include "trackedit/tests/mocks/selectioncontrollermock.h"
#include "trackedit/tests/mocks/tracknavigationcontrollermock.h"
#include "trackedit/tests/mocks/trackeditinteractionmock.h"
#include "trackedit/tests/mocks/trackeditprojectmock.h"
#include "context/tests/mocks/globalcontextmock.h"
#include "rcommand/tests/mocks/commanddispatchermock.h"
#include "playback/playbackcommands.h"

#include "testing/testcontext.h"

using namespace ::testing;

namespace au::projectscene {
static TrackItemKey makeKey(trackedit::TrackId trackId, trackedit::TrackItemId itemId)
{
    return TrackItemKey(trackedit::TrackItemKey(trackId, itemId));
}

// =====================================================================
// Clip selection tests
// =====================================================================

class TrackClipsSelectionTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_model = new TrackClipsListModel();
        m_selectionController = std::make_shared<NiceMock<trackedit::SelectionControllerMock> >();
        m_trackNavController = std::make_shared<NiceMock<trackedit::TrackNavigationControllerMock> >();
        m_trackeditInteraction = std::make_shared<NiceMock<trackedit::TrackeditInteractionMock> >();

        m_model->selectionController.set(m_selectionController);
        m_model->trackNavigationController.set(m_trackNavController);
        m_model->trackeditInteraction.set(m_trackeditInteraction);

        ON_CALL(*m_trackeditInteraction, itemGroupId(_))
        .WillByDefault(Return(int64_t(-1)));
    }

    void TearDown() override
    {
        delete m_model;
    }

    TrackClipsListModel* m_model = nullptr;
    std::shared_ptr<NiceMock<trackedit::SelectionControllerMock> > m_selectionController;
    std::shared_ptr<NiceMock<trackedit::TrackNavigationControllerMock> > m_trackNavController;
    std::shared_ptr<NiceMock<trackedit::TrackeditInteractionMock> > m_trackeditInteraction;
};

TEST_F(TrackClipsSelectionTests, SelectClip_SelectsClip_WhenNothingSelected)
{
    //! CASE Selecting a clip should select it
    auto key = makeKey(1, 42);

    ON_CALL(*m_selectionController, selectedClips())
    .WillByDefault(Return(trackedit::ClipKeyList {}));

    EXPECT_CALL(*m_selectionController, setSelectedClips(trackedit::ClipKeyList { key.key }, true))
    .Times(1);

    m_model->selectClip(key);
}

TEST_F(TrackClipsSelectionTests, SelectClip_SetsFocusedItem)
{
    //! CASE Selecting a clip should update focus
    auto key = makeKey(1, 42);

    ON_CALL(*m_selectionController, selectedClips())
    .WillByDefault(Return(trackedit::ClipKeyList {}));

    EXPECT_CALL(*m_trackNavController, setFocus(trackedit::TrackFocus::item(key.key), _)).Times(1);
    m_model->selectClip(key);
}

TEST_F(TrackClipsSelectionTests, SelectClip_SetsFocusedItem_WhenAlreadySelected)
{
    //! CASE Selecting an already selected clip should also update focus
    auto key = makeKey(1, 42);

    ON_CALL(*m_selectionController, selectedClips())
    .WillByDefault(Return(trackedit::ClipKeyList { key.key }));

    EXPECT_CALL(*m_trackNavController, setFocus(trackedit::TrackFocus::item(key.key), _)).Times(1);
    m_model->selectClip(key);
}

TEST_F(TrackClipsSelectionTests, SelectClip_SetsFocusedItem_GroupedClip)
{
    //! CASE Selecting a grouped clip should update focus
    auto key = makeKey(1, 42);
    auto other = makeKey(1, 43);
    int64_t groupId = 7;

    ON_CALL(*m_trackeditInteraction, itemGroupId(_))
    .WillByDefault(Return(groupId));
    ON_CALL(*m_trackeditInteraction, itemsInGroup(groupId))
    .WillByDefault(Return(trackedit::ItemKeys { { key.key, other.key }, {} }));

    EXPECT_CALL(*m_selectionController, setSelectedItems(trackedit::ItemKeys { { key.key, other.key }, {} }, true)).Times(1);
    EXPECT_CALL(*m_trackNavController, setFocus(trackedit::TrackFocus::track(key.key.trackId), _)).Times(1);
    EXPECT_CALL(*m_trackNavController, setFocus(trackedit::TrackFocus::item(key.key), _)).Times(1);
    m_model->selectClip(key);
}

// =====================================================================
// Label selection tests
// =====================================================================

class TrackLabelsSelectionTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_model = new TrackLabelsListModel();
        m_model->m_trackId = 1;

        m_selectionController = std::make_shared<NiceMock<trackedit::SelectionControllerMock> >();
        m_trackNavController = std::make_shared<NiceMock<trackedit::TrackNavigationControllerMock> >();
        m_trackeditInteraction = std::make_shared<NiceMock<trackedit::TrackeditInteractionMock> >();

        m_model->selectionController.set(m_selectionController);
        m_model->trackNavigationController.set(m_trackNavController);
        m_model->trackeditInteraction.set(m_trackeditInteraction);

        ON_CALL(*m_trackeditInteraction, itemGroupId(_))
        .WillByDefault(Return(int64_t(-1)));
    }

    void TearDown() override
    {
        delete m_model;
    }

    void setupActivation(const TrackItemKey& key, trackedit::Label label)
    {
        m_globalContext = std::make_shared<NiceMock<context::GlobalContextMock> >();
        m_trackeditProject = std::make_shared<NiceMock<trackedit::TrackeditProjectMock> >();
        m_commandDispatcher = std::make_shared<NiceMock<muse::rcommand::CommandDispatcherMock> >();
        m_model->globalContext.set(m_globalContext);
        m_model->commandDispatcher.set(m_commandDispatcher);
        ON_CALL(*m_globalContext, currentTrackeditProject()).WillByDefault(Return(m_trackeditProject));
        ON_CALL(*m_trackeditProject, label(key.key)).WillByDefault(Return(label));
    }

    TrackLabelsListModel* m_model = nullptr;
    std::shared_ptr<NiceMock<trackedit::SelectionControllerMock> > m_selectionController;
    std::shared_ptr<NiceMock<trackedit::TrackNavigationControllerMock> > m_trackNavController;
    std::shared_ptr<NiceMock<trackedit::TrackeditInteractionMock> > m_trackeditInteraction;
    std::shared_ptr<NiceMock<context::GlobalContextMock> > m_globalContext;
    std::shared_ptr<NiceMock<trackedit::TrackeditProjectMock> > m_trackeditProject;
    std::shared_ptr<NiceMock<muse::rcommand::CommandDispatcherMock> > m_commandDispatcher;
};

TEST_F(TrackLabelsSelectionTests, ActivateRangeSelectsAudioAndPlaysExactSelection)
{
    const auto key = makeKey(1, 42);
    trackedit::Label label;
    label.key = key.key;
    label.startTime = 2.0;
    label.endTime = 5.0;
    label.kind = trackedit::LabelKind::Range;
    setupActivation(key, label);

    EXPECT_CALL(*m_selectionController, setSelectedAllAudioData(Optional(2.0), Optional(5.0)));
    EXPECT_CALL(*m_selectionController, addSelectedLabel(key.key));
    muse::rcommand::Request captured;
    EXPECT_CALL(*m_commandDispatcher, dispatch(_)).WillOnce(DoAll(SaveArg<0>(&captured),
                                                                  Invoke([](const muse::rcommand::Request& request) {
        return muse::async::make_promise<muse::rcommand::Response>([request](auto resolve) {
            return resolve(muse::rcommand::make_response(request, muse::make_ok()));
        });
    })));

    m_model->activateLabel(key);
    EXPECT_EQ(captured.command, playback::PLAYBACK_PLAY_SELECTION_COMMAND);
}

TEST_F(TrackLabelsSelectionTests, ActivatePointSeeksAndStartsOrdinaryPlayback)
{
    const auto key = makeKey(1, 43);
    trackedit::Label label;
    label.key = key.key;
    label.startTime = 3.0;
    label.endTime = 3.0;
    label.kind = trackedit::LabelKind::Point;
    setupActivation(key, label);

    EXPECT_CALL(*m_selectionController, resetTimeSelection());
    EXPECT_CALL(*m_selectionController, setSelectionStartTime(trackedit::secs_t(3.0)));
    EXPECT_CALL(*m_selectionController, addSelectedLabel(key.key));
    muse::rcommand::Request captured;
    EXPECT_CALL(*m_commandDispatcher, dispatch(_)).WillOnce(DoAll(SaveArg<0>(&captured),
                                                                  Invoke([](const muse::rcommand::Request& request) {
        return muse::async::make_promise<muse::rcommand::Response>([request](auto resolve) {
            return resolve(muse::rcommand::make_response(request, muse::make_ok()));
        });
    })));

    m_model->activateLabel(key);
    EXPECT_EQ(captured.command, playback::PLAYBACK_SEEK_COMMAND);
    EXPECT_DOUBLE_EQ(captured.params.at(playback::PLAYBACK_SEEK_TIME_PARAM).toDouble(), 3.0);
    EXPECT_TRUE(captured.params.at(playback::PLAYBACK_SEEK_TRIGGER_PLAY_PARAM).toBool());
}

TEST_F(TrackLabelsSelectionTests, SelectLabel_SetsRangeAnchor_GroupedLabel)
{
    //! CASE Clicking a grouped label anchors later range selections at it, as a grouped clip click does
    TimelineContext context;
    m_model->setTimelineContext(&context);
    auto key = makeKey(1, 42);
    auto other = makeKey(1, 43);
    ON_CALL(*m_trackeditInteraction, itemGroupId(_)).WillByDefault(Return(int64_t(7)));
    ON_CALL(*m_trackeditInteraction, itemsInGroup(int64_t(7))).WillByDefault(Return(trackedit::ItemKeys { {}, { key.key, other.key } }));

    EXPECT_CALL(*m_selectionController, setItemSelectionAnchor(_, key.key)).Times(1);
    m_model->selectLabel(key);
    m_model->setTimelineContext(nullptr);
}

TEST_F(TrackLabelsSelectionTests, SelectLabel_SelectsLabel_WhenNothingSelected)
{
    //! CASE Selecting a label should select it
    //!
    auto key = makeKey(1, 10);

    ON_CALL(*m_selectionController, selectedLabels())
    .WillByDefault(Return(trackedit::LabelKeyList {}));

    EXPECT_CALL(*m_selectionController, setSelectedLabels(trackedit::LabelKeyList { key.key }, true))
    .Times(1);

    m_model->selectLabel(key);
}

TEST_F(TrackLabelsSelectionTests, SelectLabel_SetsFocusedItem)
{
    //! CASE Selecting a label should update focus
    auto key = makeKey(1, 10);

    ON_CALL(*m_selectionController, selectedLabels())
    .WillByDefault(Return(trackedit::LabelKeyList {}));

    EXPECT_CALL(*m_trackNavController, setFocus(trackedit::TrackFocus::item(key.key), _)).Times(1);
    m_model->selectLabel(key);
}

TEST_F(TrackLabelsSelectionTests, SelectLabel_SetsFocusedItem_WhenAlreadySelected)
{
    //! CASE Selecting an already selected label should also update focus
    auto key = makeKey(1, 10);

    ON_CALL(*m_selectionController, selectedLabels())
    .WillByDefault(Return(trackedit::LabelKeyList { key.key }));

    EXPECT_CALL(*m_trackNavController, setFocus(trackedit::TrackFocus::item(key.key), _)).Times(1);
    m_model->selectLabel(key);
}

TEST_F(TrackLabelsSelectionTests, SelectLabel_ReplacesPreviouslySelectedLabel)
{
    const auto first = makeKey(1, 10);
    const auto second = makeKey(1, 11);
    ON_CALL(*m_selectionController, selectedLabels())
    .WillByDefault(Return(trackedit::LabelKeyList { first.key }));
    ON_CALL(*m_selectionController, timeSelectionIsEmpty()).WillByDefault(Return(false));
    ON_CALL(*m_selectionController, labelsIntersectingRangeSelection())
    .WillByDefault(Return(trackedit::LabelKeyList { second.key }));

    EXPECT_CALL(*m_selectionController, setSelectedLabels(trackedit::LabelKeyList { second.key }, true)).Times(1);
    EXPECT_CALL(*m_selectionController, addSelectedLabel(second.key)).Times(0);
    m_model->selectLabel(second);
    EXPECT_TRUE(m_model->toggleTracksDataSelectionByLabel(second));
}

TEST_F(TrackLabelsSelectionTests, ClickSelectedLabelDeselectsOnRelease)
{
    const auto key = makeKey(1, 10);
    ON_CALL(*m_selectionController, selectedLabels())
    .WillByDefault(Return(trackedit::LabelKeyList { key.key }));

    m_model->selectLabel(key);
    EXPECT_CALL(*m_selectionController, removeLabelSelection(key.key)).Times(1);
    EXPECT_FALSE(m_model->toggleTracksDataSelectionByLabel(key));
}
}
