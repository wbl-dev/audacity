/*
* Audacity: A Digital Audio Editor
*/
#include "recentprojectcontextmenumodel.h"

#include "framework/global/log.h"
#include "framework/global/types/translatablestring.h"

#include <QUrl>

#include "../projectcommands.h"

using namespace agu::project;
using namespace muse::rcommand;

namespace {
const QString SHOW_IN_FOLDER_ITEM_ID("project-show-in-folder");

muse::TranslatableString showInFolderTitle()
{
#if defined(Q_OS_MAC)
    return muse::TranslatableString("action", "Show in Finder");
#elif defined(Q_OS_WIN)
    return muse::TranslatableString("action", "Show in Explorer");
#else
    return muse::TranslatableString("action", "Show in File Manager");
#endif
}
}

RecentProjectContextMenuModel::RecentProjectContextMenuModel(QString path, QString displayNameOverride, QObject* parent)
    : AbstractMenuModel(parent), m_path(std::move(path)), m_displayNameOverride(std::move(displayNameOverride))
{
}

void RecentProjectContextMenuModel::load()
{
    muse::uicomponents::AbstractMenuModel::load();

    muse::uicomponents::MenuItem* openItem = makeMenuItem(PROJECT_OPEN_COMMAND);
    IF_ASSERT_FAILED(openItem) {
        return;
    }

    openItem->setParams({
        { PROJECT_URL_PARAM, muse::Val(QUrl::fromLocalFile(m_path).toString()) },
        { PROJECT_DISPLAY_NAME_PARAM, muse::Val(m_displayNameOverride) },
    });
    openItem->setTitle(muse::TranslatableString("action", "Open"));

    muse::uicomponents::MenuItemList items = { openItem };
    if (!m_path.isEmpty()) {
        muse::uicomponents::MenuItem* showInFolderItem = new muse::uicomponents::MenuItem(this);
        showInFolderItem->setId(SHOW_IN_FOLDER_ITEM_ID);
        showInFolderItem->setTitle(showInFolderTitle());
        showInFolderItem->setEnabled(true);
        items.append(showInFolderItem);
    }

    setItems(items);
}

void RecentProjectContextMenuModel::handleMenuItem(const QString& itemId)
{
    if (itemId == SHOW_IN_FOLDER_ITEM_ID) {
        if (m_path.isEmpty()) {
            return;
        }

        platformInteractive()->revealInFileBrowser(m_path);
        return;
    }

    AbstractMenuModel::handleMenuItem(itemId);
}
