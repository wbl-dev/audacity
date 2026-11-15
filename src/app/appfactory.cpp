/*
 * Audacity: A Digital Audio Editor
 */
#include "appfactory.h"

#include "guiapp.h"
#include "pluginregistrationapp.h"

#include "muse_framework_config.h"
#include "app_config.h"

#include "framework/diagnostics/diagnosticsmodule.h"
#include "framework/draw/drawmodule.h"
#include "framework/actions/actionsmodule.h"
#include "framework/rcommand/rcommandmodule.h"
#include "framework/audioplugins/audiopluginsmodule.h"
#include "framework/interactive/interactivemodule.h"
#include "framework/ui/uimodule.h"
#include "framework/shortcuts/shortcutsmodule.h"
#include "framework/accessibility/accessibilitymodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#include "framework/dockwindow/dockmodule.h"
#include "framework/toast/toastmodule.h"
#include "framework/languages/languagesmodule.h"
#include "framework/workspace/workspacemodule.h"

#include "framework/multiwindows/multiwindowsmodule.h"

#include "appshell/appshellmodule.h"
#include "context/contextmodule.h"
#include "preferences/preferencesmodule.h"
#include "project/projectmodule.h"
#include "projectscene/projectscenemodule.h"
#include "audio/audiomodule.h"
#include "au3audio/au3audiomodule.h"
#include "playback/playbackmodule.h"
#include "trackedit/trackeditmodule.h"
#include "macromanager/macromanagermodule.h"
#include "spectrogram/spectrogrammodule.h"
#include "record/recordmodule.h"
#include "uicomponents/uicomponentsmodule.h"
#include "effects/effects_base/effectsmodule.h"
#include "effects/builtin/builtineffectsmodule.h"
#include "effects/builtin_collection/builtineffectscollectionmodule.h"
#include "importexport/import/importermodule.h"
#include "importexport/export/exportermodule.h"
#include "importexport/labels/labelsmodule.h"
#include "automation/automationmodule.h"

#if AU_MODULE_EFFECTS_NYQUIST
#include "effects/nyquist/nyquisteffectsmodule.h"
#endif

#ifdef AU_MODULE_EFFECTS_LV2
#include "effects/lv2/lv2effectsmodule.h"
#else
#include "stubs/lv2/lv2effectsstubmodule.h"
#endif

#ifdef AU_MODULE_EFFECTS_VST
#include "effects/vst/vsteffectsmodule.h"
#else
#include "stubs/vst/vsteffectsstubmodule.h"
#endif

#ifdef AU_MODULE_EFFECTS_AUDIO_UNIT
#include "effects/audio_unit/audiouniteffectsmodule.h"
#else
#include "stubs/audio_unit/audiouniteffectsstubmodule.h"
#endif

#ifdef MUSE_MODULE_TESTFLOW
#include "testflow/testflowmodule.h"
#endif

#ifdef MUSE_MODULE_EXTENSIONS
#include "framework/extensions/extensionsmodule.h"
#include "extensions/aguextensionsmodule.h"
#include "effects/extensions/extensioneffectsmodule.h"
#else
#include "framework/stubs/extensions/extensionsstubmodule.h"
#include "stubs/extensions/aguextensionsstubmodule.h"
#include "stubs/effects_extensions/extensioneffectsstubmodule.h"
#endif

#include "au3wrap/au3wrapmodule.h"

using namespace muse;
using namespace agu::app;

std::shared_ptr<muse::IApplication> AppFactory::newApp(const std::shared_ptr<AGuCmdOptions>& options) const
{
    IF_ASSERT_FAILED(options) {
        return nullptr;
    }

    switch (options->runMode) {
    case IApplication::RunMode::GuiApp:
    case IApplication::RunMode::ConsoleApp:
        // No console mode for now
        return newGuiApp(options);
    case IApplication::RunMode::AudioPluginRegistration:
        return newPluginRegistrationApp(options);
    }

    return newGuiApp(options);
}

