/*
* Audacity: A Digital Audio Editor
*/
#include "audiouniteffectsmodule.h"

#include "framework/audioplugins/iaudiopluginsscannerregister.h"
#include "framework/audioplugins/iaudiopluginmetareaderregister.h"

#include "effects/effects_base/ieffectloadersregister.h"
#include "effects/effects_base/ieffectviewlaunchregister.h"

#include "internal/audiouniteffectloader.h"
#include "internal/audiounitpluginsscanner.h"
#include "internal/audiounitpluginsmetareader.h"
#include "internal/audiounitviewlauncher.h"

static const std::string mname("effects_audiounit");

agu::effects::AudioUnitEffectsModule::AudioUnitEffectsModule()
    : m_metaReader(std::make_shared<AudioUnitPluginsMetaReader>()), m_effectLoader(std::make_shared<AudioUnitEffectLoader>()),
    m_pluginsScanner(std::make_shared<AudioUnitPluginsScanner>())
{
}

std::string agu::effects::AudioUnitEffectsModule::moduleName() const
{
    return mname;
}

void agu::effects::AudioUnitEffectsModule::registerExports()
{
}

void agu::effects::AudioUnitEffectsModule::resolveImports()
{
    auto scannerRegister = globalIoc()->resolve<muse::audioplugins::IAudioPluginsScannerRegister>(mname);
    if (scannerRegister) {
        scannerRegister->registerScanner(m_pluginsScanner);
    }

    auto metaReaderRegister = globalIoc()->resolve<muse::audioplugins::IAudioPluginMetaReaderRegister>(mname);
    if (metaReaderRegister) {
        metaReaderRegister->registerReader(m_metaReader);
    }

    auto loadersRegister = globalIoc()->resolve<IEffectLoadersRegister>(mname);
    if (loadersRegister) {
        loadersRegister->registerLoader(m_effectLoader);
    }
}

void agu::effects::AudioUnitEffectsModule::onInit(const muse::IApplication::RunMode& mode)
{
    m_metaReader->init();
    m_effectLoader->init();
    m_pluginsScanner->init(mode);
}

void agu::effects::AudioUnitEffectsModule::onDeinit()
{
    m_effectLoader->deinit();
    m_pluginsScanner->deinit();
    m_metaReader->deinit();
}

muse::modularity::IContextSetup* agu::effects::AudioUnitEffectsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AudioUnitEffectsContext(ctx);
}

// =====================================================
// AudioUnitEffectsContext
// =====================================================

void agu::effects::AudioUnitEffectsContext::registerExports()
{
}

void agu::effects::AudioUnitEffectsContext::resolveImports()
{
    auto launchRegister = ioc()->resolve<IEffectViewLaunchRegister>(mname);
    if (launchRegister) {
        launchRegister->regLauncher(EffectFamily::AudioUnit, std::make_shared<AudioUnitViewLauncher>(iocContext()));
    }
}

void agu::effects::AudioUnitEffectsContext::onDeinit()
{
}
