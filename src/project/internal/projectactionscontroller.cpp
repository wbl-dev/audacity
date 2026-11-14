#include "projectactionscontroller.h"

#include <QFileDialog>
#include <QWindow>

#include <algorithm>
#include <variant>

#include "framework/global/async/async.h"
#include "framework/global/defer.h"
#include "framework/global/translation.h"
#include "framework/global/io/path.h"
#include "framework/global/progress.h"
#include "framework/global/log.h"
#include "framework/global/types/ret.h"
#include "framework/ui/view/iconcodes.h"
#include "framework/interactive/iinteractive.h"
#include "framework/rcommand/actiontocommand.h"

#include "appshell/appshellcommands.h"

#include "audacityproject.h"
#include "projecterrors.h"
#include "project/projectcommands.h"
#include "project/types/projecttypes.h"

using namespace muse;
using namespace muse::rcommand;
using namespace au::project;

static const muse::Uri PROJECT_PAGE_URI("audacity://project");
static const muse::Uri HOME_PAGE_URI("audacity://home");
static const muse::Uri NEW_PROJECT_URI("audacity://project/new");

static const muse::Uri EXPORT_URI("audacity://project/export");
static const muse::Uri CUSTOM_FFMPEG_OPTIONS("audacity://project/export/ffmpeg");
static const muse::Uri METADATA_DIALOG_URI("audacity://project/export/metadata");
static const muse::Uri EXPORT_LABELS_URI("audacity://project/export/labels");
static const muse::Uri CUSTOM_MAPPING("audacity://project/export/mapping");

static const muse::actions::ActionCode OPEN_CUSTOM_FFMPEG_OPTIONS("open-custom-ffmpeg-options");
static const muse::actions::ActionCode OPEN_METADATA_DIALOG("open-metadata-dialog");
static const muse::actions::ActionCode OPEN_CUSTOM_MAPPING("open-custom-mapping");
namespace {
CommandQuery openConv(const Command& command, const muse::actions::ActionData& args)
{
    CommandQuery query(command);
    if (const QUrl url = args.count() > 0 ? args.arg<QUrl>(0) : QUrl(); !url.isEmpty()) {
        query.addParam(PROJECT_URL_PARAM, Val(url.toString()));
    }
    if (args.count() > 1) {
        query.addParam(PROJECT_DISPLAY_NAME_PARAM, Val(args.arg<QString>(1)));
    }
    return query;
}

CommandQuery filesConv(const Command& command, const muse::actions::ActionData& args)
{
    CommandQuery query(command);
    if (args.count() > 0) {
        ValList files;
        for (const QString& file : args.arg<QStringList>(0)) {
            files.emplace_back(file);
        }
        query.addParam(PROJECT_FILES_PARAM, Val(files));
    }
    if (args.count() > 1) {
        query.addParam(PROJECT_REMOVE_AFTER_IMPORT_PARAM, Val(args.arg<bool>(1)));
    }
    return query;
}

CommandQuery trackIdConv(const Command& command, const muse::actions::ActionData& args)
{
    CommandQuery query(command);
    if (args.count() > 0) {
        query.addParam(PROJECT_TRACK_ID_PARAM, Val(args.arg<au::trackedit::TrackId>(0)));
    }
    return query;
}

const muse::actions::ActionCodeList& prohibitedWhileRecording()
{
    static const muse::actions::ActionCodeList codes {
        "file-close",
        "project-import",
        "file-save",
        "file-save-as",
        "export-audio",
        "export-labels",
        "export-midi",
    };

    return codes;
}

const std::unordered_set<muse::actions::ActionCode>& dontRequireOpenProject()
{
    static const std::unordered_set<muse::actions::ActionCode> codes {
        "file-new",
        "file-open",
        "file-open-recent",
        "project-import-startup-media",
        "continue-last-session",
        "clear-recent",
        "plugin-manager",
        "project-show-in-folder",
    };

    return codes;
}

const muse::actions::ActionCodeList& prohibitedWithoutAudio()
{
    static const muse::actions::ActionCodeList codes {
        "export-audio",
    };

    return codes;
}

const muse::actions::ActionCodeList& prohibitedWithoutLabels()
{
    static const muse::actions::ActionCodeList codes {
        "export-labels",
    };

    return codes;
}
}

ProjectActionsController::ProjectActionsController(muse::modularity::ContextPtr ctx)
    : muse::Contextable(ctx)
{
}