std::shared_ptr<muse::IApplication> AppFactory::newGuiApp(const std::shared_ptr<AGuCmdOptions>& options) const
{
    std::shared_ptr<GuiApp> app = std::make_shared<GuiApp>(options);

    //! NOTE `diagnostics` must be first, because it installs the crash handler.
    //! For other modules, the order is (and should be) unimportant.
    app->addModule(new muse::diagnostics::DiagnosticsModule());

    app->addModule(new muse::audioplugins::AudioPluginsModule());
    app->addModule(new muse::actions::ActionsModule());
    app->addModule(new muse::rcommand::RCommandModule());
    app->addModule(new muse::draw::DrawModule());
    app->addModule(new muse::workspace::WorkspaceModule());
    app->addModule(new muse::accessibility::AccessibilityModule());
    app->addModule(new muse::interactive::InteractiveModule());
    app->addModule(new muse::mi::MultiWindowsModule());
    app->addModule(new muse::languages::LanguagesModule());
    app->addModule(new muse::ui::UiModule());
    app->addModule(new muse::uicomponents::UiComponentsModule());
    app->addModule(new muse::dock::DockModule());
    app->addModule(new muse::toast::ToastModule());
#ifdef MUSE_MODULE_SHORTCUTS
    app->addModule(new muse::shortcuts::ShortcutsModule());
#endif
    app->addModule(new muse::extensions::ExtensionsModule());
#ifdef MUSE_MODULE_TESTFLOW
    app->addModule(new muse::testflow::TestflowModule());
#endif

    // Audacity modules
    app->addModule(new agu::appshell::AppShellModule());
    app->addModule(new agu::preferences::PreferencesModule());
    app->addModule(new agu::uicomponents::UiComponentsModule());
    app->addModule(new agu::extensions::AGuExtensionsModule());
    app->addModule(new agu::effects::AudioUnitEffectsModule());
    app->addModule(new agu::effects::Lv2EffectsModule());
    app->addModule(new agu::effects::VstEffectsModule());
    app->addModule(new agu::effects::NyquistEffectsModule());
    app->addModule(new agu::effects::extensions::ExtensionEffectsModule());
    app->addModule(new agu::context::ContextModule());
    app->addModule(new agu::audio::AudioModule());
    app->addModule(new agu::au3audio::Au3AudioModule());
    app->addModule(new agu::projectscene::ProjectSceneModule());
    app->addModule(new agu::playback::PlaybackModule());
    app->addModule(new agu::record::RecordModule());
    app->addModule(new agu::trackedit::TrackeditModule());
    app->addModule(new agu::macromanager::MacroManagerModule());
    app->addModule(new agu::spectrogram::SpectrogramModule());
    app->addModule(new agu::project::ProjectModule());
    app->addModule(new agu::importexport::ExporterModule());
    app->addModule(new agu::importexport::ImporterModule());
    app->addModule(new agu::importexport::LabelsModule());
    app->addModule(new agu::au3::Au3WrapModule());
    app->addModule(new agu::effects::EffectsModule());
    app->addModule(new agu::effects::BuiltinEffectsModule());
    app->addModule(new agu::effects::BuiltinEffectsCollectionModule());
    app->addModule(new agu::automation::AutomationModule());

    return app;
}

std::shared_ptr<muse::IApplication> AppFactory::newPluginRegistrationApp(const std::shared_ptr<AGuCmdOptions>& options) const
{
    std::shared_ptr<PluginRegistrationApp> app = std::make_shared<PluginRegistrationApp>(options);

    //! NOTE `diagnostics` must be first, because it installs the crash handler. Without it
    //! a crashing validation only produces a dump on macOS, where the child happens to
    //! inherit the parent's exception port, and that dump then carries the parent's
    //! annotations.
    app->addModule(new muse::diagnostics::DiagnosticsModule());
    app->addModule(new muse::rcommand::RCommandModule()); // needed by diagnostics module

    app->addModule(new muse::audioplugins::AudioPluginsModule());
    app->addModule(new muse::actions::ActionsModule());

    app->addModule(new agu::effects::AudioUnitEffectsModule());
    app->addModule(new agu::effects::Lv2EffectsModule());
    app->addModule(new agu::effects::VstEffectsModule());
    app->addModule(new agu::effects::NyquistEffectsModule());

    return app;
}
