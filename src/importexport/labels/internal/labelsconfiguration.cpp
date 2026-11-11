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
#include <QStringDecoder>

using namespace au::importexport;

static const std::string module_name("iex_labels");

static const muse::Settings::Key LABELS_DIRECTORY_PATH(module_name, "importexport/labelsDirectoryPath");
static constexpr auto metadataPrefix = "// @onlyaudios-label-parser ";
static constexpr qint64 maxScriptBytes = 1024 * 1024;

static bool validSuffix(const QString& suffix)
{
    static const QRegularExpression pattern("^[a-z0-9][a-z0-9_-]*$");
    if (!pattern.match(suffix).hasMatch()) {
        return false;
    }
    QSet<QString> reserved {
        "aup", "aup3", "aup4", "aup4unsaved", "lof", "wav", "wave", "mp3", "flac", "ogg", "oga", "opus",
        "aif", "aiff", "aifc", "m4a", "aac", "wma", "mp4", "mov", "mid", "midi", "caf", "wv", "ape", "amr"
    };
    const auto fileTypes = Importer::Get().GetFileTypes(FileNames::FileType {});
    if (fileTypes.size() > 1) {
        for (const auto& extension : fileTypes[1].extensions) {
            QString value = QString::fromStdWString(extension.ToStdWstring()).toLower();
            if (value.startsWith('.')) {
                value.remove(0, 1);
            }
            reserved.insert(value);
        }
    }
    return !reserved.contains(suffix);
}

static QStringList normalizeSuffixes(const QStringList& suffixes)
{
    QStringList normalized;
    for (QString suffix : suffixes) {
        suffix = suffix.trimmed().toLower();
        if (suffix.startsWith('.')) {
            suffix.remove(0, 1);
        }
        if (!validSuffix(suffix)) {
            return {};
        }
        if (!normalized.contains(suffix)) {
            normalized.push_back(suffix);
        }
    }
    return normalized;
}

LabelsConfiguration::LabelsConfiguration(QString profileFilePath)
    : m_profileFilePath(std::move(profileFilePath))
{
}

void LabelsConfiguration::init()
{
    muse::settings()->setDefaultValue(LABELS_DIRECTORY_PATH, muse::Val(globalConfiguration()->userDataPath()));
    reloadParserProfiles();
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

QString LabelsConfiguration::parserScriptsDirectoryPath() const
{
    return QFileInfo(profileFilePath()).absolutePath() + "/LabelParsers";
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
    if (!m_profilesLoaded) {
        const_cast<LabelsConfiguration*>(this)->reloadParserProfiles();
    }
    return m_profiles;
}

muse::Ret LabelsConfiguration::writeProfileScript(const QString& path, const LabelParserProfile& profile) const
{
    QJsonArray suffixes;
    for (const auto& suffix : profile.suffixes) {
        suffixes.push_back(suffix);
    }
    const QJsonObject metadata { { "id", profile.id }, { "displayName", profile.displayName },
                                 { "enabled", profile.enabled }, { "suffixes", suffixes } };
    const QByteArray payload = QByteArray(metadataPrefix) + QJsonDocument(metadata).toJson(QJsonDocument::Compact)
                               + '\n' + profile.source.toUtf8();
    if (payload.size() > maxScriptBytes) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser script exceeds 1 MiB limit");
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not open parser script for writing");
    }
    if (file.write(payload) != payload.size() || !file.commit()) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not save parser script");
    }
    return muse::make_ok();
}

