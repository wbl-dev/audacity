#include "labelparserrescan.h"

#include <algorithm>

#include "labelparserregistry.h"

namespace au::importexport {
namespace {
QStringList supportedSuffixes(ILabelsConfiguration& configuration)
{
    QStringList suffixes;
    for (const auto& suffix : LabelParserRegistry(&configuration).supportedExtensions()) {
        suffixes.push_back(QString::fromStdString(suffix));
    }
    suffixes.removeDuplicates();
    std::sort(suffixes.begin(), suffixes.end());
    return suffixes;
}
}

LabelParserRescanResult rescanLabelParsers(ILabelsConfiguration& configuration)
{
    const QStringList before = supportedSuffixes(configuration);
    muse::Ret ret = configuration.reloadParserProfiles();
    const QStringList supported = supportedSuffixes(configuration);
    QStringList added;
    QStringList removed;
    for (const auto& suffix : supported) {
        if (!before.contains(suffix)) {
            added.push_back(suffix);
        }
    }
    for (const auto& suffix : before) {
        if (!supported.contains(suffix)) {
            removed.push_back(suffix);
        }
    }
    return { std::move(ret), supported, added, removed };
}
}
