/*
 * Audacity: A Digital Audio Editor
 */
#include <gtest/gtest.h>

#include <QEventLoop>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QTimer>
#include <QUrl>

#include "ui/internal/qmlnetworkaccessmanagerfactory.h"

namespace au::appshell {
namespace {
TEST(LocalNetworkPolicy, rejectsRemoteRequests)
{
    muse::ui::QmlNetworkAccessManager manager;
    const QUrl remoteUrl("https://example.invalid/audio.wav");
    QNetworkReply* reply = manager.get(QNetworkRequest(remoteUrl));

    ASSERT_NE(reply, nullptr);
    EXPECT_EQ(reply->url(), remoteUrl);
    EXPECT_EQ(reply->error(), QNetworkReply::ContentAccessDenied);
    reply->deleteLater();
}

TEST(LocalNetworkPolicy, readsLocalFiles)
{
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    ASSERT_EQ(file.write("local audio"), 11);
    ASSERT_TRUE(file.flush());

    muse::ui::QmlNetworkAccessManager manager;
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl::fromLocalFile(file.fileName())));
    ASSERT_NE(reply, nullptr);

    QEventLoop loop;
    QTimer timeout;
    bool timedOut = false;
    timeout.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, [&]() {
        timedOut = true;
        loop.quit();
    });
    timeout.start(5000);
    loop.exec();

    EXPECT_FALSE(timedOut);
    EXPECT_EQ(reply->error(), QNetworkReply::NoError);
    EXPECT_EQ(reply->readAll(), QByteArray("local audio"));
    reply->deleteLater();
}
}
}
