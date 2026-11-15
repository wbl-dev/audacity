#include "aguproject.h"

#include "framework/global/log.h"
#include "framework/global/io/fileinfo.h"
#include "framework/global/io/ioretcodes.h"

#include "project/projecterrors.h"

using namespace muse;
using namespace agu::project;
using namespace agu::trackedit;
using namespace agu::projectscene;

static QString projectDefaultTitle()
{
    return muse::qtrc("project", "Untitled project");
}

Ret AGuProject::createNew()
{
    m_au3Project = au3ProjectCreator()->create(iocContext());
    if (!m_au3Project->open()) {
        return muse::make_ret(static_cast<Ret::Code>(agu::project::Err::NoProjectError));
    }
    setPath(m_au3Project->getFileName());
    m_trackeditProject = trackeditProjectCreator()->create(m_au3Project);
    m_isNewlyCreated = true;
    m_viewState = viewStateCreator()->createViewState(m_au3Project);
    setupProjectNotifications();

    return muse::make_ret(Ret::Code::Ok);
}

muse::Ret AGuProject::load(const muse::io::path_t& path, const bool forceMode, const std::string& format_)
{
    TRACEFUNC;

    const std::string format = format_.empty() ? io::suffix(path) : format_;

    LOGD() << "try load: " << path << ", format: " << format;

    //setupProject();
    setPath(path);

    //! TODO AU4
    // if (!isAudacityFile(format)) {
    //     Ret ret = doImport(path, forceMode);
    //     if (ret) {
    //         listenIfNeedSaveChanges();
    //     }

    //     return ret;
    // }

    Ret ret = doLoad(path, forceMode, format);
    if (!ret) {
        LOGE() << "failed load, err: " << ret.toString();
        return ret;
    }

    //! TODO AU4
    // listenIfNeedSaveChanges();

    return ret;
}

Ret AGuProject::importIntoTrack(const muse::io::path_t& filePath,
                                      trackedit::TrackId dstTrackId,
                                      muse::secs_t startTime)
{
    const std::string importInfo = muse::qtrc("project", "Imported file “%1”?")
                                   .arg(filePath.toString()).toStdString();
    Ret ok = doImportIntoTrack(filePath, dstTrackId, startTime);

    if (ok) {
        //: Name of the undo history entry created by importing files
        projectHistory()->pushHistoryState(importInfo, muse::trc("project", "Import"));
    }

    return ok;
}

Ret AGuProject::importIntoTracks(const std::vector<muse::io::path_t>& filePaths,
                                       const std::vector<trackedit::TrackId>& dstTrackIds,
                                       muse::secs_t startTime)
{
    return importDroppedFiles(filePaths, dstTrackIds, {}, startTime);
}

Ret AGuProject::importDroppedFiles(const std::vector<muse::io::path_t>& audioPaths,
                                         const std::vector<trackedit::TrackId>& dstTrackIds,
                                         const std::vector<muse::io::path_t>& labelPaths,
                                         muse::secs_t startTime)
{
    IF_ASSERT_FAILED(audioPaths.size() == dstTrackIds.size()) {
        return muse::make_ret(Ret::Code::InternalError);
    }

    std::vector<std::string> failures;
    bool anyImported = false;
    for (size_t i = 0; i < audioPaths.size(); ++i) {
        Ret ok = doImportIntoTrack(audioPaths.at(i), dstTrackIds.at(i), startTime);
        if (ok) {
            anyImported = true;
        } else {
            failures.push_back(audioPaths[i].toStdString() + ": " + ok.toString());
        }
    }
    for (const auto& path : labelPaths) {
        const Ret ok = doImport(path, false);
        if (ok) {
            anyImported = true;
        } else {
            failures.push_back(path.toStdString() + ": " + ok.toString());
        }
    }

    if (anyImported) {
        projectHistory()->pushHistoryState(muse::trc("project", "Imported multiple files"), muse::trc("project", "Import"));
    }

    if (failures.empty()) {
        return muse::make_ok();
    }
    std::string detail;
    for (const auto& message : failures) {
        if (!detail.empty()) {
            detail += "\n";
        }
        detail += message;
    }
    return muse::make_ret(Ret::Code::BadData, detail);
}

