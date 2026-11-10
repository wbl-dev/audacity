/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "../../ilabelsimporter.h"
#include "../labelscriptengine.h"
#include "../labelparserregistry.h"

namespace au::importexport {
class Au3LabelsImporter : public ILabelsImporter, public muse::Contextable
{
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    LabelScriptEngine m_engine;
    LabelParserRegistry m_registry;

public:
    Au3LabelsImporter(const muse::modularity::ContextPtr& ctx, QString workerExecutable = {})
        : muse::Contextable(ctx), m_engine(std::move(workerExecutable)) {}

    muse::Ret importData(const muse::io::path_t& filePath) override;
    std::vector<std::string> supportedExtensions() const override;
};
}
