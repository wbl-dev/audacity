/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Audacity-CLA-applies
 *
 * Audacity
 * Music Composition & Notation
 *
 * Copyright (C) 2024 Audacity BVBA and others
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
#ifndef AU_PROJECT_PROJECTTYPES_H
#define AU_PROJECT_PROJECTTYPES_H

#include <filesystem>
#include <optional>

#include <QString>
#include <QUrl>

#include "io/path.h"
#include "progress.h"
#include "log.h"

#include "projectmeta.h"


namespace au::project {
struct ProjectCreateOptions
{
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;

    muse::io::path_t templatePath;
};

enum class SaveMode
{
    Save,
    SaveAs,
    SaveCopy,
    SaveSelection,
    AutoSave
};

struct ProjectFile {
    QUrl url;
    QString displayNameOverride = {};

    ProjectFile() = default;

    ProjectFile(const QUrl& url, const QString& displayNameOverride = {})
        : url(url), displayNameOverride(displayNameOverride) {}

    ProjectFile(const muse::io::path_t& path, const QString& displayNameOverride = {})
        : url(path.toQUrl()), displayNameOverride(displayNameOverride) {}

    bool isNull() const
    {
        return url.isEmpty();
    }

    bool isValid() const
    {
        return url.isValid();
    }

    bool hasDisplayName() const
    {
        if (!displayNameOverride.isEmpty()) {
            return true;
        }

        return url.isLocalFile();
    }

    QString displayName(bool includingExtension) const
    {
        if (!displayNameOverride.isEmpty()) {
            return displayNameOverride;
        }

        return muse::io::filename(path(), includingExtension).toQString();
    }

    muse::io::path_t path() const
    {
        assert(url.isEmpty() || url.isLocalFile());

        return muse::io::path_t(url);
    }

    bool operator ==(const ProjectFile& other) const
    {
        return url == other.url
               && displayNameOverride == other.displayNameOverride;
    }

    bool operator !=(const ProjectFile& other) const
    {
        return !(*this == other);
    }
};

using ProjectFilesList = std::vector<ProjectFile>;

struct RecentFile {
    muse::io::path_t path;
    QString displayNameOverride = {};

    RecentFile() = default;

    RecentFile(const muse::io::path_t& path, const QString& displayNameOverride = {})
        : path(path), displayNameOverride(displayNameOverride) {}

    static RecentFile fromProjectFile(const ProjectFile& projectFile)
    {
        return RecentFile(projectFile.path(), projectFile.displayNameOverride);
    }

    ProjectFile toProjectFile() const
    {
        return ProjectFile(path, displayNameOverride);
    }

    bool isValid() const
    {
        return !path.empty();
    }

    QString displayName(bool includingExtension) const
    {
        if (!displayNameOverride.isEmpty()) {
            return displayNameOverride;
        }

        return muse::io::filename(path, includingExtension).toQString();
    }

    bool operator ==(const RecentFile& other) const
    {
        return path == other.path
               && displayNameOverride == other.displayNameOverride;
    }

    bool operator !=(const RecentFile& other) const
    {
        return !(*this == other);
    }
};

using RecentFilesList = std::vector<RecentFile>;

struct Template
{
    QString categoryTitle;
    ProjectMeta meta;
    bool isCustom = false;
};

using Templates = QList<Template>;

class GenerateAudioTimePeriod
{
    Q_GADGET

public:
    enum class Type {
        Never = 0,
        Always,
        AfterCertainNumberOfSaves
    };
    Q_ENUM(Type)
};

using GenerateAudioTimePeriodType = GenerateAudioTimePeriod::Type;

static const std::string AUP3 = "aup3";
static const std::string AUP4 = "aup4";
static const std::string AUP4UNSAVED = "aup4unsaved";

inline bool isAudacity3File(const muse::io::path_t& filename)
{
    return muse::io::suffix(filename) == AUP3;
}

inline bool isAudacity3FileType(const std::string_view ext)
{
    return ext == AUP3;
}

inline bool isAudacity4File(const muse::io::path_t& filename)
{
    return muse::io::suffix(filename) == AUP4;
}

inline bool isAudacity4FileType(const std::string_view ext)
{
    return ext == AUP4;
}

inline bool isAudacityUnsavedFile(const muse::io::path_t& filename)
{
    return muse::io::suffix(filename) == AUP4UNSAVED;
}

inline bool isAudacityUnsavedFileType(const std::string_view ext)
{
    return ext == AUP4UNSAVED;
}

inline bool isSameProjectFile(const muse::io::path_t& lhs, const muse::io::path_t& rhs)
{
    if (lhs == rhs) {
        return true;
    }

    // Different paths can name one file: other case on a case-insensitive volume, a link, or a Windows short name
    std::error_code ec;
    return std::filesystem::equivalent(std::filesystem::path(lhs.toQString().toStdU16String()),
                                       std::filesystem::path(rhs.toQString().toStdU16String()), ec);
}

inline bool isAudacityFile(const muse::io::path_t& filename)
{
    return isAudacity3File(filename) || isAudacity4File(filename) || isAudacityUnsavedFile(filename);
}

inline bool isAudacityFileType(const std::string_view ext)
{
    return isAudacity3FileType(ext) || isAudacity4FileType(ext) || isAudacityUnsavedFileType(ext);
}
}

#endif // AU_PROJECT_PROJECTTYPES_H
