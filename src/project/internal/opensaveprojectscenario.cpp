/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Audacity-CLA-applies
 *
 * Audacity
 * A Digital Audio Editor
 *
 * Copyright (C) 2024 Audacity Limited
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

#include "opensaveprojectscenario.h"

#include "interactive/iinteractive.h"
#include "translation.h"

using namespace muse;
using namespace au::project;

RetVal<muse::io::path_t> OpenSaveProjectScenario::askLocalPath(IAudacityProjectPtr project, SaveMode saveMode) const
{
    std::string dialogTitle = muse::trc("project/save", "Save project");
    std::string filenameAddition;

    if (saveMode == SaveMode::SaveCopy) {
        //: used to form a filename suggestion, like "originalFile - copy"
        filenameAddition = " - " + muse::trc("project/save", "copy", "a copy of a file");
    } else if (saveMode == SaveMode::SaveSelection) {
        //: used to form a filename suggestion, like "originalFile - selection"
        filenameAddition = " - " + muse::trc("project/save", "selection");
    }

    muse::io::path_t defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition);

    std::vector<std::string> filter {
        muse::trc("project", "Audacity 4 files") + " (*.aup4)"
    };

    muse::io::path_t selectedPath = interactive()->selectSavingFileSync(dialogTitle, defaultPath, filter);

    if (selectedPath.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    // force save to aup4 format
    std::string suffix = muse::io::suffix(selectedPath);
    std::string correctedPath = selectedPath.toStdString();
    if (!suffix.empty()) {
        correctedPath = correctedPath.substr(0, correctedPath.size() - (suffix.size() + 1));
    }

    // check if there's a dot at the end; add one if not
    if (!correctedPath.empty() && correctedPath.back() != '.') {
        correctedPath += ".";
    }
    correctedPath += "aup4";

    const bool suffixCorrected = correctedPath != selectedPath.toStdString();
    selectedPath = correctedPath;

    // Another spelling of this project's own file is a plain save
    if (isSameProjectFile(project->path(), selectedPath)) {
        selectedPath = project->path();
    }

    if (selectedPath != project->path() && multiwindowsProvider()->isProjectAlreadyOpened(selectedPath)) {
        interactive()->errorSync(muse::trc("project-file-io", "Error Saving Project"),
                                 muse::trc("project/save",
                                           "The project was not saved because the selected project is open in another window.\n"
                                           "Please try again and select an original name."));
        return askLocalPath(project, saveMode);
    }

    // The file dialog asked about the name before the suffix was corrected
    if (suffixCorrected && selectedPath != project->path() && fileSystem()->exists(selectedPath)) {
        const int overwriteBtn = int(IInteractive::Button::CustomButton) + 1;
        const IInteractive::ButtonData overwriteBtnData(overwriteBtn, muse::trc("export", "Overwrite"));
        IInteractive::Result result = interactive()->questionSync(io::filename(selectedPath).toStdString(),
                                                                  muse::trc("export", "Do you want to overwrite?"),
                                                                  { overwriteBtnData,
                                                                    interactive()->buttonData(IInteractive::Button::Cancel) });
        if (result.button() != overwriteBtn) {
            return askLocalPath(project, saveMode);
        }
    }

    configuration()->setLastSavedProjectsPath(io::dirpath(selectedPath));

    return RetVal<muse::io::path_t>::make_ok(selectedPath);
}

muse::RetVal<muse::io::path_t> OpenSaveProjectScenario::resolveLegacyProjectFormat(const muse::io::path_t& path) const
{
    auto ret = interactive()->warningSync(muse::trc("project/open", "Audacity 3 project"),
                                          muse::trc(
                                              "project/open",
                                              "You have opened an Audacity 3 project. It must be converted before you can use it in Audacity 4."),
    {
        muse::IInteractive::ButtonData(
            muse::IInteractive::Button::Cancel, muse::trc("project/open", "Cancel"),
            false),
        muse::IInteractive::ButtonData(
            muse::IInteractive::Button::Apply, muse::trc("project/open", "Save as new project"), true)
    },
                                          int(muse::IInteractive::Button::NoButton), { muse::IInteractive::Option::WithIcon },
                                          muse::trc("project/open", "Legacy project"));

    if (ret.standardButton() != muse::IInteractive::Button::Apply) {
        return muse::make_ret(Ret::Code::Cancel);
    }

    std::string s = path.toStdString();
    auto pos = s.rfind(au::project::AUP3);
    if (pos != std::string::npos) {
        s.replace(pos, 4, au::project::AUP4);
    }

    muse::io::path_t newFilename = s;

    int counter = 1;
    while (fileSystem()->exists(newFilename)) {
        std::string base = s.substr(0, s.size() - 5);
        std::string numbered = base + "-" + std::to_string(counter) + "." + au::project::AUP4;
        newFilename = numbered;
        ++counter;
    }

    fileSystem()->copy(path, newFilename);

    interactive()->infoSync(muse::trc("project/open", ""),
                            muse::mtrc("project/open", "The project was saved as “%1”").arg(newFilename.toString()).toStdString(),
    {
        muse::IInteractive::ButtonData(
            muse::IInteractive::Button::Ok, muse::trc("project/open", "Continue"), false)
    },
                            int(muse::IInteractive::Button::NoButton), { muse::IInteractive::Option::WithIcon },
                            muse::trc("project/open", "Project saved"));
    return RetVal<muse::io::path_t>::make_ok(newFilename);
}
