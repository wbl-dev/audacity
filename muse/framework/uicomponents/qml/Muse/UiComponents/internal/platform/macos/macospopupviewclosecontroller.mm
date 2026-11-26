/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "macospopupviewclosecontroller.h"

#include <AppKit/NSEvent.h>
#include <QQuickWindow>

using namespace muse::uicomponents;

struct MacOSPopupViewCloseController::Private {
    id<NSObject> minimizeObserverToken = nil;
};

MacOSPopupViewCloseController::MacOSPopupViewCloseController(const modularity::ContextPtr& iocCtx, QObject* parent)
    : PopupViewCloseController(iocCtx, parent), m_private(std::make_unique<Private>())
{
}

MacOSPopupViewCloseController::~MacOSPopupViewCloseController()
{
    if (qApp) {
        qApp->removeNativeEventFilter(this);
    }
    removeWindowMinimizedObserver();
}

void MacOSPopupViewCloseController::doUpdateEventFilters()
{
    removeWindowMinimizedObserver();

    if (active()) {
        qApp->installNativeEventFilter(this);
        initWindowMinimizedObserver();
    } else {
        qApp->removeNativeEventFilter(this);
    }

    PopupViewCloseController::doUpdateEventFilters();
}

bool MacOSPopupViewCloseController::nativeEventFilter(const QByteArray& eventType, void* message, qintptr*)
{
    if (eventType != "mac_generic_NSEvent") {
        return false;
    }

    NSEvent* event = static_cast<NSEvent*>(message);
    if ([event type] == NSEventTypeRightMouseDown || [event type] == NSEventTypeLeftMouseDown) {
        doFocusOut(QCursor::pos());
    }

    return false;
}

void MacOSPopupViewCloseController::initWindowMinimizedObserver()
{
    QQuickItem* parent = parentItem();
    QQuickWindow* window = parent ? parent->window() : nullptr;
    if (!window) {
        return;
    }

    WId wid = window->winId();
    NSView* nsView = (__bridge NSView*)reinterpret_cast<void*>(wid);
    NSWindow* nsWindow = [nsView window];
    // A nil notification object would subscribe to every window's minimization.
    if (!nsWindow) {
        return;
    }

    m_private->minimizeObserverToken = [[NSNotificationCenter defaultCenter]
                             addObserverForName:@"NSWindowWillMiniaturizeNotification"
                             object:nsWindow
                             queue:nil
                             usingBlock:^(NSNotification*) {
                                 notifyAboutClose();
                             }];
}

void MacOSPopupViewCloseController::removeWindowMinimizedObserver()
{
    if (m_private->minimizeObserverToken) {
        [[NSNotificationCenter defaultCenter] removeObserver:m_private->minimizeObserverToken];
        m_private->minimizeObserverToken = nil;
    }
}
