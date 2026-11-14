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

#pragma once

#include "ilanguagesservice.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ilanguagesconfiguration.h"
#include "io/ifilesystem.h"
#include "multiwindows/imultiwindowsprovider.h"


class QJsonObject;
class QTranslator;

namespace muse::languages {
class LanguagesService : public ILanguagesService, public Contextable, public async::Asyncable
{
    GlobalInject<ILanguagesConfiguration> configuration;
    GlobalInject<io::IFileSystem> fileSystem;
    GlobalInject<mi::IMultiWindowsProvider> multiwindowsProvider;

public:
    LanguagesService(const modularity::ContextPtr& iocCtx)
        : Contextable(iocCtx) {}

    void init();

    const LanguagesHash& languages() const override;
    Language language(const QString& languageCode) const override;
    const Language& currentLanguage() const override;
    async::Notification currentLanguageChanged() const override;

    bool hasPlaceholderLanguage() const override;
    const Language& placeholderLanguage() const override;


    bool restartRequiredToApplyLanguage() const override;
    async::Channel<bool> restartRequiredToApplyLanguageChanged() const override;

private:
    void loadLanguages();

    void setCurrentLanguage(const QString& languageCode);
    QString effectiveLanguageCode(QString languageCode) const;

    Ret loadLanguage(Language& lang);
    Ret doLoadLanguage(Language& lang);

private:
    LanguagesHash m_languagesHash;
    Language m_currentLanguage;
    async::Notification m_currentLanguageChanged;
    Language m_placeholderLanguage;

    std::vector<QTranslator*> m_translators;

    bool m_inited = false;
    bool m_restartRequiredToApplyLanguage = false;
    async::Channel<bool> m_restartRequiredToApplyLanguageChanged;

};
}
