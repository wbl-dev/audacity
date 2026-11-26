/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore/Audacity and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include <memory>

#include "uicomponents/qml/Muse/UiComponents/internal/popupviewclosecontroller.h"

using namespace muse::uicomponents;

namespace {
class PopupViewCloseControllerTests : public ::testing::Test, public muse::async::Asyncable
{
};
}

TEST_F(PopupViewCloseControllerTests, DestroyedParentIsClearedBeforeCloseNotification)
{
    PopupViewCloseController controller(nullptr);
    auto parent = std::make_unique<QQuickItem>();
    controller.setParentItem(parent.get());
    controller.setActive(true);

    int closeCount = 0;
    controller.closeNotification().onNotify(this, [&]() {
        ++closeCount;
        EXPECT_EQ(controller.parentItem(), nullptr);
        EXPECT_FALSE(controller.active());
    });

    parent.reset();

    EXPECT_EQ(closeCount, 1);
    EXPECT_EQ(controller.parentItem(), nullptr);
    EXPECT_FALSE(controller.active());
}
