#include "labelparserregistry.h"

#include <algorithm>

#include <QFile>

static void initializeLabelResources()
{
    Q_INIT_RESOURCE(labels);
}

namespace au::importexport {
std::optional<LabelParserProfile> LabelParserRegistry::parserForSuffix(QString suffix) const
{
    suffix = suffix.trimmed().toLower();
    if (suffix.startsWith('.')) {
        suffix.remove(0, 1);
    }
    ILabelsConfiguration* settings = m_configuration ? m_configuration : configuration().get();
    if (settings) {
        for (const auto& profile : settings->parserProfiles()) {
            if (profile.enabled && profile.suffixes.contains(suffix)) {
                return profile;
            }
        }
    }
    if (suffix != "txt") {
        return std::nullopt;
    }
    initializeLabelResources();
    QFile resource(":/resources/audacity-txt.js");
    if (!resource.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }
    return LabelParserProfile { "builtin-txt", "Audacity TXT", { "txt" }, true, QString::fromUtf8(resource.readAll()) };
}

std::vector<std::string> LabelParserRegistry::supportedExtensions() const
{
    std::vector<std::string> extensions { "txt" };
    ILabelsConfiguration* settings = m_configuration ? m_configuration : configuration().get();
    if (settings) {
        for (const auto& profile : settings->parserProfiles()) {
            if (!profile.enabled) {
                continue;
            }
            for (const auto& suffix : profile.suffixes) {
                const std::string value = suffix.toStdString();
                if (std::find(extensions.begin(), extensions.end(), value) == extensions.end()) {
                    extensions.push_back(value);
                }
            }
        }
    }
    return extensions;
}
}
