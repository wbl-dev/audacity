#pragma once

#include <optional>
#include <string>
#include <vector>

#include <QString>
#include "../labelparserprofile.h"
#include "../ilabelsconfiguration.h"
#include "modularity/ioc.h"

namespace au::importexport {
class LabelParserRegistry
{
    muse::GlobalInject<ILabelsConfiguration> configuration;
    ILabelsConfiguration* m_configuration = nullptr;

public:
    explicit LabelParserRegistry(ILabelsConfiguration* configuration = nullptr)
        : m_configuration(configuration) {}
    std::optional<LabelParserProfile> parserForSuffix(QString suffix) const;
    std::vector<std::string> supportedExtensions() const;
};
}