Ret AGuProject::import(const muse::io::path_t& path, const bool forceMode)
{
    const std::string importInfo = muse::qtrc("project", "Imported file “%1”?")
                                   .arg(path.toString()).toStdString();

    Ret ok = doImport(path, forceMode);
    if (ok) {
        projectHistory()->pushHistoryState(importInfo, muse::trc("project", "Import"));
    }

    return ok;
}

Ret AGuProject::import(const std::vector<muse::io::path_t>& paths, bool forceMode)
{
    std::vector<std::string> failures;
    bool anyImported = false;
    for (const auto& path: paths) {
        const Ret result = doImport(path, forceMode);
        if (result) {
            anyImported = true;
        } else {
            failures.push_back(path.toStdString() + ": " + result.toString());
        }
    }

    if (anyImported) {
        projectHistory()->pushHistoryState(muse::trc("project", "Imported multiple files"), muse::trc("project", "Import"));
    }

    if (failures.empty()) {
        return muse::make_ok();
    }
    std::string detail;
    for (const auto& message : failures) {
        if (!detail.empty()) {
            detail += "\n";
        }
        detail += message;
    }
    return muse::make_ret(Ret::Code::BadData, detail);
}

muse::Ret AGuProject::doLoad(const io::path_t& path, const bool forceMode, const std::string& format)
{
    muse::Ret ret = muse::make_ret(Ret::Code::Ok);
    TRACEFUNC;

    UNUSED(forceMode);
    UNUSED(format);

    if (!io::FileInfo::exists(path)) {
        LOGE() << "file does not exist at path: \"" << path << "\"";
        return make_ret(Err::ProjectFileNotFound, path);
    }

    m_au3Project = au3ProjectCreator()->create(iocContext());
    ret = m_au3Project->load(path);
    if (!ret) {
        LOGE() << "Failed load:" << path;
        return ret;
    }

    LOGI() << "success loaded au3 project: " << m_au3Project->title();

    m_trackeditProject = trackeditProjectCreator()->create(m_au3Project);

    m_viewState = viewStateCreator()->createViewState(m_au3Project);
    setupProjectNotifications();

    // For restored never-saved projects, ensure proper initialization
    // by calling reload() to trigger tracksChanged notifications.
    // This fixes issues with recording in restored projects.
    if (m_au3Project->isRecovered()) {
        m_trackeditProject->reload();

        if (m_au3Project->isTemporary()) {
            m_isNewlyCreated = true;
            LOGD() << "[project] Restored never-saved project, marked as newly created";
        }
    }

    return ret;
}

void AGuProject::setupProjectNotifications()
{
    m_au3Project->projectChanged().onNotify(this, [this]() {
        m_displayNameChanged.notify();
        setNeedSave(true);
    }, muse::async::Asyncable::Mode::SetReplace);
}

Ret AGuProject::doImport(const muse::io::path_t& path, const bool forceMode) const
{
    TRACEFUNC;

    UNUSED(forceMode);

    if (!m_au3Project) {
        return muse::make_ret(muse::Ret::Code::InternalError);
    }

    const bool ok = importer()->import(path);
    m_trackeditProject->reload();

    if (ok) {
        return muse::make_ok();
    }
    const muse::Ret error = importer()->lastError();
    return error.success() ? muse::make_ret(Ret::Code::BadData, "Could not import " + path.toStdString()) : error;
}

Ret AGuProject::doImportIntoTrack(const muse::io::path_t& path, trackedit::TrackId dstTrackId, muse::secs_t startTime)
{
    TRACEFUNC;

    if (!m_au3Project) {
        return muse::make_ret(muse::Ret::Code::InternalError);
    }

    const bool ok = importer()->importIntoTrack(path, dstTrackId, startTime);
    m_trackeditProject->reload();

    return ok ? muse::make_ret(Ret::Code::Ok) : muse::make_ret(Ret::Code::InternalError);
}

void AGuProject::close()
{
    m_aboutCloseBegin.notify();
    clipboard()->clearTrackData();
    m_au3Project->close();
    m_aboutCloseEnd.notify();
}

muse::async::Notification AGuProject::aboutCloseBegin() const
{
    return m_aboutCloseBegin;
}

muse::async::Notification AGuProject::aboutCloseEnd() const
{
    return m_aboutCloseEnd;
}