void ProjectActionsController::init()
{
    auto cd = commandDispatcher();

    cd->onRequest(this, PROJECT_NEW_COMMAND, [this]() { return newProject(); });
    cd->onRequest(this, PROJECT_OPEN_COMMAND, [this](const Params& params) { return open(params); });
    cd->onRequest(this, PROJECT_CLEAR_RECENT_COMMAND, [this]() { return clearRecentProjects(); });
    cd->onRequest(this, PROJECT_IMPORT_COMMAND, [this](const Params& params) { return importFiles(params); });
    cd->onRequest(this, PROJECT_IMPORT_STARTUP_MEDIA_COMMAND, [this](const Params& params) { return importStartupMedia(params); });

    cd->onRequest(this, PROJECT_SAVE_COMMAND, [this]() { return Ret(saveProject(SaveMode::Save)); });
    //! TODO AU4: decide whether to implement these functions from scratch in AU4 or
    //! to install our own implementation of the UI (BasicUI API)
    //! right now there's only BasicUI stub which means there's no progress dialog shown on saving
    cd->onRequest(this, PROJECT_SAVE_AS_COMMAND, [this]() { return Ret(saveProject(SaveMode::SaveAs)); });
    cd->onRequest(this, PROJECT_CLOSE_COMMAND, [this]() { return closeProject(); });

    cd->onRequest(this, PROJECT_EXPORT_AUDIO_COMMAND, [this]() { return exportAudio(); });
    cd->onRequest(this, PROJECT_EXPORT_LABELS_COMMAND, [this](const Params& params) { return exportLabels(params); });
    cd->onRequest(this, PROJECT_EXPORT_MIDI_COMMAND, [this]() { return exportMIDI(); });

    cd->onRequest(this, PROJECT_OPEN_METADATA_DIALOG_COMMAND, [this]() { return openMetadataDialog(); });
    cd->onRequest(this, PROJECT_OPEN_CUSTOM_FFMPEG_OPTIONS_COMMAND, [this]() { return openCustomFFmpegOptions(); });
    cd->onRequest(this, PROJECT_OPEN_CUSTOM_MAPPING_COMMAND, [this]() { return openCustomMapping(); });

    static const std::vector<ActionToCommand> actionToCommand {
            { "file-new", PROJECT_NEW_COMMAND, {} },
            { "file-open", PROJECT_OPEN_COMMAND, openConv },
            { "file-open-recent", PROJECT_OPEN_COMMAND, openConv },
            { "clear-recent", PROJECT_CLEAR_RECENT_COMMAND, {} },
            { "project-import", PROJECT_IMPORT_COMMAND, filesConv },
            { "project-import-startup-media", PROJECT_IMPORT_STARTUP_MEDIA_COMMAND, filesConv },
            { "file-save", PROJECT_SAVE_COMMAND, {} },
            { "file-save-as", PROJECT_SAVE_AS_COMMAND, {} },
            { "file-close", PROJECT_CLOSE_COMMAND, {} },
            { "export-audio", PROJECT_EXPORT_AUDIO_COMMAND, {} },
            { "export-labels", PROJECT_EXPORT_LABELS_COMMAND, trackIdConv },
            { "export-midi", PROJECT_EXPORT_MIDI_COMMAND, {} },
            { "open-metadata-editor", PROJECT_OPEN_METADATA_DIALOG_COMMAND, {} },
            { OPEN_METADATA_DIALOG, PROJECT_OPEN_METADATA_DIALOG_COMMAND, {} },
            { OPEN_CUSTOM_FFMPEG_OPTIONS, PROJECT_OPEN_CUSTOM_FFMPEG_OPTIONS_COMMAND, {} },
            { OPEN_CUSTOM_MAPPING, PROJECT_OPEN_CUSTOM_MAPPING_COMMAND, {} },
    };

    registerActionToCommand(this, actionToCommand, commandDispatcher(), dispatcher());

    globalContext()->currentTrackeditProjectChanged().onNotify(this, [this]() {
        listenTrackeditProjectChanges();
    });

    listenTrackeditProjectChanges();

    recordController()->isRecordingChanged().onNotify(this, [this]() {
        m_actionEnabledChanged.send(prohibitedWhileRecording());
    });
}

