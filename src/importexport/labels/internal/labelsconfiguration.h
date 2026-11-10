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
    muse::Ret saveParserProfile(LabelParserProfile profile, bool allowBuiltinOverride = false) override;
    muse::Ret removeParserProfile(const QString& id) override;

    muse::io::path_t labelsDirectoryPath() const override;
    void setLabelsDirectoryPath(const muse::io::path_t& path) override;

private:
    std::vector<LabelParserProfile> readProfiles(bool* valid = nullptr) const;
    muse::Ret writeProfiles(const std::vector<LabelParserProfile>& profiles) const;
    QString profileFilePath() const;

    QString m_profileFilePath;
};
}