QString AGuProject::displayName() const
{
    if (isNewlyCreated()) {
        if (m_path.empty()) {
            QString workTitle = QString::fromStdString(m_au3Project->title());
            if (workTitle.isEmpty()) {
                return projectDefaultTitle();
            }
            return workTitle;
        }
        return io::filename(m_path).toQString();
    }

    return io::filename(m_path, false /*isSuffixInteresting*/).toQString();
}

muse::async::Notification AGuProject::displayNameChanged() const
{
    return m_displayNameChanged;
}

void AGuProject::setPath(const io::path_t& path)
{
    if (m_path == path) {
        return;
    }

    m_path = path;
    m_pathChanged.notify();

}

bool AGuProject::isNewlyCreated() const
{
    return m_isNewlyCreated;
}

bool AGuProject::isImported() const
{
    return m_isImported;
}

String AGuProject::title() const
{
    return String::fromStdString(m_au3Project->title());
}

ValNt<bool> AGuProject::needSave() const
{
    ValNt<bool> needSave;

    if (m_au3Project) {
        needSave.val = m_au3Project->hasUnsavedChanges();

        if (m_isNewlyCreated) {
            needSave.val = true;
        }
    } else {
        needSave.val = false;
    }

    needSave.notification = m_needSaveNotification;
    return needSave;
}

Ret AGuProject::canSave() const
{
    //! TODO AU4
    // if (!m_masterNotation->hasParts()) {
    //     return make_ret(Err::NoPartsError);
    // }

    // Ret ret = m_engravingProject->checkCorrupted();
    // if (!ret) {
    //     Err errorCode = m_engravingProject->isCorruptedUponLoading() ? Err::CorruptionUponOpeningError : Err::CorruptionError;
    //     ret.setCode(static_cast<int>(errorCode));
    // }

    // return ret;
    return true;
}

bool AGuProject::needAutoSave() const
{
    return m_needAutoSave;
}

void AGuProject::setNeedAutoSave(bool val)
{
    LOGD() << "[project] setNeedAutoSave: " << val;
    m_needAutoSave = val;
}

bool AGuProject::hasUnsavedChanges()
{
    return needSave().val;
}

Ret AGuProject::save(const muse::io::path_t& path, SaveMode saveMode)
{
    TRACEFUNC;

    switch (saveMode) {
    case SaveMode::Save:
    case SaveMode::SaveAs:
    case SaveMode::SaveCopy: {
        muse::io::path_t savePath = path;
        if (savePath.empty()) {
            IF_ASSERT_FAILED(!m_path.empty()) {
                return false;
            }

            savePath = m_path;
        }

        Ret ret = saveProject(savePath);
        if (ret) {
            if (saveMode != SaveMode::SaveCopy) {
                markAsSaved(savePath);
            }
        }

        return ret;
    }
    case SaveMode::AutoSave:
        return saveProject(path, false /*generateBackup*/, true /*createThumbnail*/);
    case SaveMode::SaveSelection:
        return make_ret(Err::UnknownError);
    }

    return make_ret(Err::UnknownError);
}

Ret AGuProject::saveProject(const muse::io::path_t& path, bool generateBackup, bool createThumbnail)
{
    return doSave(path, generateBackup, createThumbnail);
}

