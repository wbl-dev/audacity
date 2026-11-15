#ifndef AU_PROJECT_PROJECTACTIONSCONTROLLER_H
#define AU_PROJECT_PROJECTACTIONSCONTROLLER_H

#include "framework/global/async/asyncable.h"
#include "framework/global/modularity/ioc.h"
#include "framework/global/io/ifilesystem.h"
#include "framework/global/iapplication.h"

#include "framework/actions/actionable.h"
#include "framework/actions/iactionsdispatcher.h"
#include "framework/actions/actiontypes.h"
#include "framework/rcommand/commandable.h"
#include "framework/rcommand/icommanddispatcher.h"
#include "framework/global/async/channel.h"
#include "framework/interactive/iinteractive.h"
#include "framework/interactive/iplatforminteractive.h"
#include "framework/ui/imainwindow.h"

#include "context/iglobalcontext.h"
#include "project/irecentfilescontroller.h"
#include "iopensaveprojectscenario.h"
#include "framework/toast/itoastservice.h"
#include "trackedit/iprojecthistory.h"
#include "record/irecordcontroller.h"
#include "importexport/export/internal/exportconfiguration.h"
#include "importexport/import/iimporter.h"
#include "effects/effects_base/imissingeffectchecker.h"

#include "project/iprojectconfiguration.h"
#include "project/iprojectfilescontroller.h"
#include "project/iaguproject.h"
#include "multiwindows/imultiwindowsprovider.h"

namespace agu::project {
class ProjectActionsController : public IProjectFilesController, public muse::actions::Actionable, public muse::rcommand::Commandable,
    public muse::async::Asyncable, public muse::Contextable
{
    muse::GlobalInject<muse::IApplication> application;
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<importexport::ExportConfiguration> exportConfiguration;
    muse::GlobalInject<muse::IPlatformInteractive> platformInteractive;
    muse::GlobalInject<IRecentFilesController> recentFilesController;
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<muse::toast::IToastService> toastService;

    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher { this };
    muse::ContextInject<muse::ui::IMainWindow> mainWindow { this };
    muse::ContextInject<agu::context::IGlobalContext> globalContext { this };
    muse::ContextInject<muse::IInteractive> interactive { this };
    muse::ContextInject<IOpenSaveProjectScenario> openSaveProjectScenario { this };
    muse::ContextInject<trackedit::IProjectHistory> projectHistory { this };
    muse::ContextInject<record::IRecordController> recordController { this };
    muse::ContextInject<importexport::IImporter> importer { this };
    muse::ContextInject<effects::IMissingEffectChecker> missingEffectChecker { this };

public:
    ProjectActionsController(muse::modularity::ContextPtr ctx = nullptr);

    void init();

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

    [[nodiscard]] muse::Ret openProject(const ProjectFile& file) override;
    bool isUrlSupported(const QUrl& url) const override;
    bool isFileSupported(const muse::io::path_t& path) const override;
    bool closeOpenedProject(bool quitApp = false) override;
    bool quitDiscardingUnsavedChanges() override;
    bool saveProject(const muse::io::path_t& path = muse::io::path_t()) override;
    bool saveProjectLocally(const muse::io::path_t& filePath = muse::io::path_t(), SaveMode saveMode = SaveMode::Save) override;
    bool isProjectOpened(const muse::io::path_t& projectPath) const;
    muse::async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const;

private:
    bool doCloseOpenedProject(bool quitApp, bool discardUnsavedChanges);
    project::IAGuProjectPtr currentProject() const;

    muse::Ret newProject();
    muse::Ret open(const muse::rcommand::Params& params);
    muse::Ret importFiles(const muse::rcommand::Params& params);

    muse::Ret importStartupMedia(const muse::rcommand::Params& params);
    muse::Ret processMediaFiles(const muse::io::paths_t& paths);
    muse::Ret closeProject();

    muse::Ret openProject(const muse::io::path_t& path,
                          const muse::String& displayNameOverride = muse::String());
    muse::Ret loadWithFallback(const IAGuProjectPtr& project, const muse::io::path_t& loadPath, const std::string& format);
    muse::Ret doOpenProject(const muse::io::path_t& filePath);
    IAGuProjectPtr createProjectInCurrentWindow();
    //! TODO AU4
    // muse::Ret openAudacityUrl(const QUrl& url);
    muse::RetVal<IAGuProjectPtr> loadProject(const muse::io::path_t& filePath);
    muse::io::paths_t selectOpeningFiles();
    muse::io::paths_t selectImportFiles();

    bool shouldRetryLoadAfterError(const muse::Ret& ret, const muse::io::path_t& filepath);
    void warnProjectCannotBeOpened(const muse::Ret& ret, const muse::io::path_t& filepath) const;

    muse::IInteractive::Button askAboutSavingProject(IAGuProjectPtr project);

    muse::Ret canSaveProject() const;
    bool saveProject(SaveMode saveMode);

    RecentFile makeRecentFile(IAGuProjectPtr project);
    muse::Ret clearRecentProjects();

    muse::Ret exportAudio();
    muse::Ret exportLabels(const muse::rcommand::Params& params);
    muse::Ret exportMIDI();

    void undo();
    void redo();

    muse::Ret openPageIfNeed(muse::Uri pageUri);

    muse::Ret openCustomFFmpegOptions();
    muse::Ret openMetadataDialog();
    muse::Ret openCustomMapping();

    void listenTrackeditProjectChanges();

    bool m_isProjectSaving = false;
    bool m_isProjectClosing = false;
    bool m_isProjectProcessing = false;

    muse::async::Channel<muse::actions::ActionCodeList> m_actionEnabledChanged;
};
}

#endif // AU_PROJECT_PROJECTACTIONSCONTROLLER_H
