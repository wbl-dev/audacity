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

#include <AppKit/AppKit.h>
#include <QGuiApplication>
#include <QQuickWindow>

#include <memory>

#include "uicomponents/qml/Muse/UiComponents/internal/platform/macos/macospopupviewclosecontroller.h"

using namespace muse::uicomponents;

namespace {
class MacOSPopupViewCloseControllerTests : public ::testing::Test, public muse::async::Asyncable
{
protected:
    void SetUp() override
    {
        if (QGuiApplication::platformName() != "cocoa") {
            GTEST_SKIP() << "Native minimize notifications require the Cocoa platform";
        }
    }

    static NSWindow* nativeWindow(QQuickWindow& window)
    {
        NSView* view = (__bridge NSView*)reinterpret_cast<void*>(window.winId());
        return [view window];
    }

    static void notifyMinimized(NSWindow* window)
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:NSWindowWillMiniaturizeNotification object:window];
    }
};
}

TEST_F(MacOSPopupViewCloseControllerTests, DetachedParentCanAttachAfterActivation)
{
    QQuickItem parent;
    MacOSPopupViewCloseController controller(nullptr);
    controller.setParentItem(&parent);
    int closeCount = 0;
    controller.closeNotification().onNotify(this, [&]() { ++closeCount; });

    // A delayed popup can open after undo has detached its anchor from the scene.
    controller.setActive(true);
    QQuickWindow window;
    NSWindow* native = nativeWindow(window);
    ASSERT_NE(native, nil);
    notifyMinimized(native);
    EXPECT_EQ(closeCount, 0);

    parent.setParentItem(window.contentItem());
    notifyMinimized(native);
    EXPECT_EQ(closeCount, 1);
    controller.setActive(false);
    parent.setParentItem(nullptr);
}

TEST_F(MacOSPopupViewCloseControllerTests, MinimizeObserversBelongToEachPopup)
{
    QQuickWindow window;
    NSWindow* native = nativeWindow(window);
    ASSERT_NE(native, nil);
    QQuickItem parent(window.contentItem());
    MacOSPopupViewCloseController first(nullptr);
    MacOSPopupViewCloseController second(nullptr);
    first.setParentItem(&parent);
    second.setParentItem(&parent);
    int firstCloses = 0;
    int secondCloses = 0;
    first.closeNotification().onNotify(this, [&]() { ++firstCloses; });
    second.closeNotification().onNotify(this, [&]() { ++secondCloses; });
    first.setActive(true);
    second.setActive(true);

    notifyMinimized(native);
    EXPECT_EQ(firstCloses, 1);
    EXPECT_EQ(secondCloses, 1);

    first.setActive(false);
    notifyMinimized(native);
    EXPECT_EQ(firstCloses, 1);
    EXPECT_EQ(secondCloses, 2);
    second.setActive(false);
}

TEST_F(MacOSPopupViewCloseControllerTests, ObserverFollowsParentWindow)
{
    QQuickWindow firstWindow;
    QQuickWindow secondWindow;
    NSWindow* firstNative = nativeWindow(firstWindow);
    NSWindow* secondNative = nativeWindow(secondWindow);
    ASSERT_NE(firstNative, nil);
    ASSERT_NE(secondNative, nil);
    QQuickItem parent(firstWindow.contentItem());
    MacOSPopupViewCloseController controller(nullptr);
    controller.setParentItem(&parent);
    int closeCount = 0;
    controller.closeNotification().onNotify(this, [&]() { ++closeCount; });
    controller.setActive(true);

    parent.setParentItem(secondWindow.contentItem());
    notifyMinimized(firstNative);
    EXPECT_EQ(closeCount, 0);
    notifyMinimized(secondNative);
    EXPECT_EQ(closeCount, 1);

    parent.setParentItem(nullptr);
    ASSERT_EQ(parent.window(), nullptr);
    // Detaching can also close the popup through visibleChanged.
    int closesAfterDetach = closeCount;
    notifyMinimized(secondNative);
    EXPECT_EQ(closeCount, closesAfterDetach);
    controller.setActive(false);
}

TEST_F(MacOSPopupViewCloseControllerTests, DestroyingControllerRemovesObserver)
{
    QQuickWindow window;
    NSWindow* native = nativeWindow(window);
    ASSERT_NE(native, nil);
    QQuickItem parent(window.contentItem());
    auto controller = std::make_unique<MacOSPopupViewCloseController>(nullptr);
    controller->setParentItem(&parent);
    int closeCount = 0;
    controller->closeNotification().onNotify(this, [&]() { ++closeCount; });
    controller->setActive(true);
    controller.reset();

    notifyMinimized(native);
    EXPECT_EQ(closeCount, 0);
}
