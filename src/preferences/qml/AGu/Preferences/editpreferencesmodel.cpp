/*
 * Audacity: A Digital Audio Editor
 */
#include "editpreferencesmodel.h"

#include "settings.h"
#include "translation.h"

namespace agu::appshell {
EditPreferencesModel::EditPreferencesModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void EditPreferencesModel::init()
{
    effectsConfiguration()->applyEffectToAllAudioChanged().onNotify(this, [this]{
        emit applyEffectToAllAudioChanged();
    });

    projectsceneConfiguration()->stereoHeightsPrefChanged().onNotify(this, [this] {
        emit stereoHeightsPrefChanged();
    });

    trackeditConfiguration()->deleteBehaviorChanged().onNotify(this, [this] {
        emit deleteBehaviorPrefChanged();
    });

    trackeditConfiguration()->closeGapBehaviorChanged().onNotify(this, [this] {
        emit closeGapBehaviorChanged();
    });

    trackeditConfiguration()->pasteBehaviorChanged().onNotify(this, [this] {
        emit pasteBehaviorPrefChanged();
    });

    trackeditConfiguration()->pasteInsertBehaviorChanged().onNotify(this, [this] {
        emit pasteInsertBehaviorPrefChanged();
    });

    trackeditConfiguration()->pasteAsNewClipChanged().onNotify(this, [this] {
        emit pasteAsNewClipChanged();
    });

    trackeditConfiguration()->askBeforeConvertingToMonoOrStereoChanged().onNotify(this, [this]{
        emit askBeforeConvertingToMonoOrStereoChanged();
    });

}

bool EditPreferencesModel::askBeforeConvertingToMonoOrStereo() const
{
    return trackeditConfiguration()->askBeforeConvertingToMonoOrStereo();
}

void EditPreferencesModel::setAskBeforeConvertingToMonoOrStereo(bool value)
{
    trackeditConfiguration()->setAskBeforeConvertingToMonoOrStereo(value);
}

bool EditPreferencesModel::applyEffectToAllAudio() const
{
    return effectsConfiguration()->applyEffectToAllAudio();
}

void EditPreferencesModel::setApplyEffectToAllAudio(bool value)
{
    if (applyEffectToAllAudio() == value) {
        return;
    }
    effectsConfiguration()->setApplyEffectToAllAudio(value);
}

projectscene::StereoHeightsPref::AsymmetricStereoHeights
EditPreferencesModel::stereoHeightsPref() const
{
    return projectsceneConfiguration()->stereoHeightsPref();
}

void EditPreferencesModel::setStereoHeightsPref(
    projectscene::StereoHeightsPref::AsymmetricStereoHeights pref)
{
    if (stereoHeightsPref() == pref) {
        return;
    }
    projectsceneConfiguration()->setStereoHeightsPref(pref);
}

int EditPreferencesModel::deleteBehavior() const
{
    return static_cast<int>(trackeditConfiguration()->deleteBehavior());
}

void EditPreferencesModel::setDeleteBehavior(int pref)
{
    if (deleteBehavior() == pref) {
        return;
    }
    trackeditConfiguration()->setDeleteBehavior(static_cast<agu::trackedit::DeleteBehavior>(pref));
}

int EditPreferencesModel::closeGapBehavior() const
{
    return static_cast<int>(trackeditConfiguration()->closeGapBehavior());
}

void EditPreferencesModel::setCloseGapBehavior(int pref)
{
    if (closeGapBehavior() == pref) {
        return;
    }
    trackeditConfiguration()->setCloseGapBehavior(static_cast<agu::trackedit::CloseGapBehavior>(pref));
}

int EditPreferencesModel::pasteBehavior() const
{
    return static_cast<int>(trackeditConfiguration()->pasteBehavior());
}

void EditPreferencesModel::setPasteBehavior(int pref)
{
    if (pasteBehavior() == pref) {
        return;
    }
    trackeditConfiguration()->setPasteBehavior(static_cast<agu::trackedit::PasteBehavior>(pref));
}

int EditPreferencesModel::pasteInsertBehavior() const
{
    return static_cast<int>(trackeditConfiguration()->pasteInsertBehavior());
}

void EditPreferencesModel::setPasteInsertBehavior(int pref)
{
    if (pasteInsertBehavior() == pref) {
        return;
    }
    trackeditConfiguration()->setPasteInsertBehavior(static_cast<agu::trackedit::PasteInsertBehavior>(pref));
}

bool EditPreferencesModel::pasteAsNewClip() const
{
    return trackeditConfiguration()->pasteAsNewClip();
}

void EditPreferencesModel::setPasteAsNewClip(bool value)
{
    if (pasteAsNewClip() == value) {
        return;
    }
    trackeditConfiguration()->setPasteAsNewClip(value);
}

int EditPreferencesModel::zoomPreset1() const
{
    return static_cast<int>(projectsceneConfiguration()->zoomPreset1());
}

void EditPreferencesModel::setZoomPreset1(int preset)
{
    if (zoomPreset1() == preset) {
        return;
    }
    projectsceneConfiguration()->setZoomPreset1(static_cast<projectscene::ZoomPresets::Preset>(preset));
    emit zoomPreset1Changed();
}

int EditPreferencesModel::zoomPreset2() const
{
    return static_cast<int>(projectsceneConfiguration()->zoomPreset2());
}

void EditPreferencesModel::setZoomPreset2(int preset)
{
    if (zoomPreset2() == preset) {
        return;
    }
    projectsceneConfiguration()->setZoomPreset2(static_cast<projectscene::ZoomPresets::Preset>(preset));
    emit zoomPreset2Changed();
}

QVariantList EditPreferencesModel::zoomPresetList() const
{
    QVariantList list;
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "Fit to Width") }, { "value", 0 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "Zoom to Selection") }, { "value", 1 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "Zoom Default") }, { "value", 2 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "Minutes") }, { "value", 3 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "Seconds") }, { "value", 4 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "5ths of Seconds") }, { "value", 5 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "10ths of Seconds") }, { "value", 6 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "20ths of Seconds") }, { "value", 7 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "50ths of Seconds") }, { "value", 8 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "100ths of Seconds") }, { "value", 9 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "500ths of Seconds") }, { "value", 10 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "MilliSeconds") }, { "value", 11 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "Samples") }, { "value", 12 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "4 Pixels per Sample") }, { "value", 13 } };
    list << QVariantMap{ { "title", muse::qtrc("appshell/preferences", "Max Zoom") }, { "value", 14 } };
    return list;
}
}
