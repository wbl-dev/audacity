#pragma once

#include <gmock/gmock.h>

#include "../project/iaguproject.h"

namespace agu::projectscene {
class AGu4ProjectMock : public project::IAGuProject
{
public:
    MOCK_METHOD(muse::Ret, createNew, (), (override));
    MOCK_METHOD(uintptr_t, au3ProjectPtr, (), (const, override));
    MOCK_METHOD(muse::Ret, load, (const muse::io::path_t&, bool, const std::string&), (override));
    MOCK_METHOD(muse::Ret, import, (const muse::io::path_t&, bool), (override));
    MOCK_METHOD(muse::Ret, import, (const std::vector<muse::io::path_t>&, bool), (override));
    MOCK_METHOD(muse::Ret, importIntoTrack, (const muse::io::path_t&, trackedit::TrackId, muse::secs_t), (override));
    MOCK_METHOD(muse::Ret, importIntoTracks,
                (const std::vector<muse::io::path_t>&, const std::vector<trackedit::TrackId>&, muse::secs_t), (override));
    MOCK_METHOD(muse::Ret, importDroppedFiles,
                (const std::vector<muse::io::path_t>&, const std::vector<trackedit::TrackId>&,
                 const std::vector<muse::io::path_t>&, muse::secs_t), (override));
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(muse::async::Notification, aboutCloseBegin, (), (const, override));
    MOCK_METHOD(muse::async::Notification, aboutCloseEnd, (), (const, override));
    MOCK_METHOD(bool, isNewlyCreated, (), (const, override));
    MOCK_METHOD(bool, isImported, (), (const, override));
    MOCK_METHOD(QString, displayName, (), (const, override));
    MOCK_METHOD(muse::io::path_t, path, (), (const, override));
    MOCK_METHOD(muse::async::Notification, pathChanged, (), (const, override));
    MOCK_METHOD(muse::ValNt<bool>, needSave, (), (const, override));
    MOCK_METHOD(muse::Ret, canSave, (), (const, override));
    MOCK_METHOD(bool, needAutoSave, (), (const, override));
    MOCK_METHOD(void, setNeedAutoSave, (bool), (override));
    MOCK_METHOD(bool, hasUnsavedChanges, (), (override));
    MOCK_METHOD(muse::Ret, save, (const muse::io::path_t&, project::SaveMode), (override));
    MOCK_METHOD(const agu::trackedit::ITrackeditProjectPtr, trackeditProject, (), (const, override));
    MOCK_METHOD(projectscene::IProjectViewStatePtr, viewState, (), (const, override));

private:
    void createAu3Project();
};
}
