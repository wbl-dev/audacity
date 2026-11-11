/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"

#include "../ilabelsconfiguration.h"
#include <QString>

namespace au::importexport {
class LabelsConfiguration : public ILabelsConfiguration
{
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;

public:
    explicit LabelsConfiguration(QString profileFilePath = {});

    void init();

    std::vector<FileFilter> fileFilter() const override;
    std::vector<FileFilter> importFileFilter() const override;
    std::vector<LabelParserProfile> parserProfiles() const override;
    QString parserScriptsDirectoryPath() const override;
    muse::Ret reloadParserProfiles() override;
    muse::Ret saveParserProfile(LabelParserProfile profile, bool allowBuiltinOverride = false) override;
    muse::Ret removeParserProfile(const QString& id) override;

    muse::io::path_t labelsDirectoryPath() const override;
    void setLabelsDirectoryPath(const muse::io::path_t& path) override;

private:
    std::vector<LabelParserProfile> readProfiles(bool* valid = nullptr) const;
    QString profileFilePath() const;
    muse::Ret migrateLegacyProfiles(const QString& directoryPath);
    muse::Ret writeProfileScript(const QString& filePath, const LabelParserProfile& profile) const;

    QString m_profileFilePath;
    std::vector<LabelParserProfile> m_profiles;
    bool m_profilesLoaded = false;
};
}
