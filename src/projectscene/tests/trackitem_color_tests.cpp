#include "projectscene/view/trackspanel/labeltrackitem.h"

#include "trackedit/tests/mocks/tracknavigationcontrollermock.h"

#include <gtest/gtest.h>

using namespace agu::projectscene;
using namespace agu::trackedit;

class TestLabelTrackItem : public LabelTrackItem
{
public:
    using TrackItem::trackNavigationController;
};

TEST(TrackItemColorTests, TrackColorIndexFollowsChangedTrack)
{
    auto navigation = std::make_shared<::testing::NiceMock<TrackNavigationControllerMock> >();

    TestLabelTrackItem item;
    item.trackNavigationController.set(navigation);
    Track track;
    track.id = 12;
    track.type = TrackType::Label;
    track.colorIndex = 2;
    item.init(track);
    EXPECT_EQ(item.colorIndex(), 2);

    track.colorIndex = 5;
    item.init(track);
    EXPECT_EQ(item.colorIndex(), 5);
}
