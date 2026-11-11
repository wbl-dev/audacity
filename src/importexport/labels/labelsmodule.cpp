/*
* Audacity: A Digital Audio Editor
*/

#include "labelsmodule.h"

#include <QtQml/qqml.h>

#include "framework/global/modularity/ioc.h"
#include "framework/interactive/iinteractiveuriregister.h"
#include "framework/rcommand/icommandsregister.h"
#include "framework/rcommand/icommandsstate.h"

#include "internal/au3/au3labelsimporter.h"
#include "internal/au3/au3labelsexporter.h"

#include "internal/labelsconfiguration.h"
#include "internal/labelscommandsregister.h"
#include "internal/labelscommandscontroller.h"
#include "internal/labelscommandsstate.h"

#include "view/exportlabelsmodel.h"

using namespace au::importexport;

static const std::string mname("iex_labels");

static void labels_init_qrc()
{
    Q_INIT_RESOURCE(labels);
}

LabelsModule::LabelsModule() {}

std::string LabelsModule::moduleName() const
{
    return mname;
}

void LabelsModule::registerExports()
{
    m_configuration = std::make_shared<LabelsConfiguration>();

    globalIoc()->registerExport<ILabelsConfiguration>(mname, m_configuration);
}

void LabelsModule::registerResources()
{
    labels_init_qrc();
}

void LabelsModule::registerUiTypes()
{
    qmlRegisterType<ExportLabelsModel>("Audacity.Export", 1, 0, "ExportLabelsModel");
}

void LabelsModule::resolveImports()
{
    auto cr = globalIoc()->resolve<muse::rcommand::ICommandsRegister>(mname);
    if (cr) {
        cr->reg(std::make_shared<LabelsCommandsRegister>());
    }
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(muse::Uri("audacity://project/export/labels"), "Export/ExportLabelsDialog.qml");
    }
}

void LabelsModule::onInit(const muse::IApplication::RunMode&)
{
    m_configuration->init();
}

muse::modularity::IContextSetup* LabelsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new LabelsContext(ctx);
}

// =====================================================
// LabelsContext
// =====================================================

void LabelsContext::registerExports()
{
    m_commandsController = std::make_shared<LabelsCommandsController>(iocContext());
    ioc()->registerExport<ILabelsImporter>(mname, new Au3LabelsImporter(iocContext()));
    ioc()->registerExport<ILabelsExporter>(mname, new Au3LabelsExporter(iocContext()));
}

void LabelsContext::resolveImports()
{
    auto cs = ioc()->resolve<muse::rcommand::ICommandsState>(mname);
    if (cs) {
        cs->reg(std::make_shared<LabelsCommandsState>());
    }
}

void LabelsContext::onInit(const muse::IApplication::RunMode&)
{
    m_commandsController->init();
}

void LabelsContext::onDeinit()
{
}
