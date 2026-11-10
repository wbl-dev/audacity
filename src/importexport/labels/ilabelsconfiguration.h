/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "io/path.h"

#include "modularity/imoduleinterface.h"

#include "labelstypes.h"
#include "labelparserprofile.h"
#include "types/ret.h"

namespace au::importexport {
class ILabelsConfiguration : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(ILabelsConfiguration)

public:
    virtual ~ILabelsConfiguration() = default;

    virtual std::vector<FileFilter> fileFilter() const = 0;
    virtual std::vector<FileFilter> importFileFilter() const = 0;
    virtual std::vector<LabelParserProfile> parserProfiles() const = 0;
    virtual muse::Ret saveParserProfile(LabelParserProfile profile, bool allowBuiltinOverride = false) = 0;
    virtual muse::Ret removeParserProfile(const QString& id) = 0;

    virtual muse::io::path_t labelsDirectoryPath() const = 0;
    virtual void setLabelsDirectoryPath(const muse::io::path_t& path) = 0;
};
}
