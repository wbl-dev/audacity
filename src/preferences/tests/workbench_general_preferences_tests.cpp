/*
 * Audacity: A Digital Audio Editor
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "framework/stubs/update/updateconfigurationstub.h"
#include "languages/ilanguagesconfiguration.h"
#include "languages/ilanguagesservice.h"
#include "modularity/ioc.h"
#include "muse_framework_config.h"
#include "stubs/usageinfo/usageinfostub.h"

#include "../qml/Audacity/Preferences/generalpreferencesmodel.h"
#include "../qml/Audacity/Preferences/updatepreferencesmodel.h"
#include "../qml/Audacity/Preferences/usageinfopreferencesmodel.h"

namespace au::appshell {
namespace {

class LanguagesConfigurationMock : public muse::languages::ILanguagesConfiguration
{
public:
    MOCK_METHOD((muse::ValCh<QString>), currentLanguageCode, (), (const, override));
    MOCK_METHOD(void, setCurrentLanguageCode, (const QString& languageCode), (const, override));
    MOCK_METHOD(QUrl, languagesUpdateUrl, (), (const, override));
    MOCK_METHOD(QUrl, languageFileServerUrl, (const QString& languageCode), (const, override));
    MOCK_METHOD(muse::io::path_t, languagesAppDataPath, (), (const, override));
    MOCK_METHOD(muse::io::path_t, languagesUserAppDataPath, (), (const, override));
    MOCK_METHOD(muse::io::path_t, appLanguagesJsonPath, (), (const, override));
    MOCK_METHOD(muse::io::path_t, builtinLanguagesJsonPath, (), (const, override));
    MOCK_METHOD(muse::io::path_t, builtinLanguageFilePath,
                (const QString& resourceName, const QString& languageCode), (const, override));
    MOCK_METHOD(muse::io::path_t, userLanguageFilePath,
                (const QString& resourceName, const QString& languageCode), (const, override));
    MOCK_METHOD(QStringList, languageResourceNames, (), (const, override));
};

class LanguagesServiceMock : public muse::languages::ILanguagesService
{
public:
    MOCK_METHOD((const muse::languages::LanguagesHash&), languages, (), (const, override));
    MOCK_METHOD(muse::languages::Language, language, (const QString& languageCode), (const, override));
    MOCK_METHOD((const muse::languages::Language&), currentLanguage, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentLanguageChanged, (), (const, override));
    MOCK_METHOD(bool, hasPlaceholderLanguage, (), (const, override));
    MOCK_METHOD((const muse::languages::Language&), placeholderLanguage, (), (const, override));
    MOCK_METHOD(muse::Progress, update, (const QString& languageCode), (override));
    MOCK_METHOD(bool, restartRequiredToApplyLanguage, (), (const, override));
    MOCK_METHOD((muse::async::Channel<bool>), restartRequiredToApplyLanguageChanged, (), (const, override));
};

class workbench_general_preferences : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_languagesConfiguration = std::make_shared<::testing::NiceMock<LanguagesConfigurationMock> >();
        m_languagesService = std::make_shared<::testing::NiceMock<LanguagesServiceMock> >();
        m_updateConfiguration = std::make_shared<muse::update::UpdateConfigurationStub>();
        m_usageInfo = std::make_shared<au::usageinfo::UsageInfoStub>();

        auto* ioc = muse::modularity::globalIoc();
        ioc->registerExport<muse::languages::ILanguagesConfiguration>("preferences_tests", m_languagesConfiguration);
        ioc->registerExport<muse::languages::ILanguagesService>("preferences_tests", m_languagesService);
        ioc->registerExport<muse::update::IUpdateConfiguration>("preferences_tests", m_updateConfiguration);
        ioc->registerExport<au::usageinfo::IUsageInfo>("preferences_tests", m_usageInfo);
    }

    void TearDown() override
    {
        auto* ioc = muse::modularity::globalIoc();
        ioc->unregister<muse::languages::ILanguagesConfiguration>("preferences_tests");
        ioc->unregister<muse::languages::ILanguagesService>("preferences_tests");
        ioc->unregister<muse::update::IUpdateConfiguration>("preferences_tests");
        ioc->unregister<au::usageinfo::IUsageInfo>("preferences_tests");
    }

    std::shared_ptr<LanguagesConfigurationMock> m_languagesConfiguration;
    std::shared_ptr<LanguagesServiceMock> m_languagesService;
    std::shared_ptr<muse::update::UpdateConfigurationStub> m_updateConfiguration;
    std::shared_ptr<au::usageinfo::UsageInfoStub> m_usageInfo;
    muse::languages::LanguagesHash m_languages;
};

TEST_F(workbench_general_preferences, hides_online_sections_but_keeps_crash_policy)
{
    GeneralPreferencesModel generalModel;
    UpdatePreferencesModel updateModel;
    UsageInfoPreferencesModel usageInfoModel;

    EXPECT_FALSE(generalModel.onlineLanguageUpdatesAvailable());
    EXPECT_FALSE(generalModel.nonDiagnosticOnlineFeaturesAvailable());
    EXPECT_FALSE(updateModel.isAppUpdatable());
    EXPECT_FALSE(usageInfoModel.isUsageInfoAvailable());
#ifdef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
    EXPECT_TRUE(usageInfoModel.isCrashReportingAvailable());
#else
    EXPECT_FALSE(usageInfoModel.isCrashReportingAvailable());
#endif
}

TEST_F(workbench_general_preferences, selecting_bundled_language_does_not_request_online_update)
{
    muse::languages::Language english {
        .code = "en_US",
        .name = "English",
        .files = { { "audacity", "/bundle/audacity_en_US.qm" } },
    };
    muse::languages::Language chinese {
        .code = "zh_CN",
        .name = "简体中文",
        .files = { { "audacity", "/bundle/audacity_zh_CN.qm" } },
    };
    m_languages.insert(english.code, english);
    m_languages.insert(chinese.code, chinese);

    muse::ValCh<QString> currentLanguage;
    currentLanguage.val = english.code;
    ON_CALL(*m_languagesConfiguration, currentLanguageCode()).WillByDefault(::testing::Return(currentLanguage));
    ON_CALL(*m_languagesService, languages()).WillByDefault(::testing::ReturnRef(m_languages));
    ON_CALL(*m_languagesService, hasPlaceholderLanguage()).WillByDefault(::testing::Return(false));

    EXPECT_CALL(*m_languagesConfiguration, setCurrentLanguageCode(chinese.code)).Times(1);
    EXPECT_CALL(*m_languagesService, update(::testing::_)).Times(0);

    GeneralPreferencesModel model;
    const QVariantList catalog = model.languages();
    ASSERT_EQ(catalog.size(), 3);

    model.setCurrentLanguageCode(chinese.code);
    model.checkUpdateForCurrentLanguage();
}

} // namespace
} // namespace au::appshell
