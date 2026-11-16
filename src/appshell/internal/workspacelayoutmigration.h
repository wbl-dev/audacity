/*
 * AGu: A Digital Audio Editor
 */
#pragma once

namespace muse::ui {
class IUiState;
}

namespace agu::appshell {
void migrateLegacyWorkspaceLayout(muse::ui::IUiState& state);
}
