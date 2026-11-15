import QtQuick
import Muse.Ui
import AGu.Effects
import AGu.BuiltinEffects
import AGu.BuiltinEffectsCollection

import "../../../dynamics/timeline"

BuiltinEffectBase {
    property alias playState: playStateModel.playState

    readonly property color gridColor: ui.theme.extra["dynamics_effect_grid_color"]

    DynamicsPlayStateModel {
        id: playStateModel
    }

    Component.onCompleted: {
        playStateModel.init()
    }
}
