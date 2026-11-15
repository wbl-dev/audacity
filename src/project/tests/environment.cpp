/*
* Audacity: A Digital Audio Editor
*/

#include "testing/environment.h"
#include "testing/testcontext.h"

#include "trackedit/itrackeditproject.h"
#include "au3wrap/au3wrapmodule.h"

#include "projectscene/tests/mocks/projectsceneconfigurationmock.h"

#include "project/tests/mocks/projectconfigurationmock.h"
#include "project/tests/mocks/trackeditprojectcreatormock.h"
#include "project/tests/mocks/projectviewstatecreatormock.h"
#include "trackedit/tests/mocks/clipboardmock.h"

namespace agu::project {
static auto s_testCtx = agu::testutils::makeTestContext();
static muse::testing::SuiteEnvironment aguproject_se
    = muse::testing::SuiteEnvironment()
      .setDependencyModules({ new agu::au3::Au3WrapModule()
                            })
      .setPreInit([](){
    std::shared_ptr<ProjectConfigurationMock> projectConfigurator(new ProjectConfigurationMock(), [](ProjectConfigurationMock*){});

    ON_CALL(*projectConfigurator, temporaryDir())
    .WillByDefault(::testing::Return(""));

    muse::modularity::globalIoc()->registerExport<IProjectConfiguration>("utests", projectConfigurator);

    std::shared_ptr<TrackeditProjectCreatorMock> trackeditProjectCreatorMock(
        new TrackeditProjectCreatorMock(),
        [](TrackeditProjectCreatorMock*) {}
        );

    ON_CALL(*trackeditProjectCreatorMock, create(::testing::_))
    .WillByDefault(::testing::Return(agu::trackedit::ITrackeditProjectPtr {}));

    muse::modularity::globalIoc()->registerExport<agu::trackedit::ITrackeditProjectCreator>(
        "utests",
        trackeditProjectCreatorMock
        );

    std::shared_ptr<agu::projectscene::ProjectViewStateCreatorMock> projectViewStateCreatorMock(
        new agu::projectscene::ProjectViewStateCreatorMock(),
        [](agu::projectscene::ProjectViewStateCreatorMock*) {}
        );

    ON_CALL(*projectViewStateCreatorMock, createViewState(::testing::_))
    .WillByDefault(::testing::Return(agu::projectscene::IProjectViewStatePtr {}));

    muse::modularity::globalIoc()->registerExport<agu::projectscene::IProjectViewStateCreator>(
        "utests",
        projectViewStateCreatorMock
        );

    std::shared_ptr<::testing::NiceMock<agu::projectscene::ProjectSceneConfigurationMock> > projectSceneConfigurator(new ::testing::NiceMock<agu::projectscene::ProjectSceneConfigurationMock>(),
                                                                                                                    [](agu::projectscene::
                                                                                                                       ProjectSceneConfigurationMock
                                                                                                                       *)
    {
    });

    static std::vector<agu::projectscene::ClipColorInfo> colorInfos = {
        { muse::TranslatableString::untranslatable("blue"), 1 },
        { muse::TranslatableString::untranslatable("red"), 2 },
    };

    ON_CALL(*projectSceneConfigurator, clipColorInfos())
    .WillByDefault(::testing::ReturnRef(colorInfos));
    ON_CALL(*projectSceneConfigurator, clipColor(1))
    .WillByDefault(::testing::Return(muse::Color(0x00, 0x00, 0xFF)));
    ON_CALL(*projectSceneConfigurator, clipColor(2))
    .WillByDefault(::testing::Return(muse::Color(0xFF, 0x00, 0x00)));
    ON_CALL(*projectSceneConfigurator, clipSelectedColor(1))
    .WillByDefault(::testing::Return(muse::Color(0x80, 0x80, 0xFF)));
    ON_CALL(*projectSceneConfigurator, clipSelectedColor(2))
    .WillByDefault(::testing::Return(muse::Color(0xFF, 0x80, 0x80)));

    muse::modularity::globalIoc()->unregister<agu::projectscene::IProjectSceneConfiguration>("utests");
    muse::modularity::globalIoc()->registerExport<agu::projectscene::IProjectSceneConfiguration>("utests", projectSceneConfigurator);

    std::shared_ptr<agu::trackedit::ClipboardMock> clipboardMock(
        new agu::trackedit::ClipboardMock(),
        [](agu::trackedit::ClipboardMock*) {}
        );

    muse::modularity::ioc(s_testCtx)->registerExport<agu::trackedit::ITrackeditClipboard>(
        "utests",
        clipboardMock
        );
}).setPostInit([]() {
}).setDeInit([]()
{
    // Unregister and delete ProjectConfiguration
    {
        std::shared_ptr<IProjectConfiguration> projectConfiguratorPtr
            =muse::modularity::globalIoc()->resolve<IProjectConfiguration>("utests");
        muse::modularity::globalIoc()->unregister<IProjectConfiguration>("utests");

        if (projectConfiguratorPtr) {
            delete projectConfiguratorPtr.get();
        }
    }

    // Unregister and delete TrackeditProjectCreator
    {
        std::shared_ptr<agu::trackedit::ITrackeditProjectCreator> trackeditProjectCreatorPtr
            =muse::modularity::globalIoc()->resolve<agu::trackedit::ITrackeditProjectCreator>("utests");
        muse::modularity::globalIoc()->unregister<agu::trackedit::ITrackeditProjectCreator>("utests");

        if (trackeditProjectCreatorPtr) {
            delete static_cast<TrackeditProjectCreatorMock*>(trackeditProjectCreatorPtr.get());
        }
    }

    // Unregister and delete ProjectViewStateCreator
    {
        std::shared_ptr<agu::projectscene::IProjectViewStateCreator> projectViewStateCreatorPtr
            =muse::modularity::globalIoc()->resolve<agu::projectscene::IProjectViewStateCreator>("utests");
        muse::modularity::globalIoc()->unregister<agu::projectscene::IProjectViewStateCreator>("utests");

        if (projectViewStateCreatorPtr) {
            delete static_cast<agu::projectscene::ProjectViewStateCreatorMock*>(projectViewStateCreatorPtr.get());
        }
    }

    // Unregister and delete ProjectSceneConfiguration
    {
        std::shared_ptr<agu::projectscene::IProjectSceneConfiguration> projectSceneConfigPtr
            =muse::modularity::globalIoc()->resolve<agu::projectscene::IProjectSceneConfiguration>("utests");
        muse::modularity::globalIoc()->unregister<agu::projectscene::IProjectSceneConfiguration>("utests");

        if (projectSceneConfigPtr) {
            delete static_cast<agu::projectscene::ProjectSceneConfigurationMock*>(projectSceneConfigPtr.get());
        }
    }

    // Unregister and delete ClipboardMock
    {
        std::shared_ptr<agu::trackedit::ITrackeditClipboard> clipboardPtr
            =muse::modularity::ioc(s_testCtx)->resolve<agu::trackedit::ITrackeditClipboard>("utests");

        muse::modularity::ioc(s_testCtx)->unregister<agu::trackedit::ITrackeditClipboard>("utests");

        if (clipboardPtr) {
            delete static_cast<agu::trackedit::ClipboardMock*>(clipboardPtr.get());
        }
    }
    muse::modularity::removeIoC(s_testCtx);
});
} // namespace agu::project
