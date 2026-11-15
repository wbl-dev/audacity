/*
 * Audacity: A Digital Audio Editor
 */
#include "aguextensionsmodule.h"

#include "framework/global/api/iapiregister.h"

#include "api/audiotransformapi.h"
#include "api/nativeapi.h"
#include "api/preferencesapi.h"
#include "extensionpreferences.h"

namespace agu::extensions {
namespace {
const std::string mname = "agu_extensions";
}

std::string AGuExtensionsModule::moduleName() const
{
    return mname;
}

void AGuExtensionsModule::registerApi()
{
    auto registry = muse::modularity::globalIoc()->resolve<muse::api::IApiRegister>(mname);
    if (registry) {
        registry->regApiCreator(mname, "MuseApi.Native", new muse::api::ApiCreator<NativeApi>());
        registry->regApiCreator(mname, "AGu.AudioTransform", new muse::api::ApiCreator<AudioTransformApi>());
        registry->regApiCreator(mname, "AGu.Preferences", new muse::api::ApiCreator<PreferencesApi>());
    }
}

muse::modularity::IContextSetup* AGuExtensionsModule::newContext(const muse::modularity::ContextPtr& context) const
{
    return new AGuExtensionsContext(context);
}

AGuExtensionsContext::AGuExtensionsContext(const muse::modularity::ContextPtr& context)
    : IContextSetup(context)
{
}

void AGuExtensionsContext::onInit(const muse::IApplication::RunMode&)
{
    m_extensions = ioc()->resolve<muse::extensions::IExtensionsProvider>(mname);
    enableExtensionsByDefault();
    registerPreferenceDefaults();
    if (m_extensions) {
        m_extensions->manifestListChanged().onNotify(this, [this] {
            enableExtensionsByDefault();
            registerPreferenceDefaults();
        });
    }
}

void AGuExtensionsContext::enableExtensionsByDefault() const
{
    if (!m_extensions) {
        return;
    }
    for (const auto& manifest : m_extensions->manifestList()) {
        m_extensions->setEnabled(manifest.uri, true);
    }
}

void AGuExtensionsContext::registerPreferenceDefaults() const
{
    if (!m_extensions) {
        return;
    }
    for (const auto& manifest : m_extensions->manifestList()) {
        registerExtensionPreferenceDefaults(manifest);
    }
}
} // namespace agu::extensions