void ProjectActionsController::listenTrackeditProjectChanges()
{
    trackedit::ITrackeditProjectPtr prj = globalContext()->currentTrackeditProject();
    if (!prj) {
        return;
    }

    prj->hasAudioContent().ch.onReceive(this, [this](bool) {
        m_actionEnabledChanged.send(prohibitedWithoutAudio());
    }, muse::async::Asyncable::Mode::SetReplace);

    prj->hasLabels().ch.onReceive(this, [this](bool) {
        m_actionEnabledChanged.send(prohibitedWithoutLabels());
    }, muse::async::Asyncable::Mode::SetReplace);

    m_actionEnabledChanged.send(prohibitedWithoutAudio());
    m_actionEnabledChanged.send(prohibitedWithoutLabels());
}

muse::async::Channel<muse::actions::ActionCodeList> ProjectActionsController::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

bool ProjectActionsController::canReceiveAction(const muse::actions::ActionCode& code) const
{
    const IAudacityProjectPtr project = currentProject();
    if (!project) {
        return muse::contains(dontRequireOpenProject(), code);
    }

    const trackedit::ITrackeditProjectPtr trackeditProject = globalContext()->currentTrackeditProject();
    if (muse::contains(prohibitedWithoutAudio(), code) && (!trackeditProject || !trackeditProject->hasAudioContent().val)) {
        return false;
    }

    if (muse::contains(prohibitedWithoutLabels(), code) && (!trackeditProject || !trackeditProject->hasLabels().val)) {
        return false;
    }

    if (muse::contains(prohibitedWhileRecording(), code) && recordController()->isRecording()) {
        return false;
    }

    return true;
}

IAudacityProjectPtr ProjectActionsController::currentProject() const
{
    return globalContext()->currentProject();
}

Ret ProjectActionsController::openProject(const ProjectFile& file)
{
    LOGI() << "Try open project: url = " << file.url.toString() << ", displayNameOverride = " << file.displayNameOverride;

    if (file.isNull() || file.url.isLocalFile()) {
        muse::io::paths_t filenames = file.isNull() ? selectOpeningFiles() : muse::io::paths_t { file.path() };
        muse::io::path_t filename = filenames.empty() ? muse::io::path_t() : filenames.front();

        if (filename.empty()) {
            return make_ret(Ret::Code::Cancel);
        }

        if (au::project::isAudacity3File(filename)) {
            auto resolved = openSaveProjectScenario()->resolveLegacyProjectFormat(filename);
            if (!resolved.ret) {
                return resolved.ret;
            }
            filename = resolved.val;
        }

        return openProject(filename, file.displayNameOverride);
    }

    return make_ret(Err::UnsupportedUrl);
}

muse::Ret ProjectActionsController::newProject()
{
    //! NOTE This method is synchronous,
    //! but inside `multiwindowsProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing) {
        return make_ret(Ret::Code::Busy);
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    if (globalContext()->currentProject()) {
        //! Check, if any project is already open in the current window
        //! and there is already a created instance without a project, then activate it
        if (multiwindowsProvider()->isHasWindowWithoutProject()) {
            multiwindowsProvider()->activateWindowWithoutProject();
            return make_ok();
        }

        //! Otherwise, we will create a new instance
        QStringList args;
        args << "--session-type" << "start-with-new";
        multiwindowsProvider()->openNewWindow(args);
        return make_ok();
    }

    auto project = createProjectInCurrentWindow();
    if (!project) {
        LOGE() << "Failed to create new project in current window";
    }

    muse::async::Async::call(this, [this, ok = !!project]() {
        openPageIfNeed(ok ? PROJECT_PAGE_URI : HOME_PAGE_URI);
    });

    return project ? make_ok() : make_ret(Ret::Code::InternalError);
}

