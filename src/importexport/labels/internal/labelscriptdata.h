/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <optional>
#include <vector>

#include <QString>

namespace au::importexport {
enum class ScriptLabelKind {
    Point, Range
};

struct ScriptLabel {
    ScriptLabelKind kind = ScriptLabelKind::Point;
    double start = 0.0;
    double end = 0.0;
    QString title;
    QString color;
    QString owner;
    std::optional<double> lowFrequency;
    std::optional<double> highFrequency;
};

struct ParsedLabelTrack {
    QString trackName;
    std::vector<ScriptLabel> labels;
};

struct LabelScriptInput {
    QString fileName;
    QString extension;
    QString text;
};
}
