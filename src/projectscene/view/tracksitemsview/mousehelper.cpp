/*
 * Audacity: A Digital Audio Editor
 */
#include "mousehelper.h"

namespace agu::projectscene {
void MouseHelper::callUngrabMouseOnItem(QQuickItem* item)
{
    if (item) {
        item->ungrabMouse();
    }
}
}
