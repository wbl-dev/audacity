/*
* Audacity: A Digital Audio Editor
*/

#include "labelsconfiguration.h"

#include <algorithm>

#include "global/settings.h"
#include "global/translation.h"
#include "au3-import-export/Import.h"
#include "au3-files/FileNames.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>

using namespace au::importexport;

static const std::string module_name("iex_labels");

static const muse::Settings::Key LABELS_DIRECTORY_PATH(module_name, "importexport/labelsDirectoryPath");

LabelsConfiguration::LabelsConfiguration(QString profileFilePath)
    : m_profileFilePath(std::move(profileFilePath))
{
}

void LabelsConfiguration::init()
{
    muse::settings()->setDefaultValue(LABELS_DIRECTORY_PATH, muse::Val(globalConfiguration()->userDataPath()));
}

std::vector<FileFilter> LabelsConfiguration::fileFilter() const
{
    return { { FileType::TEXT, muse::trc("importexport", "Text file (*.txt)") },
        { FileType::SUBRIP, muse::trc("importexport", "SubRip text file (*.srt)") },
        { FileType::WEBVTT, muse::trc("importexport", "WebVTT file (*.vtt)") } };
}

std::vector<FileFilter> LabelsConfiguration::importFileFilter() const
{
    std::vector<FileFilter> filters { { FileType::TEXT, muse::trc("importexport", "Text file (*.txt)") } };
    for (const auto& profile : parserProfiles()) {
        if (!profile.enabled) {
            continue;
        }
        QStringList patterns;
        for (const QString& suffix : profile.suffixes) {
            if (suffix != "txt") {
                patterns.push_back("*." + suffix);
            }
        }
        if (!patterns.empty()) {
            filters.push_back({ FileType::UNDEFINED,
                                QString("%1 (%2)").arg(profile.displayName, patterns.join(' ')).toStdString() });
        }
    }
    return filters;
}

QString LabelsConfiguration::profileFilePath() const
{
    if (!m_profileFilePath.isEmpty()) {
        return m_profileFilePath;
    }
    return globalConfiguration()->userDataPath().toQString() + "/label-parsers.json";
}

std::vector<LabelParserProfile> LabelsConfiguration::readProfiles(bool* valid) const
{
    if (valid) {
        *valid = true;
    }
    QFile file(profileFilePath());
    if (!file.exists()) {
        return {};
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (valid) {
            *valid = false;
        }
        return {};
    }
    if (file.size() > 4 * 1024 * 1024) {
        if (valid) {
            *valid = false;
        }
        return {};
    }
    const QByteArray bytes = file.read(4 * 1024 * 1024 + 1);
    if (bytes.size() > 4 * 1024 * 1024 || !file.atEnd()) {
        if (valid) {
            *valid = false;
        }
        return {};
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);
    if (!document.isObject() || !document.object().value("profiles").isArray()) {
        if (valid) {
            *valid = false;
        }
        return {};
    }
    std::vector<LabelParserProfile> profiles;
    for (const auto& value : document.object().value("profiles").toArray()) {
        const QJsonObject object = value.toObject();
        if (!object.value("id").isString() || !object.value("displayName").isString()
            || !object.value("source").isString() || !object.value("suffixes").isArray()) {
            if (valid) {
                *valid = false;
            }
            return {};
        }
        LabelParserProfile profile;
        profile.id = object.value("id").toString();
        profile.displayName = object.value("displayName").toString();
        profile.enabled = object.value("enabled").toBool(true);
        profile.source = object.value("source").toString();
        for (const auto& suffix : object.value("suffixes").toArray()) {
            if (!suffix.isString()) {
                if (valid) {
                    *valid = false;
                }
                return {};
            }
            profile.suffixes.push_back(suffix.toString());
        }
        profiles.push_back(std::move(profile));
    }
    return profiles;
}

std::vector<LabelParserProfile> LabelsConfiguration::parserProfiles() const
{
    return readProfiles();
}

muse::Ret LabelsConfiguration::writeProfiles(const std::vector<LabelParserProfile>& profiles) const
{
    const QString path = profileFilePath();
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not create parser settings directory");
    }
    QJsonArray array;
    for (const auto& profile : profiles) {
        QJsonArray suffixes;
        for (const auto& suffix : profile.suffixes) {
            suffixes.push_back(suffix);
        }
        array.push_back(QJsonObject { { "id", profile.id }, { "displayName", profile.displayName },
                            { "enabled", profile.enabled }, { "source", profile.source },
                            { "suffixes", suffixes } });
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not open parser settings for writing");
    }
    const QByteArray payload = QJsonDocument(QJsonObject { { "profiles", array } }).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size() || !file.commit()) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not save parser settings");
    }
    return muse::make_ok();
}

