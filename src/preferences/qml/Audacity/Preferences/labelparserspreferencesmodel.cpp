#include "labelparserspreferencesmodel.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>
#include <QUuid>
#include <QStringDecoder>

#include "importexport/labels/internal/labelparserregistry.h"
#include "importexport/labels/internal/labelscriptengine.h"

using namespace au::appshell;
using namespace au::importexport;

LabelParsersPreferencesModel::LabelParsersPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

QVariantList LabelParsersPreferencesModel::profiles() const
{
    QVariantList items;
    LabelParserRegistry registry;
    if (const auto builtin = registry.parserForSuffix("txt"); builtin && builtin->id == "builtin-txt") {
        items.push_back(QVariantMap { { "id", builtin->id }, { "name", builtin->displayName },
                            { "suffixes", "txt" }, { "enabled", true }, { "script", builtin->source },
                            { "readOnly", true } });
    }
    for (const auto& profile : configuration()->parserProfiles()) {
        items.push_back(QVariantMap { { "id", profile.id }, { "name", profile.displayName },
                            { "suffixes", profile.suffixes.join(", ") }, { "enabled", profile.enabled },
                            { "script", profile.source }, { "readOnly", false } });
    }
    return items;
}

QVariantMap LabelParsersPreferencesModel::save(const QString& id, const QString& name, const QString& suffixes,
                                               bool enabled, const QString& script, bool allowBuiltinOverride)
{
    const QString profileId = id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id;
    LabelParserProfile profile { profileId, name, suffixes.split(',', Qt::SkipEmptyParts), enabled, script };
    const muse::Ret result = configuration()->saveParserProfile(std::move(profile), allowBuiltinOverride);
    return { { "ok", result.success() }, { "id", profileId }, { "error", QString::fromStdString(result.text()) },
        { "requiresOverride", result.code() == static_cast<int>(muse::Ret::Code::NotAllowed) } };
}

QVariantMap LabelParsersPreferencesModel::remove(const QString& id)
{
    const muse::Ret result = configuration()->removeParserProfile(id);
    return { { "ok", result.success() }, { "error", QString::fromStdString(result.text()) } };
}

QVariantMap LabelParsersPreferencesModel::test(const QString& script, const QString& samplePath) const
{
    QUrl url(samplePath);
    const QString path = url.isLocalFile() ? url.toLocalFile() : samplePath;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return { { "ok", false }, { "error", "Could not open sample file" } };
    }
    if (file.size() > 4 * 1024 * 1024) {
        return { { "ok", false }, { "error", "Sample file exceeds 4 MiB limit" } };
    }
    QStringDecoder decoder(QStringDecoder::Utf8);
    const QByteArray bytes = file.read(4 * 1024 * 1024 + 1);
    if (bytes.size() > 4 * 1024 * 1024 || !file.atEnd()) {
        return { { "ok", false }, { "error", "Sample file exceeds 4 MiB limit" } };
    }
    const QString text = decoder.decode(bytes);
    if (decoder.hasError()) {
        return { { "ok", false }, { "error", "Sample file is not UTF-8" } };
    }
    const QFileInfo info(path);
    const auto result = LabelScriptEngine().parse(script, { info.fileName(), info.suffix().toLower(), text });
    if (!result.ret.success()) {
        return { { "ok", false }, { "error", QString::fromStdString(result.ret.text()) } };
    }
    QVariantList preview;
    for (size_t i = 0; i < std::min<size_t>(result.val.labels.size(), 20); ++i) {
        const auto& label = result.val.labels[i];
        preview.push_back(QVariantMap { { "type", label.kind == ScriptLabelKind::Point ? "point" : "range" },
                              { "start", label.start }, { "end", label.end }, { "title", label.title } });
    }
    return { { "ok", true }, { "trackName", result.val.trackName }, { "count", static_cast<int>(result.val.labels.size()) },
        { "labels", preview } };
}
