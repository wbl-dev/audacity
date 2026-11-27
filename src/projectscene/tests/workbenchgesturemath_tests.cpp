/*
 * Audacity: A Digital Audio Editor
 */
#include <gtest/gtest.h>

#include <QFile>
#include <QFileInfo>
#include <QJSEngine>

namespace {
QJSValue loadScript(QJSEngine& engine, const QString& name)
{
    const QString path = QFileInfo(QString::fromUtf8(__FILE__)).absolutePath()
                         + QStringLiteral("/../qml/AGu/ProjectScene/") + name;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return engine.newErrorObject(QJSValue::GenericError, QStringLiteral("Cannot open ") + path);
    }
    return engine.evaluate(QString::fromUtf8(file.readAll()), path);
}
}

TEST(WorkbenchGestureMathTests, NarrowOverviewCenterRemainsPanTarget)
{
    QJSEngine engine;
    ASSERT_FALSE(loadScript(engine, QStringLiteral("tracksitemsview/OverviewNavigatorMath.js")).isError());
    const QJSValue edgeAt = engine.globalObject().property(QStringLiteral("resizeEdgeAt"));
    ASSERT_TRUE(edgeAt.isCallable());

    EXPECT_EQ(edgeAt.call({ 105, 100, 110 }).toInt(), 0);
    EXPECT_EQ(edgeAt.call({ 99, 100, 110 }).toInt(), -1);
    EXPECT_EQ(edgeAt.call({ 111, 100, 110 }).toInt(), 1);
}

TEST(WorkbenchGestureMathTests, VolumeSnapAndFineMovementRespectBounds)
{
    QJSEngine engine;
    ASSERT_FALSE(loadScript(engine, QStringLiteral("trackspanel/audio/VolumeSliderMath.js")).isError());
    const QJSValue snap = engine.globalObject().property(QStringLiteral("snapVolume"));
    const QJSValue clamp = engine.globalObject().property(QStringLiteral("clampVolume"));
    const QJSValue pointer = engine.globalObject().property(QStringLiteral("volumeAtPointer"));
    ASSERT_TRUE(snap.isCallable());
    ASSERT_TRUE(clamp.isCallable());
    ASSERT_TRUE(pointer.isCallable());

    EXPECT_DOUBLE_EQ(snap.call({ 0.9, false }).toNumber(), 0.0);
    EXPECT_DOUBLE_EQ(snap.call({ 0.9, true }).toNumber(), 0.9);
    EXPECT_DOUBLE_EQ(snap.call({ 1.3, false }).toNumber(), 1.3);
    EXPECT_DOUBLE_EQ(clamp.call({ 14, -60, 12 }).toNumber(), 12.0);
    EXPECT_DOUBLE_EQ(clamp.call({ -64, -60, 12 }).toNumber(), -60.0);

    const double handleCenter = 4.0 + (60.0 / 72.0) * 92.0;
    EXPECT_NEAR(pointer.call({ handleCenter + 2.0, 100, 8, 2.0, -60, 12 }).toNumber(), 0.0, 1e-9);
}