muse::Ret ProjectActionsController::open(const Params& params)
{
    const QUrl url = params.contains(PROJECT_URL_PARAM) ? QUrl(params.at(PROJECT_URL_PARAM).toQString()) : QUrl();
    if (params.contains(PROJECT_URL_PARAM) && (!url.isValid() || url.isEmpty())) {
        return make_ret(Ret::Code::BadArgs);
    }

    const QString displayNameOverride = params.at(PROJECT_DISPLAY_NAME_PARAM).toQString();

    Ret ret = make_ret(Ret::Code::Cancel);

    if (!url.isEmpty() && !url.isLocalFile()) {
        ret = openProject(ProjectFile(url, displayNameOverride));
        if (!ret) {
            openPageIfNeed(HOME_PAGE_URI);
        }
        return ret;
    }

    const muse::io::paths_t filePaths = url.isLocalFile() ? muse::io::paths_t { muse::io::path_t(url) } : selectOpeningFiles();

    if (filePaths.empty()) {
        ret = make_ret(Ret::Code::Cancel);
    } else if (filePaths.size() > 1) {
        auto projectIt = std::find_if(filePaths.cbegin(), filePaths.cend(), [](const auto& filePath) {
            return au::project::isAudacityFile(filePath);
        });

        if (projectIt != filePaths.cend()) {
            ret = openProject(ProjectFile(QUrl::fromLocalFile(projectIt->toQString()), displayNameOverride));
        } else {
            muse::io::paths_t supportedFilePaths;
            supportedFilePaths.reserve(filePaths.size());
            for (const auto& filePath : filePaths) {
                if (isFileSupported(filePath)) {
                    supportedFilePaths.emplace_back(filePath);
                }
            }

            if (supportedFilePaths.empty()) {
                ret = make_ret(Err::UnsupportedUrl);
            } else {
                ret = processMediaFiles(supportedFilePaths);
            }
        }
    } else if (!isFileSupported(filePaths.front())) {
        interactive()->error(muse::trc("project", "Error opening file"),
                             muse::mtrc("project", "Could not open file: %1").arg(filePaths.front().toString()).toStdString());
        ret = make_ret(Err::UnsupportedUrl);
    } else if (au::project::isAudacityFile(filePaths.front())) {
        ret = openProject(ProjectFile(QUrl::fromLocalFile(filePaths.front().toQString()), displayNameOverride));
    } else {
        ret = processMediaFiles({ filePaths.front() });
    }

    if (!ret) {
        openPageIfNeed(HOME_PAGE_URI);
    }

    return ret;
}

muse::Ret ProjectActionsController::importFiles(const Params& params)
{
    const IAudacityProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return make_ret(Ret::Code::NotSupported);
    }

    muse::io::paths_t filePaths;
    if (params.contains(PROJECT_FILES_PARAM)) {
        for (const Val& file : params.at(PROJECT_FILES_PARAM).toList()) {
            const io::path_t path = file.toPath();
            if (path.empty()) {
                continue;
            }

            const io::path_t actualPath = fileSystem()->absoluteFilePath(path);
            filePaths.emplace_back(actualPath.empty() ? path : actualPath);
        }

        if (filePaths.empty()) {
            return make_ret(Ret::Code::BadArgs);
        }
    } else {
        filePaths = selectImportFiles();

        if (filePaths.empty()) {
            return make_ret(Ret::Code::Cancel);
        }
    }

    const muse::Ret result = project->import(filePaths);
    if (!result) {
        interactive()->error(muse::trc("project", "Import failed"), result.text());
    }
    return result;
}

muse::Ret ProjectActionsController::importStartupMedia(const Params& params)
{
    muse::io::paths_t filePaths;
    for (const Val& file : params.at(PROJECT_FILES_PARAM).toList()) {
        if (const io::path_t path = file.toPath(); !path.empty()) {
            filePaths.emplace_back(path);
        }
    }

    if (filePaths.empty()) {
        return make_ret(Ret::Code::BadArgs);
    }

    const bool removeAfterImport = params.at(PROJECT_REMOVE_AFTER_IMPORT_PARAM, Val(false)).toBool();

    Ret ret = processMediaFiles(filePaths);
    if (removeAfterImport) {
        for (const auto& filePath : filePaths) {
            fileSystem()->remove(filePath);
        }
    }

    if (!ret) {
        openPageIfNeed(HOME_PAGE_URI);
    }

    return ret;
}

