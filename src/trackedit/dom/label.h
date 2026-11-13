/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include <vector>

#include "global/types/string.h"
#include "../trackedittypes.h"

namespace au::trackedit {
enum class LabelKind {
    Point,
    Range,
};

struct Label {
    LabelKey key;

    muse::String title;
    ClipColorIndex colorIndex = CLIP_COLOR_INDEX_NONE;
    double startTime = 0.0;
    double endTime = 0.0;
    double lowFrequency = 0.0;
    double highFrequency = 0.0;
    int64_t groupId = -1;
    LabelKind kind = LabelKind::Point;
    muse::String color = {};
    muse::String owner = {};

    inline bool isValid() const { return key.isValid(); }
};

using Labels = std::vector<Label>;
}
