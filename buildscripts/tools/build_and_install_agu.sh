#!/usr/bin/env bash

# Build the macOS app and install it where Spotlight can find it.
set -euo pipefail

if [[ "$(uname -s)" != Darwin ]]; then
    echo "This script is for macOS only." >&2
    exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"
build_dir="${AGU_BUILD_DIR:-${repo_root}/build/workbench-debug}"
install_dir="${AGU_INSTALL_DIR:-/Applications}"
source_app="${build_dir}/src/app/AGu.app"
destination="${install_dir}/AGu.app"

if [[ ! -f "${build_dir}/CMakeCache.txt" ]]; then
    echo "Build directory is not configured: ${build_dir}" >&2
    echo "Run: cmake --preset workbench-debug" >&2
    exit 1
fi

echo "Building AGu in ${build_dir}..."
cmake --build "${build_dir}" --target agu --parallel "${AGU_BUILD_JOBS:-4}"

if [[ ! -x "${source_app}/Contents/MacOS/AGu" ]]; then
    echo "Built app is missing: ${source_app}" >&2
    exit 1
fi

if [[ ! -d "${install_dir}" || ! -w "${install_dir}" ]]; then
    echo "Installation directory is unavailable or not writable: ${install_dir}" >&2
    echo "Set AGU_INSTALL_DIR to a writable Applications directory." >&2
    exit 1
fi

if pgrep -x AGu >/dev/null; then
    echo "Asking AGu to quit before installing..."
    if ! osascript -e 'tell application id "org.agu.app" to quit'; then
        echo "Please quit AGu after saving your project." >&2
    fi

    for ((attempt = 0; attempt < 120; attempt++)); do
        if ! pgrep -x AGu >/dev/null; then
            break
        fi
        sleep 1
    done

    if pgrep -x AGu >/dev/null; then
        echo "AGu is still running; installation was not changed." >&2
        exit 1
    fi
fi

stage_dir="$(mktemp -d "${install_dir}/.agu-install.XXXXXX")"
staged_app="${stage_dir}/AGu.app"
backup_app="${stage_dir}/Previous.app"

cleanup() {
    status=$?
    trap - EXIT
    if [[ ${status} -ne 0 && -d "${backup_app}" && ! -e "${destination}" ]]; then
        mv "${backup_app}" "${destination}" || true
    fi
    rm -rf "${stage_dir}"
    exit "${status}"
}
trap cleanup EXIT

echo "Copying app to ${install_dir}..."
ditto "${source_app}" "${staged_app}"
codesign --verify --deep --strict "${staged_app}"

if [[ -e "${destination}" ]]; then
    mv "${destination}" "${backup_app}"
fi
mv "${staged_app}" "${destination}"
touch "${destination}"

# Register the new bundle with macOS, then ask Spotlight to index it.
lsregister=/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister
if [[ -x "${lsregister}" ]]; then
    # Spotlight can still find the build bundle after it is unregistered,
    # leaving that result without an icon. Register both, with the installed
    # copy last so it remains the preferred application for this bundle ID.
    if ! "${lsregister}" -f "${source_app}"; then
        echo "Warning: macOS did not register the build bundle automatically." >&2
    fi
    if ! "${lsregister}" -f "${destination}"; then
        echo "Warning: macOS did not register the app automatically." >&2
    fi
fi
mdimport "${source_app}" >/dev/null 2>&1 || true
mdimport "${destination}" >/dev/null 2>&1 || true
echo "Installed ${destination}"
