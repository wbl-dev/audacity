/*
* Audacity: A Digital Audio Editor
*/

#include "testing/environment.h"

#include "au3wrap/au3wrapmodule.h"

#include "project/tests/mocks/projectconfigurationmock.h"
#include "mocks/playbackconfigurationmock.h"

using namespace ::testing;

static muse::testing::SuiteEnvironment playback_se
    = muse::testing::SuiteEnvironment()
      .setDependencyModules({ new agu::au3::Au3WrapModule() })
      .setPreInit([](){
    std::shared_ptr<agu::playback::PlaybackConfigurationMock> playbackConfigurator(new agu::playback::PlaybackConfigurationMock(),
                                                                                  [](agu::playback::PlaybackConfigurationMock*){});
    std::shared_ptr<agu::project::ProjectConfigurationMock> projectConfigurator(new agu::project::ProjectConfigurationMock(),
                                                                               [](agu::project::ProjectConfigurationMock*){});

    ON_CALL(*projectConfigurator, temporaryDir())
    .WillByDefault(Return(""));

    muse::modularity::globalIoc()->registerExport<agu::playback::IPlaybackConfiguration>("utests", playbackConfigurator);
    muse::modularity::globalIoc()->registerExport<agu::project::IProjectConfiguration>("utests", projectConfigurator);
}).setPostInit([]() {
}).setDeInit([](){
    std::shared_ptr<agu::playback::IPlaybackConfiguration> playbackConfiguratorPtr
        = muse::modularity::globalIoc()->resolve<agu::playback::IPlaybackConfiguration>("utests");
    muse::modularity::globalIoc()->unregister<agu::playback::IPlaybackConfiguration>("utests");

    std::shared_ptr<agu::project::IProjectConfiguration> projectConfiguratorPtr
        = muse::modularity::globalIoc()->resolve<agu::project::IProjectConfiguration>("utests");
    muse::modularity::globalIoc()->unregister<agu::project::IProjectConfiguration>("utests");

    agu::playback::IPlaybackConfiguration* playbackConfigurator = playbackConfiguratorPtr.get();
    agu::project::IProjectConfiguration* projectConfigurator = projectConfiguratorPtr.get();
    delete playbackConfigurator;
    delete projectConfigurator;
});
