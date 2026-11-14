/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "qmlnetworkaccessmanagerfactory.h"

#include <QNetworkReply>
#include <QTimer>

using namespace muse::ui;

namespace {
class DeniedNetworkReply final : public QNetworkReply
{
public:
    DeniedNetworkReply(const QNetworkRequest& request, QNetworkAccessManager::Operation operation, QObject* parent)
        : QNetworkReply(parent)
    {
        setRequest(request);
        setUrl(request.url());
        setOperation(operation);
        setError(QNetworkReply::ContentAccessDenied, "Remote resources are unavailable");
        open(QIODevice::ReadOnly);
        QTimer::singleShot(0, this, [this]() {
            setFinished(true);
            emit errorOccurred(error());
            emit finished();
        });
    }

    void abort() override {}

protected:
    qint64 readData(char*, qint64) override { return -1; }
};
}

QNetworkReply* QmlNetworkAccessManager::createRequest(Operation operation, const QNetworkRequest& request, QIODevice* outgoingData)
{
    const QString scheme = request.url().scheme().toLower();
    if (scheme == "qrc" || scheme == "file" || scheme == "data") {
        return QNetworkAccessManager::createRequest(operation, request, outgoingData);
    }
    return new DeniedNetworkReply(request, operation, this);
}

QNetworkAccessManager* QmlNetworkAccessManagerFactory::create(QObject* parent)
{
    return new QmlNetworkAccessManager(parent);
}