Ret AGuProject::doSave(const muse::io::path_t& savePath, bool generateBackup, bool createThumbnail)
{
    TRACEFUNC;

    UNUSED(generateBackup);

    if ((fileSystem()->exists(savePath) && !fileSystem()->isWritable(savePath))) {
        LOGE() << "failed save, not writable path: " << savePath;
        return make_ret(io::Err::FSWriteError);
    }

    if (savePath.empty()) {
        LOGE() << "failed save, path is empty";
        return make_ret(io::Err::FSNotExist);
    }

    if (createThumbnail) {
        std::optional<std::vector<uint8_t> > pngData = thumbnailCreator()->createThumbnail();
        if (!pngData.has_value()) {
            LOGE() << "Failed to create thumbnail for: " << savePath;
        } else {
            m_au3Project->saveThumbnail(std::move(pngData).value());
        }
    }

    auto ret = m_au3Project->save(savePath);
    if (!ret) {
        return make_ret(Ret::Code::UnknownError);
    }

    return muse::make_ok();

    //! TODO AU4
    // QString targetContainerPath = engraving::containerPath(path).toQString();
    // muse::io::path_t targetMainFilePath = engraving::mainFilePath(path);
    // muse::io::path_t targetMainFileName = engraving::mainFileName(path);
    // QString savePath = targetContainerPath + "_saving";

    // // Step 1: check writable
    // {
    //     if ((fileSystem()->exists(savePath) && !fileSystem()->isWritable(savePath))
    //         || (fileSystem()->exists(targetContainerPath) && !fileSystem()->isWritable(targetContainerPath))) {
    //         LOGE() << "failed save, not writable path: " << targetContainerPath;
    //         return make_ret(io::Err::FSWriteError);
    //     }

    //     if (ioMode == engraving::MscIoMode::Dir) {
    //         // Dir needs to be created, otherwise we can't move to it
    //         if (!QDir(targetContainerPath).mkpath(".")) {
    //             LOGE() << "Couldn't create container directory: " << targetContainerPath;
    //             return make_ret(io::Err::FSMakingError);
    //         }
    //     }
    // }

    // // Step 2: write project
    // {
    //     MscWriter::Params params;
    //     params.filePath = savePath;
    //     params.mainFileName = targetMainFileName.toQString();
    //     params.mode = ioMode;
    //     IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
    //         return make_ret(Ret::Code::InternalError);
    //     }

    //     MscWriter msczWriter(params);
    //     Ret ret = writeProject(msczWriter, false /*onlySelection*/, createThumbnail);
    //     msczWriter.close();

    //     if (!ret) {
    //         LOGE() << "failed write project to buffer: " << ret.toString();
    //         return ret;
    //     }

    //     if (msczWriter.hasError()) {
    //         LOGE() << "MscWriter has error after writing project";
    //         return make_ret(Ret::Code::UnknownError);
    //     }
    // }

    // // Step 3: create backup if need
    // {
    //     if (generateBackup) {
    //         makeCurrentFileAsBackup();
    //     }
    // }

    // // Step 4: replace to saved file
    // {
    //     if (ioMode == MscIoMode::Dir) {
    //         RetVal<io::paths_t> filesToBeMoved = fileSystem()->scanFiles(savePath, { "*" }, io::ScanMode::FilesAndFoldersInCurrentDir);
    //         if (!filesToBeMoved.ret) {
    //             return filesToBeMoved.ret;
    //         }

    //         Ret ret = muse::make_ok();

    //         for (const muse::io::path_t& fileToBeMoved : filesToBeMoved.val) {
    //             muse::io::path_t destinationFile
    //                 = muse::io::path_t(targetContainerPath).appendingComponent(io::filename(fileToBeMoved));
    //             LOGD() << fileToBeMoved << " to " << destinationFile;
    //             ret = fileSystem()->move(fileToBeMoved, destinationFile, true);
    //             if (!ret) {
    //                 return ret;
    //             }
    //         }

    //         // Try to remove the temp save folder (not problematic if fails)
    //         ret = fileSystem()->remove(savePath, true);
    //         if (!ret) {
    //             LOGW() << ret.toString();
    //         }
    //     } else {
    //         Ret ret = fileSystem()->move(savePath, targetContainerPath, true);
    //         if (!ret) {
    //             return ret;
    //         }
    //     }
    // }

    // // make file readable by all
    // {
    //     QFile::setPermissions(targetMainFilePath.toQString(),
    //                           QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    // }

    // LOGI() << "success save file: " << targetContainerPath;
    // return make_ret(Ret::Code::Ok);
}

void AGuProject::markAsSaved(const muse::io::path_t& path)
{
    TRACEFUNC;

    //! NOTE: order is important
    m_isNewlyCreated = false;

    setNeedSave(false);

    setPath(path);

    // Mark the AU3 project as saved
    if (m_au3Project) {
        m_au3Project->markAsSaved();
    }
}

void AGuProject::setNeedSave(bool needSave)
{
    LOGD() << "[project] setNeedSave: " << needSave;

    // Set autosave flag when project needs saving
    setNeedAutoSave(needSave);

    // Trigger save notification
    m_needSaveNotification.notify();
}

const ITrackeditProjectPtr AGuProject::trackeditProject() const
{
    return m_trackeditProject;
}

IProjectViewStatePtr AGuProject::viewState() const
{
    return m_viewState;
}

uintptr_t AGuProject::au3ProjectPtr() const
{
    return m_au3Project->au3ProjectPtr();
}