muse::Ret LabelsConfiguration::saveParserProfile(LabelParserProfile profile, bool allowBuiltinOverride)
{
    profile.id = profile.id.trimmed();
    profile.displayName = profile.displayName.trimmed();
    if (profile.id.isEmpty() || profile.displayName.isEmpty() || profile.source.trimmed().isEmpty()) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser ID, name, and script are required");
    }
    if (profile.source.toUtf8().size() > 1024 * 1024) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser script exceeds 1 MiB limit");
    }
    static const QRegularExpression validSuffix("^[a-z0-9][a-z0-9_-]*$");
    QSet<QString> reserved {
        "aup", "aup3", "aup4", "aup4unsaved", "lof", "wav", "wave", "mp3", "flac", "ogg", "oga", "opus", "aif", "aiff",
        "aifc", "m4a", "aac", "wma", "mp4", "mov", "mid", "midi", "caf", "wv", "ape", "amr"
    };
    const auto fileTypes = Importer::Get().GetFileTypes(FileNames::FileType {});
    if (fileTypes.size() > 1) {
        for (const auto& extension : fileTypes[1].extensions) {
            QString value = QString::fromStdWString(extension.ToStdWstring()).toLower();
            if (value.startsWith('.')) {
                value.remove(0, 1);
            }
            if (!value.isEmpty() && value != "*") {
                reserved.insert(value);
            }
        }
    }
    QSet<QString> owned;
    QStringList normalized;
    for (QString suffix : profile.suffixes) {
        suffix = suffix.trimmed().toLower();
        if (suffix.startsWith('.')) {
            suffix.remove(0, 1);
        }
        if (!validSuffix.match(suffix).hasMatch() || reserved.contains(suffix)) {
            return muse::make_ret(muse::Ret::Code::BadArgs, "Invalid or reserved file suffix: " + suffix.toStdString());
        }
        if (!owned.contains(suffix)) {
            owned.insert(suffix);
            normalized.push_back(suffix);
        }
    }
    if (normalized.empty()) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "At least one file suffix is required");
    }
    profile.suffixes = normalized;

    bool valid = true;
    auto profiles = readProfiles(&valid);
    if (!valid) {
        return muse::make_ret(muse::Ret::Code::BadData, "Parser settings file is invalid");
    }
    auto existing = std::find_if(profiles.begin(), profiles.end(), [&](const auto& item) { return item.id == profile.id; });
    const bool alreadyOverridesTxt = existing != profiles.end() && existing->enabled && existing->suffixes.contains("txt");
    if (profile.enabled && owned.contains("txt") && !allowBuiltinOverride && !alreadyOverridesTxt) {
        return muse::make_ret(muse::Ret::Code::NotAllowed, "Overriding the built-in TXT parser requires confirmation");
    }
    if (profile.enabled) {
        for (const auto& other : profiles) {
            if (other.id == profile.id || !other.enabled) {
                continue;
            }
            for (const auto& suffix : other.suffixes) {
                if (owned.contains(suffix)) {
                    return muse::make_ret(muse::Ret::Code::BadArgs, "Suffix already belongs to an enabled parser: " + suffix.toStdString());
                }
            }
        }
    }
    if (existing == profiles.end()) {
        profiles.push_back(std::move(profile));
    } else {
        *existing = std::move(profile);
    }
    return writeProfiles(profiles);
}

muse::Ret LabelsConfiguration::removeParserProfile(const QString& id)
{
    bool valid = true;
    auto profiles = readProfiles(&valid);
    if (!valid) {
        return muse::make_ret(muse::Ret::Code::BadData, "Parser settings file is invalid");
    }
    const auto originalSize = profiles.size();
    std::erase_if(profiles, [&](const auto& profile) { return profile.id == id; });
    if (profiles.size() == originalSize) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser profile does not exist");
    }
    return writeProfiles(profiles);
}

muse::io::path_t LabelsConfiguration::labelsDirectoryPath() const
{
    return muse::settings()->value(LABELS_DIRECTORY_PATH).toString();
}

void LabelsConfiguration::setLabelsDirectoryPath(const muse::io::path_t& path)
{
    muse::settings()->setSharedValue(LABELS_DIRECTORY_PATH, muse::Val(path));
}
