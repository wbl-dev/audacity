#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QUrl>

#include "projectscene/view/tracksitemsview/dropcontroller.h"
#include "context/tests/mocks/globalcontextmock.h"
#include "project/tests/mocks/audacityprojectmock.h"

using namespace ::testing;

namespace au::projectscene {
namespace {
class ImporterMock : public importexport::IImporter
{
public:
    MOCK_METHOD(importexport::FileInfo, fileInfo, (const muse::io::path_t&), (override));
    MOCK_METHOD(bool, import, (const muse::io::path_t&), (override));
    MOCK_METHOD(bool, importIntoTrack, (const muse::io::path_t&, trackedit::TrackId, trackedit::secs_t), (override));
    MOCK_METHOD(bool, importFromSystemClipboard, (const std::vector<muse::io::path_t>&, trackedit::secs_t), (override));
    MOCK_METHOD(std::vector<std::string>, supportedExtensions, (), (const, override));
};

class LabelsImporterMock : public importexport::ILabelsImporter
{
public:
    MOCK_METHOD(muse::Ret, importData, (const muse::io::path_t&), (override));
    MOCK_METHOD(std::vector<std::string>, supportedExtensions, (), (const, override));
};
}

class DropControllerTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_controller = std::make_unique<DropController>();
        m_importer = std::make_shared<NiceMock<ImporterMock> >();
        m_labelsImporter = std::make_shared<NiceMock<LabelsImporterMock> >();
        m_context = std::make_shared<NiceMock<context::GlobalContextMock> >();
        m_project = std::make_shared<NiceMock<project::AudacityProjectMock> >();
        m_controller->importer.set(m_importer);
        m_controller->labelsImporter.set(m_labelsImporter);
        m_controller->globalContext.set(m_context);
        ON_CALL(*m_context, currentProject()).WillByDefault(Return(m_project));
        ON_CALL(*m_importer, supportedExtensions()).WillByDefault(Return(std::vector<std::string> { "wav", "txt", "ann" }));
        ON_CALL(*m_labelsImporter, supportedExtensions()).WillByDefault(Return(std::vector<std::string> { "txt", "ann" }));
    }

    std::unique_ptr<DropController> m_controller;
    std::shared_ptr<NiceMock<ImporterMock> > m_importer;
    std::shared_ptr<NiceMock<LabelsImporterMock> > m_labelsImporter;
    std::shared_ptr<NiceMock<context::GlobalContextMock> > m_context;
    std::shared_ptr<NiceMock<project::AudacityProjectMock> > m_project;
};

TEST_F(DropControllerTests, CustomLabelDropDoesNotReserveAudioTrack)
{
    const auto labelUrl = QUrl::fromLocalFile("/tmp/example.ann").toString();
    EXPECT_CALL(*m_importer, fileInfo(_)).Times(0);
    m_controller->probeAudioFiles({ labelUrl });
    EXPECT_EQ(m_controller->requiredTracksCount(), 0);
    EXPECT_CALL(*m_project, importDroppedFiles(IsEmpty(), IsEmpty(),
                                               ElementsAre(muse::io::path_t("/tmp/example.ann")), _)).WillOnce(Return(muse::make_ok()));
    m_controller->handleDroppedFiles({}, 0.0);
}

TEST_F(DropControllerTests, MixedAudioAndLabelDropUsesSeparatePaths)
{
    const auto audioUrl = QUrl::fromLocalFile("/tmp/example.wav").toString();
    const auto labelUrl = QUrl::fromLocalFile("/tmp/example.TXT").toString();
    EXPECT_CALL(*m_importer, fileInfo(muse::io::path_t("/tmp/example.wav")))
    .WillOnce(Return(importexport::FileInfo { "/tmp/example.wav", 3.0, 1 }));
    m_controller->probeAudioFiles({ audioUrl, labelUrl });
    EXPECT_EQ(m_controller->requiredTracksCount(), 1);
    EXPECT_CALL(*m_project, importDroppedFiles(
                    ElementsAre(muse::io::path_t("/tmp/example.wav")), ElementsAre(7),
                    ElementsAre(muse::io::path_t("/tmp/example.TXT")), _)).WillOnce(Return(muse::make_ok()));
    m_controller->handleDroppedFiles({ 7 }, 0.0);
}
}
