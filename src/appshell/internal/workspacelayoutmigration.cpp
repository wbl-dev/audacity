/*
 * AGu: A Digital Audio Editor
 */
#include "workspacelayoutmigration.h"

#include "ui/iuistate.h"

void agu::appshell::migrateLegacyWorkspaceLayout(muse::ui::IUiState& state)
{
    static const QString revisionKey = "agu/layoutRevision";
    if (state.uiItemState(revisionKey) == "1") {
        return;
    }

    for (const QString& page : { QString("Home"), QString("ProjectPage") }) {
        const QByteArray layout = state.pageState(page).val;
        if (!layout.isEmpty()) {
            const QString backupKey = "agu/layoutBackup/" + page;
            if (state.pageState(backupKey).val.isEmpty()) {
                state.setPageState(backupKey, layout);
            }
            state.setPageState(page, {});
        }
    }

    const QByteArray window = state.windowGeometry();
    if (!window.isEmpty()) {
        if (state.pageState("agu/layoutBackup/window").val.isEmpty()) {
            state.setPageState("agu/layoutBackup/window", window);
        }
        state.setWindowGeometry({});
    }

    state.setUiItemState(revisionKey, "1");
}
