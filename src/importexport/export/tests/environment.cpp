/*
* Audacity: A Digital Audio Editor
*/

#include "testing/environment.h"

#include "framework/global/settings.h"

#include "au3wrap/au3wrapmodule.h"

#include "project/tests/mocks/projectconfigurationmock.h"

#include "RegisterExportPlugins.h"
#include "RegisterImportPlugins.h"

using namespace ::testing;

static muse::testing::SuiteEnvironment exporter_se
    = muse::testing::SuiteEnvironment()
      .setDependencyModules({ new agu::au3::Au3WrapModule() })
      .setPreInit([](){
    std::shared_ptr<agu::project::ProjectConfigurationMock> projectConfigurator(new agu::project::ProjectConfigurationMock(),
                                                                               [](agu::project::ProjectConfigurationMock*){});

    ON_CALL(*projectConfigurator, temporaryDir())
    .WillByDefault(Return(""));

    muse::modularity::globalIoc()->registerExport<agu::project::IProjectConfiguration>("utests", projectConfigurator);

    //! NOTE: Settings persist between runs, e.g. the importer order, which
    //! is saved on first run. Start from defaults to be deterministic.
    constexpr auto keepDefaultSettings = true;
    muse::settings()->reset(keepDefaultSettings);

    //! NOTE: Au3WrapModule::onAllInited initializes the importer and exporter
    //! registries from what is registered by then
    RegisterExportPlugins();
    RegisterImportPlugins();
}).setPostInit([]() {
}).setDeInit([](){
    std::shared_ptr<agu::project::IProjectConfiguration> projectConfiguratorPtr
        = muse::modularity::globalIoc()->resolve<agu::project::IProjectConfiguration>("utests");
    muse::modularity::globalIoc()->unregister<agu::project::IProjectConfiguration>("utests");

    agu::project::IProjectConfiguration* projectConfigurator = projectConfiguratorPtr.get();
    delete projectConfigurator;
});
