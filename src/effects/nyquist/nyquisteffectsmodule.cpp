/*
* Audacity: A Digital Audio Editor
*/
#include "nyquisteffectsmodule.h"

#include "framework/audioplugins/iaudiopluginsscannerregister.h"
#include "framework/audioplugins/iaudiopluginmetareaderregister.h"

#include "effects/effects_base/iparameterextractorregistry.h"
#include "effects/effects_base/ieffectviewlaunchregister.h"
#include "effects/effects_base/ieffectloadersregister.h"

#include "internal/nyquisteffectsrepository.h"
#include "internal/nyquistparameterextractorservice.h"
#include "internal/nyquistviewlauncher.h"
#include "internal/nyquistpluginsscanner.h"
#include "internal/nyquistpluginsmetareader.h"
#include "internal/nyquisteffectsloader.h"

#include "nyquistprompt/nyquistpromptloader.h"
#include "nyquistprompt/nyquistpromptviewmodel.h"

namespace {
const std::string mname("effects_nyquist");
}

static void nyquist_init_qrc()
{
    Q_INIT_RESOURCE(nyquist);
}

agu::effects::NyquistEffectsModule::NyquistEffectsModule()
    : m_nyquistEffectsRepository(std::make_unique<NyquistEffectsRepository>())
{
}

agu::effects::NyquistEffectsModule::~NyquistEffectsModule() = default;

std::string agu::effects::NyquistEffectsModule::moduleName() const
{
    return mname;
}

void agu::effects::NyquistEffectsModule::registerExports()
{
}

void agu::effects::NyquistEffectsModule::registerResources()
{
    nyquist_init_qrc();
}

void agu::effects::NyquistEffectsModule::resolveImports()
{
    m_pluginsScanner = std::make_shared<NyquistPluginsScanner>();
    auto scannerRegister = globalIoc()->resolve<muse::audioplugins::IAudioPluginsScannerRegister>(mname);
    if (scannerRegister) {
        scannerRegister->registerScanner(m_pluginsScanner);
    }

    m_nyquistMetaReader = std::make_shared<NyquistPluginsMetaReader>();
    auto metaReaderRegister = globalIoc()->resolve<muse::audioplugins::IAudioPluginMetaReaderRegister>(moduleName());
    if (metaReaderRegister) {
        metaReaderRegister->registerReader(m_nyquistMetaReader);
    }

    m_effectLoader = std::make_shared<NyquistEffectsLoader>();
    auto loadersRegister = globalIoc()->resolve<IEffectLoadersRegister>(moduleName());
    if (loadersRegister) {
        loadersRegister->registerLoader(m_effectLoader);
    }
}

void agu::effects::NyquistEffectsModule::onPreInit(const muse::IApplication::RunMode& runMode)
{
    if (runMode != muse::IApplication::RunMode::AudioPluginRegistration) {
        m_nyquistPromptLoader = std::make_unique<NyquistPromptLoader>();
        m_nyquistPromptLoader->preInit();
    }
}

void agu::effects::NyquistEffectsModule::onInit(const muse::IApplication::RunMode& mode)
{
    m_nyquistMetaReader->init();
    m_effectLoader->init();
    m_pluginsScanner->init(mode);
    m_nyquistEffectsRepository->init();
}

void agu::effects::NyquistEffectsModule::onAllInited(const muse::IApplication::RunMode&)
{
    // onAllInited, because it depends on Au3Wrap and BuiltinEffects modules
    if (m_nyquistPromptLoader) {
        m_nyquistPromptLoader->init();
    }
}

void agu::effects::NyquistEffectsModule::onDeinit()
{
    m_nyquistMetaReader->deinit();
    m_effectLoader->deinit();
    m_pluginsScanner->deinit();
}

muse::modularity::IContextSetup* agu::effects::NyquistEffectsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new NyquistEffectsContext(ctx);
}

agu::effects::NyquistEffectsContext::NyquistEffectsContext(const muse::modularity::ContextPtr& ctx)
    : muse::modularity::IContextSetup(ctx)
{
}

void agu::effects::NyquistEffectsContext::resolveImports()
{
    auto paramExtractorRegistry = muse::modularity::globalIoc()->resolve<IParameterExtractorRegistry>(mname);
    if (paramExtractorRegistry) {
        paramExtractorRegistry->registerExtractor(std::make_shared<NyquistParameterExtractorService>());
    }

    auto launchRegister = ioc()->resolve<IEffectViewLaunchRegister>(mname);
    if (launchRegister) {
        launchRegister->regLauncher(EffectFamily::Nyquist, std::make_shared<NyquistViewLauncher>(iocContext()));
    }
}

void agu::effects::NyquistEffectsContext::onPreInit(const muse::IApplication::RunMode&)
{
}

void agu::effects::NyquistEffectsContext::onInit(const muse::IApplication::RunMode&)
{
}

void agu::effects::NyquistEffectsContext::onDeinit()
{
}
