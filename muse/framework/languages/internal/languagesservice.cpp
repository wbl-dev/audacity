/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "languagesservice.h"

#include <QDir>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTranslator>


#include "multiwindows/resourcelockguard.h"

#include "global/translation.h"
#include "global/log.h"

using namespace Qt::Literals::StringLiterals;

using namespace muse;
using namespace muse::languages;

static const std::string LANGUAGES_RESOURCE_NAME("LANGUAGES");

void LanguagesService::init()
{
    TRACEFUNC;

    fileSystem()->makePath(configuration()->languagesUserAppDataPath());

    loadLanguages();

    ValCh<QString> languageCode = configuration()->currentLanguageCode();
    setCurrentLanguage(languageCode.val);

    // Remember the active language code so the "Restart required" text can go away if the language
    // is changed and later reverted in the same session. Cannot use m_currentLanguage.code
    // because m_currentLanguage holds the effective language: if the language code is "system",
    // m_currentLanguage will hold "en-us" for example (whatever the system is set to).
    QString activeLanguageCode = languageCode.val;
    languageCode.ch.onReceive(this, [this, activeLanguageCode](const QString& newLanguageCode) {
        //! NOTE To change the language at the moment, a restart is required
        m_restartRequiredToApplyLanguage = newLanguageCode != activeLanguageCode;
        m_restartRequiredToApplyLanguageChanged.send(m_restartRequiredToApplyLanguage);
    });

    m_inited = true;
}

const LanguagesHash& LanguagesService::languages() const
{
    return m_languagesHash;
}

Language LanguagesService::language(const QString& languageCode) const
{
    QString effectiveLanguageCode = this->effectiveLanguageCode(languageCode);
    if (effectiveLanguageCode.isEmpty()) {
        return {};
    }

    if (effectiveLanguageCode == PLACEHOLDER_LANGUAGE_CODE) {
        return m_placeholderLanguage;
    }

    return m_languagesHash[effectiveLanguageCode];
}

const Language& LanguagesService::currentLanguage() const
{
    return m_currentLanguage;
}

async::Notification LanguagesService::currentLanguageChanged() const
{
    return m_currentLanguageChanged;
}

const Language& LanguagesService::placeholderLanguage() const
{
    return m_placeholderLanguage;
}

bool LanguagesService::hasPlaceholderLanguage() const
{
    for (const QString& resourceName : configuration()->languageResourceNames()) {
        if (!fileSystem()->exists(configuration()->builtinLanguageFilePath(resourceName, PLACEHOLDER_LANGUAGE_CODE))) {
            return false;
        }
    }

    return true;
}

void LanguagesService::loadLanguages()
{
    TRACEFUNC;

    RetVal<ByteArray> languagesJson;
    {
        mi::ReadResourceLockGuard lock_guard(multiwindowsProvider(), LANGUAGES_RESOURCE_NAME);
        const io::path_t appLanguagesJsonPath = configuration()->appLanguagesJsonPath();
        if (!appLanguagesJsonPath.empty()) {
            languagesJson = fileSystem()->readFile(appLanguagesJsonPath);
        }

        if (!languagesJson.ret) {
            languagesJson = fileSystem()->readFile(configuration()->builtinLanguagesJsonPath());
        }
    }
    if (!languagesJson.ret) {
        LOGE() << "Failed to read languages.json: " << languagesJson.ret.toString();
        return;
    }

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(languagesJson.val.toQByteArrayNoCopy(), &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        LOGE() << "Failed to parse languages.json: " << err.errorString();
        return;
    }

    QJsonObject languagesObject = jsonDoc.object();
    for (auto it = languagesObject.begin(); it != languagesObject.end(); ++it) {
        QJsonObject langObject = it.value().toObject();

        Language lang;
        lang.code = it.key();
        lang.name = langObject.value("name").toString();
        lang.fallbackLanguages = langObject.value("fallbackLanguages").toVariant().toStringList();

        m_languagesHash.insert(it.key(), lang);
    }

    m_placeholderLanguage.code = PLACEHOLDER_LANGUAGE_CODE;
    m_placeholderLanguage.name = "«Placeholder translations»";
}

void LanguagesService::setCurrentLanguage(const QString& languageCode)
{
    TRACEFUNC;

    QString effectiveLanguageCode = this->effectiveLanguageCode(languageCode);
    if (effectiveLanguageCode.isEmpty()) {
        LOGE() << "Unsupported language code: " << languageCode;
        return;
    }

    if (effectiveLanguageCode == m_currentLanguage.code) {
        return;
    }

    Language& lang = languageCode == PLACEHOLDER_LANGUAGE_CODE
                     ? m_placeholderLanguage
                     : m_languagesHash[effectiveLanguageCode];

    if (!lang.isLoaded()) {
        loadLanguage(lang);
    }

    for (QTranslator* t : std::as_const(m_translators)) {
        QCoreApplication::instance()->removeTranslator(t);
        delete t;
    }
    m_translators.clear();
    m_translators.reserve(lang.files.size());

    auto installTranslatorsForLanguage = [this](const Language& l) {
        for (const io::path_t& file : l.files) {
            QTranslator* translator = new QTranslator();
            bool ok = translator->load(file.toQString());
            if (ok) {
                QCoreApplication::instance()->installTranslator(translator);
                m_translators.push_back(translator);
            } else {
                LOGE() << "Error loading translator " << file.toQString();
                delete translator;
            }
        }
    };

    // Install translators for fallback languages in reverse order:
    // Qt searches the most recently installed translator first,
    // and the least recently installed translator last.
    for (auto it = lang.fallbackLanguages.crbegin(); it != lang.fallbackLanguages.crend(); ++it) {
        Language& fallbackLang = m_languagesHash[*it];
        installTranslatorsForLanguage(fallbackLang);
    }

    installTranslatorsForLanguage(lang);

    QLocale locale(lang.code);
    QLocale::setDefault(locale);
    qGuiApp->setLayoutDirection(locale.textDirection());

    lang.direction = locale.textDirection();

    // Currently, no need to retranslate the UI on language change, because we require restart

    m_currentLanguage = lang;
    m_currentLanguageChanged.notify();
}