muse::Ret ProjectActionsController::processMediaFiles(const muse::io::paths_t& paths)
{
    if (paths.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    muse::io::paths_t actualPaths;
    actualPaths.reserve(paths.size());
    for (const auto& givenPath : paths) {
        io::path_t actualPath = fileSystem()->absoluteFilePath(givenPath);
        if (actualPath.empty()) {
            return make_ret(Ret::Code::UnknownError);
        }

        actualPaths.emplace_back(actualPath);
    }

    if (globalContext()->currentProject()) {
        QStringList args;
        args << "--session-type" << "start-with-new";
        for (const auto& actualPath : actualPaths) {
            args << "--import-media-file" << actualPath.toQString();
        }

        multiwindowsProvider()->openNewWindow(args);
        return make_ret(Ret::Code::Ok);
    }

    IAudacityProjectPtr project = createProjectInCurrentWindow();
    if (!project) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = openPageIfNeed(PROJECT_PAGE_URI);
    if (!ret) {
        return ret;
    }

    return project->import(actualPaths);
}

bool ProjectActionsController::isUrlSupported(const QUrl& url) const
{
    if (url.isLocalFile()) {
        return isFileSupported(muse::io::path_t(url));
    }

    return false;
}

bool ProjectActionsController::isFileSupported(const muse::io::path_t& path) const
{
    if (au::project::isAudacityFile(path)) {
        return true;
    }

    const std::string ext = io::suffix(path);
    if (ext.empty()) {
        return false;
    }

    const auto supportedExtensions = importer()->supportedExtensions();
    return std::find(supportedExtensions.cbegin(), supportedExtensions.cend(), ext) != supportedExtensions.cend();
}

muse::Ret ProjectActionsController::closeProject()
{
    // reset preferred export sample rate
    exportConfiguration()->setExportSampleRate(-1);

    if (multiwindowsProvider()->windowCount() > 1) {
        mainWindow()->qWindow()->close();
        return make_ok();
    }

    return Ret(closeOpenedProject(false));
}

bool ProjectActionsController::closeOpenedProject(const bool quitApp)
{
    return doCloseOpenedProject(quitApp, false);
}

bool ProjectActionsController::quitDiscardingUnsavedChanges()
{
    if (!doCloseOpenedProject(false, true)) {
        return false;
    }

    commandDispatcher()->dispatch(appshell::APP_QUIT_COMMAND);
    return true;
}

bool ProjectActionsController::doCloseOpenedProject(const bool quitApp, const bool discardUnsavedChanges)
{
    if (m_isProjectClosing) {
        return false;
    }

    m_isProjectClosing = true;
    DEFER {
        m_isProjectClosing = false;
    };

    const IAudacityProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return true;
    }

    if (!discardUnsavedChanges && project->hasUnsavedChanges()) {
        const IInteractive::Button btn = askAboutSavingProject(project);

        if (btn == IInteractive::Button::Cancel) {
            return false;
        }

        if (btn == IInteractive::Button::Save) {
            if (!saveProject()) {
                return false;
            }

        }
    }

    interactive()->closeAllDialogsSync();

    project->close();

    globalContext()->setCurrentProject(nullptr);

    if (quitApp) {
        //! NOTE: we need to call `quit` in the next event loop due to controlling the lifecycle of this method
        muse::async::Async::call(this, [this](){
            commandDispatcher()->dispatch(appshell::APP_QUIT_COMMAND);
        });
    } else {
        Ret ret = openPageIfNeed(HOME_PAGE_URI);
        if (!ret) {
            LOGE() << ret.toString();
        }
    }

    return true;
}

bool ProjectActionsController::saveProject(const muse::io::path_t& path)
{
    if (!path.empty()) {
        return saveProjectLocally(path);
    }

    return saveProject(SaveMode::Save);
}

bool ProjectActionsController::saveProjectLocally(const muse::io::path_t& filePath, SaveMode saveMode)
{
    IAudacityProjectPtr project = currentProject();
    if (!project) {
        return false;
    }

    Ret ret = project->save(filePath, saveMode);
    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    recentFilesController()->prependRecentFile(makeRecentFile(project));
    return true;
}

muse::io::paths_t ProjectActionsController::selectOpeningFiles()
{
    std::vector<std::string> supportedExtensions = importer()->supportedExtensions();

    std::string mediaExt;
    for (const std::string& ext : supportedExtensions) {
        if (ext.empty()) {
            continue;
        }

        if (!mediaExt.empty()) {
            mediaExt += " ";
        }

        mediaExt += "*." + ext;
    }

    const std::string projectExt = "*.aup3 *.aup4";
    const std::string allExt = mediaExt.empty() ? projectExt : projectExt + " " + mediaExt;

    std::vector<std::string> filter {
        trc("project", "All supported files") + " (*.aup4,*.mp3, ...) (" + allExt + ")",
        trc("project", "Audacity project files") + " (*.aup3,*.aup4, ...) (" + projectExt + ")",
        trc("project", "Audacity 3 files") + " (*.aup3, ...) (*.aup3)",
        trc("project", "Audacity 4 files") + " (*.aup4, ...) (*.aup4)",
        trc("project", "Importable audio and media files") + " (*.mp3,*.aac, ...) (" + mediaExt + ")",
    };

    io::path_t defaultDir = configuration()->lastOpenedProjectsPath();

    if (defaultDir.empty()) {
        defaultDir = configuration()->userProjectsPath();
    }

    if (defaultDir.empty()) {
        defaultDir = configuration()->defaultUserProjectsPath();
    }

    //: Title of a file picker dialog
    io::paths_t filePaths = interactive()->selectOpeningFilesSync(muse::trc("project",
                                                                            "Open"), defaultDir, filter,
                                                                  QFileDialog::HideNameFilterDetails);

    if (!filePaths.empty()) {
        configuration()->setLastOpenedProjectsPath(io::dirpath(filePaths.front()));
    }

    return filePaths;
}

