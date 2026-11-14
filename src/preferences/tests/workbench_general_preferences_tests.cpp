/*
 * Audacity: A Digital Audio Editor
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "languages/ilanguagesconfiguration.h"
#include "languages/ilanguagesservice.h"
#include "modularity/ioc.h"
#include "muse_framework_config.h"

#include "../qml/Audacity/Preferences/generalpreferencesmodel.h"
#include "../qml/Audacity/Preferences/crashreportspreferencesmodel.h"

namespace au::appshell {
namespace {

class LanguagesConfigurationMock : public muse::languages::ILanguagesConfiguration
{
public:
    MOCK_METHOD((muse::ValCh<QString>), currentLanguageCode, (), (const, override));
    MOCK_METHOD(void, setCurrentLanguageCode, (const QString& languageCode), (const, override));
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

        auto* ioc = muse::modularity::globalIoc();
        ioc->registerExport<muse::languages::ILanguagesConfiguration>("preferences_tests", m_languagesConfiguration);
        ioc->registerExport<muse::languages::ILanguagesService>("preferences_tests", m_languagesService);
    }

    void TearDown() override
    {
        auto* ioc = muse::modularity::globalIoc();
        ioc->unregister<muse::languages::ILanguagesConfiguration>("preferences_tests");
        ioc->unregister<muse::languages::ILanguagesService>("preferences_tests");
    }

    std::shared_ptr<LanguagesConfigurationMock> m_languagesConfiguration;
    std::shared_ptr<LanguagesServiceMock> m_languagesService;
    muse::languages::LanguagesHash m_languages;
};

TEST_F(workbench_general_preferences, keeps_crash_policy)
{
    CrashReportsPreferencesModel crashReportsModel;

#ifdef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
    EXPECT_TRUE(crashReportsModel.isCrashReportingAvailable());
#else
    EXPECT_FALSE(crashReportsModel.isCrashReportingAvailable());
#endif
}

TEST_F(workbench_general_preferences, selects_bundled_language)
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

    GeneralPreferencesModel model;
    const QVariantList catalog = model.languages();
    ASSERT_EQ(catalog.size(), 3);

    model.setCurrentLanguageCode(chinese.code);
}

} // namespace
} // namespace au::appshell