QString LanguagesService::effectiveLanguageCode(QString languageCode) const
{
    languageCode.replace(u'-', u'_');

    auto tryCode = [this](const QString& code) -> QString {
        if (m_languagesHash.contains(code)) {
            return code;
        }

        static const std::map<QString, QString> SPECIAL_CASES {
            { u"ca_valencia"_s, u"ca@valencia"_s },
            { u"ca_ES_valencia"_s, u"ca@valencia"_s },
            { u"en_AU"_s, u"en_GB"_s },
            { u"en_NZ"_s, u"en_GB"_s },
            { u"en"_s, u"en_US"_s },
            { u"hi"_s, u"hi_IN"_s },
            { u"mn"_s, u"mn_MN"_s },
            { u"zh"_s, u"zh_CN"_s }
        };

        auto it = SPECIAL_CASES.find(code);
        if (it != SPECIAL_CASES.cend()) {
            return it->second;
        }

        // Not found
        return QString();
    };

    if (languageCode.isEmpty() || languageCode == SYSTEM_LANGUAGE_CODE) {
        const QStringList systemLanguages = QLocale::system().uiLanguages();
        LOGI() << "System languages: " << systemLanguages;

        for (QString code : systemLanguages) {
            code.replace(u'-', u'_');

            // Prefer Swedish (Modern) over Swedish (Traditional)
            // when using system language.
            if (code == u"sv_SE"_s) {
                code = u"sv"_s;
            }

            code = tryCode(code);
            if (!code.isEmpty()) {
                return code;
            }
        }

        // Not found
        return QString();
    }

    if (languageCode == PLACEHOLDER_LANGUAGE_CODE) {
        return languageCode;
    }

    return tryCode(languageCode);
}

Ret LanguagesService::loadLanguage(Language& lang)
{
    Ret ret = doLoadLanguage(lang);
    if (!ret) {
        LOGE() << "Failed to load language " << lang.code << ": " << ret.toString();
        return ret;
    }

    for (const QString& fallbackCode : lang.fallbackLanguages) {
        Language& fallbackLang = m_languagesHash[fallbackCode];

        if (!fallbackLang.isLoaded()) {
            ret = doLoadLanguage(fallbackLang);
            if (!ret) {
                LOGE() << "Failed to load fallback language " << fallbackLang.code << ": " << ret.toString();
                return ret;
            }
        }
    }

    return muse::make_ok();
}

Ret LanguagesService::doLoadLanguage(Language& lang)
{
    io::path_t languagesAppDataPath = configuration()->languagesAppDataPath();
    io::path_t languagesUserAppDataPath = configuration()->languagesUserAppDataPath();

    QString languageSuffix = QString("_%1.qm").arg(lang.code);

    std::string filter = ("*" + languageSuffix).toStdString();

    RetVal<io::paths_t> appFilePaths = fileSystem()->scanFiles(languagesAppDataPath, { filter });
    if (!appFilePaths.ret) {
        LOGE() << "Failed to scan files for language " << lang.code << ": " << appFilePaths.ret.toString();
        return appFilePaths.ret;
    }

    for (const io::path_t& appFilePath : appFilePaths.val) {
        io::path_t filename = io::filename(appFilePath);
        io::path_t userFilePath = languagesUserAppDataPath.appendingComponent(filename);

        QFileInfo appFileInfo(appFilePath.toQString());
        QFileInfo userFileInfo(userFilePath.toQString());
        bool useUserPath = userFileInfo.exists() && userFileInfo.lastModified() > appFileInfo.lastModified();

        QString resourceName = filename.toQString().chopped(languageSuffix.size());
        lang.files[resourceName] = useUserPath ? userFilePath : appFilePath;
    }

    for (const QString& resourceName : configuration()->languageResourceNames()) {
        if (!lang.files.contains(resourceName)) {
            LOGE() << "Could not find resource " << resourceName << " for language " << lang.code;
            return muse::make_ret(muse::Ret::Code::InternalError);
        }
    }

    return muse::make_ok();
}

bool LanguagesService::restartRequiredToApplyLanguage() const
{
    return m_restartRequiredToApplyLanguage;
}

async::Channel<bool> LanguagesService::restartRequiredToApplyLanguageChanged() const
{
    return m_restartRequiredToApplyLanguageChanged;
}