muse::Ret LabelsConfiguration::migrateLegacyProfiles(const QString& directoryPath)
{
    const QString marker = directoryPath + "/.legacy-json-migrated";
    if (QFile::exists(marker) || !QFile::exists(profileFilePath())) {
        return muse::make_ok();
    }
    bool valid = false;
    const auto legacy = readProfiles(&valid);
    if (!valid) {
        return muse::make_ret(muse::Ret::Code::BadData, "Legacy parser settings file is invalid");
    }
    for (auto profile : legacy) {
        if (!profile.enabled || profile.source.trimmed().isEmpty() || profile.source.toUtf8().size() > maxScriptBytes) {
            continue;
        }
        profile.suffixes = normalizeSuffixes(profile.suffixes);
        if (profile.suffixes.empty()) {
            continue;
        }
        const QString target = directoryPath + "/" + profile.suffixes.front() + ".js";
        if (!QFile::exists(target)) {
            const auto result = writeProfileScript(target, profile);
            if (!result.success()) {
                return result;
            }
        }
    }
    QFile done(marker);
    if (!done.open(QIODevice::WriteOnly) || done.write("migrated\n") < 0) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not mark parser migration complete");
    }
    return muse::make_ok();
}

muse::Ret LabelsConfiguration::reloadParserProfiles()
{
    const QString directoryPath = parserScriptsDirectoryPath();
    if (!QDir().mkpath(directoryPath)) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not create parser scripts directory");
    }
    const auto migration = migrateLegacyProfiles(directoryPath);
    std::vector<LabelParserProfile> profiles;
    QSet<QString> claimed;
    QSet<QString> profileIds;
    QStringList invalidFiles;
    const auto files = QDir(directoryPath).entryInfoList({ "*.js" }, QDir::Files | QDir::Readable, QDir::Name);
    for (const auto& file : files) {
        const QString primarySuffix = file.completeBaseName();
        if (file.isSymLink() || primarySuffix != primarySuffix.toLower() || !validSuffix(primarySuffix)
            || file.size() > maxScriptBytes) {
            invalidFiles.push_back(file.fileName());
            continue;
        }
        QFile script(file.absoluteFilePath());
        if (!script.open(QIODevice::ReadOnly)) {
            invalidFiles.push_back(file.fileName());
            continue;
        }
        const QByteArray bytes = script.read(maxScriptBytes + 1);
        QStringDecoder decoder(QStringDecoder::Utf8);
        const QString source = decoder.decode(bytes);
        if (bytes.size() > maxScriptBytes || !script.atEnd() || decoder.hasError() || source.trimmed().isEmpty()) {
            invalidFiles.push_back(file.fileName());
            continue;
        }
        LabelParserProfile profile { primarySuffix, primarySuffix.toUpper(), { primarySuffix }, true, source,
                                     file.absoluteFilePath() };
        if (source.startsWith(metadataPrefix)) {
            const QString line = source.section('\n', 0, 0);
            QJsonParseError error;
            const auto document = QJsonDocument::fromJson(line.mid(QString(metadataPrefix).size()).toUtf8(), &error);
            if (!document.isObject()) {
                invalidFiles.push_back(file.fileName());
                continue;
            }
            const auto object = document.object();
            if (!object.value("id").isString() || !object.value("displayName").isString()
                || !object.value("suffixes").isArray()) {
                invalidFiles.push_back(file.fileName());
                continue;
            }
            profile.id = object.value("id").toString();
            profile.displayName = object.value("displayName").toString();
            profile.enabled = object.value("enabled").toBool(true);
            profile.suffixes.clear();
            for (const auto& value : object.value("suffixes").toArray()) {
                if (!value.isString()) {
                    profile.suffixes.clear();
                    break;
                }
                profile.suffixes.push_back(value.toString());
            }
            profile.suffixes = normalizeSuffixes(profile.suffixes);
            if (profile.id.trimmed().isEmpty() || profile.displayName.trimmed().isEmpty()
                || profile.suffixes.empty() || profile.suffixes.front() != primarySuffix) {
                invalidFiles.push_back(file.fileName());
                continue;
            }
            profile.source = source.mid(line.length() + 1);
            if (profile.source.trimmed().isEmpty()) {
                invalidFiles.push_back(file.fileName());
                continue;
            }
        }
        bool conflict = false;
        if (profile.enabled) {
            for (const auto& suffix : profile.suffixes) {
                conflict |= claimed.contains(suffix);
            }
        }
        if (conflict || profileIds.contains(profile.id)) {
            invalidFiles.push_back(file.fileName());
            continue;
        }
        if (profile.enabled) {
            for (const auto& suffix : profile.suffixes) {
                claimed.insert(suffix);
            }
        }
        profileIds.insert(profile.id);
        profiles.push_back(std::move(profile));
    }
    m_profiles = std::move(profiles);
    m_profilesLoaded = true;
    if (!invalidFiles.empty()) {
        return muse::make_ret(muse::Ret::Code::BadData,
                              "Skipped invalid parser scripts: " + invalidFiles.join(", ").toStdString());
    }
    return migration;
}