muse::io::paths_t ProjectActionsController::selectImportFiles()
{
    std::string audioFileExt
        = "*.aac *.ac3 *.mp2 *.mp3 *.wma *.wav *.flac *.ogg *.opus *.aif *.aiff *.amr *.ape *.au *.dts *.mpc *.tta *.wv *.shn *.voc *.mmf";
    std::string videoFileExt
        = "*.avi *.mp4 *.mkv *.mov *.flv *.wmv *.asf *.webm *.mpg *.mpeg *.m4v *.ts *.gxf *.mxf *.nut *.dv *.3gp *.3g2 *.mj2";
    std::string gameMediaFileExt
        =
            "*.roq *.bethsoftvid *.c93 *.dsicin *.dxa *.ea *.cdata *.film_cpk *.idcin *.ipmovie *.psxstr *.rl2 *.siff *.smk *.thp *.tiertexseq *.vmd *.wc3movie *.wsaud *.wsvqa *.txd";
    std::string streamingFileExt = "*.rtsp *.sdp *.nsv *.pva *.msnwctcp *.lmlm4 *.redir";
    std::string animationAndImageFileExt = "*.gif *.flic *.swf *.image2 *.image2pipe";
    std::string rawFileExt
        =
            "*.al *.ul *.s16be *.u16be *.s8 *.u8 *.ub *.uw *.4xm *.MTV *.afc *.aifc *.apc *.apl *.mac *.avs *.302 *.daud *.ffm *.cgi *.mm *.mpegtsraw *.mpegvideo *.nuv *.sw *.sb *.son *.sol *.vfwcap";
    std::string allExt = audioFileExt + " " + videoFileExt + " " + gameMediaFileExt + " " + streamingFileExt + " "
                         + animationAndImageFileExt + " " + rawFileExt;
    for (const std::string& extension : importer()->supportedExtensions()) {
        allExt += " *." + extension;
    }

    std::vector<std::string> filter {
        trc("project", "All supported files") + " (*.mp3,*.aac, ...) (" + allExt + ")",
        trc("project", "Audio files") + " (*.mp3,*.aac, ...) (" + audioFileExt + ")",
        trc("project", "Video files") + " (*.mp4,*.avi, ...) (" + videoFileExt + ")",
        trc("project", "Game media files") + " (*.roq,*.ea, ...) (" + gameMediaFileExt + ")",
        trc("project", "Streaming files") + " (*.rtsp,*.sdp, ...) (" + streamingFileExt + ")",
        trc("project", "Animation and image files") + " (*.gif, ...) (" + animationAndImageFileExt + ")",
        trc("project", "Raw files") + " (*.al,*.ul, ...) (" + rawFileExt + ")"
    };

    io::path_t defaultDir = configuration()->lastOpenedProjectsPath();

    if (defaultDir.empty()) {
        defaultDir = configuration()->userProjectsPath();
    }

    if (defaultDir.empty()) {
        defaultDir = configuration()->defaultUserProjectsPath();
    }

    io::paths_t filePaths = interactive()->selectOpeningFilesSync(muse::trc("project",
                                                                            "Open"), defaultDir, filter,
                                                                  QFileDialog::HideNameFilterDetails);

    if (!filePaths.empty()) {
        configuration()->setLastOpenedProjectsPath(io::dirpath(filePaths.front()));
    }

    return filePaths;
}

IInteractive::Button ProjectActionsController::askAboutSavingProject(IAudacityProjectPtr project)
{
    std::string title;

    if (project->isNewlyCreated()) {
        title = muse::qtrc("project", "Do you want to save changes to the project before closing?").toStdString();
    } else {
        title = muse::qtrc("project", "Do you want to save changes to the project “%1” before closing?")
                .arg(project->displayName()).toStdString();
    }

    std::string body = muse::trc("project", "Your changes will be lost if you don’t save them.");

    IInteractive::Result result = interactive()->warningSync(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save, { IInteractive::Option::WithIcon },
                                                             muse::trc("project", "Unsaved changes"));

    return result.standardButton();
}

