/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <QtQml/qqml.h>

#include "effectsviewtypes.h"

#include "framework/uicomponents/qml/Muse/UiComponents/menuitem.h"

#define REGISTER_AUDACITY_EFFECTS_SINGLETON_TYPE(Factory) \
    qmlRegisterSingletonType<Factory>("AGu.Effects", 1, 0, #Factory, \
                                      [] (QQmlEngine*, QJSEngine*) -> QObject* { \
        return new Factory(); \
    });

namespace agu::effects::utils {
muse::uicomponents::MenuItemList toMenuItemList(const std::vector<DropdownOption>&, int selectedIndex, QObject* parent = nullptr);
}