muse::Ret LabelsConfiguration::saveParserProfile(LabelParserProfile profile, bool allowBuiltinOverride)
{
    profile.id = profile.id.trimmed();
    profile.displayName = profile.displayName.trimmed();
    if (profile.id.isEmpty() || profile.displayName.isEmpty() || profile.source.trimmed().isEmpty()) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser ID, name, and script are required");
    }
    if (profile.source.toUtf8().size() > maxScriptBytes) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser script exceeds 1 MiB limit");
    }
    profile.suffixes = normalizeSuffixes(profile.suffixes);
    if (profile.suffixes.empty()) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Invalid or reserved file suffix");
    }
    const auto profiles = parserProfiles();
    auto existing = std::find_if(profiles.begin(), profiles.end(), [&](const auto& item) { return item.id == profile.id; });
    const bool alreadyOverridesTxt = existing != profiles.end() && existing->enabled && existing->suffixes.contains("txt");
    if (profile.enabled && profile.suffixes.contains("txt") && !allowBuiltinOverride && !alreadyOverridesTxt) {
        return muse::make_ret(muse::Ret::Code::NotAllowed, "Overriding the built-in TXT parser requires confirmation");
    }
    if (profile.enabled) {
        for (const auto& other : profiles) {
            if (other.id == profile.id || !other.enabled) {
                continue;
            }
            for (const auto& suffix : other.suffixes) {
                if (profile.suffixes.contains(suffix)) {
                    return muse::make_ret(muse::Ret::Code::BadArgs, "Suffix already belongs to an enabled parser: " + suffix.toStdString());
                }
            }
        }
    }
    const QString directoryPath = parserScriptsDirectoryPath();
    if (!QDir().mkpath(directoryPath)) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not create parser scripts directory");
    }
    const QString path = directoryPath + "/" + profile.suffixes.front() + ".js";
    if (QFile::exists(path) && (existing == profiles.end() || existing->sourcePath != path)) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser script file already exists");
    }
    const auto saved = writeProfileScript(path, profile);
    if (!saved.success()) {
        return saved;
    }
    if (existing != profiles.end() && existing->sourcePath != path) {
        QFile::remove(existing->sourcePath);
    }
    return reloadParserProfiles();
}

muse::Ret LabelsConfiguration::removeParserProfile(const QString& id)
{
    const auto profiles = parserProfiles();
    const auto existing = std::find_if(profiles.begin(), profiles.end(), [&](const auto& item) { return item.id == id; });
    if (existing == profiles.end()) {
        return muse::make_ret(muse::Ret::Code::BadArgs, "Parser profile does not exist");
    }
    if (!QFile::remove(existing->sourcePath)) {
        return muse::make_ret(muse::Ret::Code::InternalError, "Could not remove parser script");
    }
    return reloadParserProfiles();
}

muse::io::path_t LabelsConfiguration::labelsDirectoryPath() const
{
    return muse::settings()->value(LABELS_DIRECTORY_PATH).toString();
}

void LabelsConfiguration::setLabelsDirectoryPath(const muse::io::path_t& path)
{
    muse::settings()->setSharedValue(LABELS_DIRECTORY_PATH, muse::Val(path));
}