Ret ProjectActionsController::canSaveProject() const
{
    auto project = currentProject();
    if (!project) {
        LOGW() << "no current project";
        return make_ret(Err::NoProjectError);
    }

    return project->canSave();
}

bool ProjectActionsController::saveProject(SaveMode saveMode)
{
    if (m_isProjectSaving) {
        return false;
    }

    m_isProjectSaving = true;
    DEFER {
        m_isProjectSaving = false;
    };

    IAudacityProjectPtr project = currentProject();

    if (saveMode == SaveMode::Save && !project->isNewlyCreated()) {
        return saveProjectLocally({}, saveMode);
    }

    RetVal<muse::io::path_t> response = openSaveProjectScenario()->askLocalPath(project, saveMode);
    if (!response.ret) {
        LOGE() << response.ret.toString();
        return false;
    }

    return saveProjectLocally(response.val, saveMode);
}

muse::Ret ProjectActionsController::openProject(const muse::io::path_t& path, const String& displayNameOverride)
{
    //! NOTE This method is synchronous,
    //! but inside `multiwindowsProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing) {
        return make_ret(Ret::Code::InternalError);
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    //! Step 1. Take absolute path
    io::path_t actualPath = fileSystem()->absoluteFilePath(path);
    if (actualPath.empty()) {
        // We assume that a valid path has been specified to this method
        return make_ret(Ret::Code::UnknownError);
    }

    //! Step 2. If the project is already open in the current window, then just switch to showing the project
    if (isProjectOpened(actualPath)) {
        return openPageIfNeed(PROJECT_PAGE_URI);
    }

    //! Step 3. Check, if the project already opened in another window, then activate the window with the project
    if (multiwindowsProvider()->isProjectAlreadyOpened(actualPath)) {
        multiwindowsProvider()->activateWindowWithProject(actualPath);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 4. Check, if a any project is already open in the current window,
    //! then create a new instance
    if (globalContext()->currentProject()) {
        QStringList args;
        args << actualPath.toQString();

        if (!displayNameOverride.isEmpty()) {
            args << "--project-display-name-override" << displayNameOverride;
        }
        multiwindowsProvider()->openNewWindow(args);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 5. Open project in the current window
    return doOpenProject(actualPath);
}

IAudacityProjectPtr ProjectActionsController::createProjectInCurrentWindow()
{
    IAudacityProjectPtr project = std::make_shared<Audacity4Project>(iocContext());
    Ret ret = project->createNew();
    if (!ret) {
        LOGE() << ret.toString();
        return nullptr;
    }

    globalContext()->setCurrentProject(project);
    projectHistory()->init();

    return project;
}

Ret ProjectActionsController::doOpenProject(const io::path_t& filePath)
{
    TRACEFUNC;

    RetVal<IAudacityProjectPtr> rv = loadProject(filePath);
    if (!rv.ret) {
        return rv.ret;
    }

    IAudacityProjectPtr project = rv.val;

    // Check if this is an autosave of a newly created project
    if (!project->isNewlyCreated() && !au::project::isAudacityUnsavedFile(project->path())) {
        recentFilesController()->prependRecentFile(makeRecentFile(project));
    }

    globalContext()->setCurrentProject(project);

#ifndef AU_LOAD_TIMETRACK
    const auto trackeditProject = project->trackeditProject();
    if (trackeditProject && trackeditProject->timeTrackFound()) {
        interactive()->infoSync(muse::trc("project/open", "Time Track not supported"),
                                muse::trc("project/open",
                                          "The project contains a time track, which is not yet supported in Audacity 4, and will need to be removed. This does not affect your original Audacity 3 project."),
        {
            muse::IInteractive::ButtonData(
                muse::IInteractive::Button::Ok, muse::trc("project/open", "OK"), false)
        });

        // When saving we do a full project rewrite
        // We need to save the project immediately to remove the time track from the project file and avoid showing this message repeatedly
        saveProject(SaveMode::Save);
    }
#endif

    projectHistory()->init();

    const Ret ret = openPageIfNeed(PROJECT_PAGE_URI);
    if (ret) {
        missingEffectChecker()->warnIfEffectsMissing();
    }

    return ret;
}

RetVal<IAudacityProjectPtr> ProjectActionsController::loadProject(const io::path_t& filePath)
{
    TRACEFUNC;

    //! TODO AU4
    // auto project = projectCreator()->newProject();
    // IF_ASSERT_FAILED(project) {
    //     return make_ret(Ret::Code::InternalError);
    // }
    IAudacityProjectPtr project = std::make_shared<Audacity4Project>(iocContext());

    //! TODO AU4
    // bool hasUnsavedChanges = project->hasUnsavedChanges();
    // io::path_t loadPath = hasUnsavedChanges ? project->autoSavePath(filePath) : filePath;

    const io::path_t loadPath = filePath;
    const std::string format = io::suffix(filePath);

    if (Ret result = loadWithFallback(project, loadPath, format); !result) {
        return result;
    }

    //! TODO AU4
    // if (hasUnsavedChanges) {
    //     //! NOTE: redirect the project to the original file path
    //     project->setPath(filePath);

    //     project->markAsUnsaved();
    // }

    // Mark project as newly created if it's an autosave of a new project
    if (project->isNewlyCreated()) {
        // Mark as newly created (this will be implemented if needed)
        // project->markAsNewlyCreated();
    }

    return RetVal<IAudacityProjectPtr>::make_ok(project);
}

Ret ProjectActionsController::loadWithFallback(const IAudacityProjectPtr& project,
                                               const muse::io::path_t& loadPath,
                                               const std::string& format)
{
    bool forceLoad = false;
    Ret result = project->load(loadPath, forceLoad, format);

    if (result || result.code() == static_cast<int>(Ret::Code::Cancel)) {
        return result;
    }

    forceLoad = shouldRetryLoadAfterError(result, loadPath);
    if (forceLoad) {
        result = project->load(loadPath, forceLoad, format);
    }

    return result;
}

bool ProjectActionsController::isProjectOpened(const muse::io::path_t& projectPath) const
{
    auto project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    LOGD() << "project->path: " << project->path() << ", check path: " << projectPath;
    if (project->path() == projectPath) {
        return true;
    }

    return false;
}

RecentFile ProjectActionsController::makeRecentFile(IAudacityProjectPtr project)
{
    RecentFile file;
    file.path = project->path();

    return file;
}

muse::Ret ProjectActionsController::clearRecentProjects()
{
    recentFilesController()->clearRecentFiles();
    return make_ok();
}

bool ProjectActionsController::shouldRetryLoadAfterError(const Ret& ret, const muse::io::path_t& filepath)
{
    if (ret) {
        return true;
    }
    warnProjectCannotBeOpened(ret, filepath);
    return false;
}

void ProjectActionsController::warnProjectCannotBeOpened(const Ret& ret, const muse::io::path_t& filepath) const
{
    const std::string title
        = ret.data<std::string>("title",
                                muse::mtrc("project", "Cannot read file %1")
                                .arg(io::toNativeSeparators(filepath).toString())
                                .toStdString());

    const std::string body
        = ret.data<std::string>("body", !ret.text().empty() ? ret.text() : muse::trc("project",
                                                                                     "An error occurred while reading this file."));
    interactive()->error(title, body);
}

muse::Ret ProjectActionsController::exportAudio()
{
    interactive()->open(EXPORT_URI);
    return make_ok();
}

muse::Ret ProjectActionsController::exportLabels(const Params& params)
{
    muse::UriQuery query(EXPORT_LABELS_URI);

    const trackedit::TrackId trackId = params.at(PROJECT_TRACK_ID_PARAM, Val(static_cast<int64_t>(-1))).toInt64();
    query.addParam("trackId", Val(trackId));

    interactive()->open(query);
    return make_ok();
}

muse::Ret ProjectActionsController::exportMIDI()
{
    NOT_IMPLEMENTED;
    return make_ret(Ret::Code::NotImplemented);
}

void ProjectActionsController::undo()
{
    NOT_IMPLEMENTED;
}

void ProjectActionsController::redo()
{
    NOT_IMPLEMENTED;
}

muse::Ret ProjectActionsController::openPageIfNeed(muse::Uri pageUri)
{
    if (interactive()->isOpened(pageUri).val) {
        return muse::make_ret(muse::Ret::Code::Ok);
    }

    interactive()->open(pageUri);
    return muse::make_ok();
}

muse::Ret ProjectActionsController::openCustomFFmpegOptions()
{
    interactive()->open(CUSTOM_FFMPEG_OPTIONS);
    return make_ok();
}

muse::Ret ProjectActionsController::openMetadataDialog()
{
    interactive()->open(METADATA_DIALOG_URI);
    return make_ok();
}

muse::Ret ProjectActionsController::openCustomMapping()
{
    interactive()->open(CUSTOM_MAPPING);
    return make_ok();
}
