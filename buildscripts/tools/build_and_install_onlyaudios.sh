#!/usr/bin/env bash

# Build the macOS app and install it where Spotlight can find it.
set -euo pipefail

if [[ "$(uname -s)" != Darwin ]]; then
    echo "This script is for macOS only." >&2
    exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"
build_dir="${ONLYAUDIOS_BUILD_DIR:-${repo_root}/build/audacity-debug}"
install_dir="${ONLYAUDIOS_INSTALL_DIR:-/Applications}"
source_app="${build_dir}/src/app/OnlyAudios.app"
destination="${install_dir}/OnlyAudios.app"

if [[ ! -f "${build_dir}/CMakeCache.txt" ]]; then
    echo "Build directory is not configured: ${build_dir}" >&2
    echo "Run: cmake --preset audacity-debug" >&2
    exit 1
fi

echo "Building OnlyAudios in ${build_dir}..."
cmake --build "${build_dir}" --target audacity --parallel "${ONLYAUDIOS_BUILD_JOBS:-4}"

if [[ ! -x "${source_app}/Contents/MacOS/OnlyAudios" ]]; then
    echo "Built app is missing: ${source_app}" >&2
    exit 1
fi

if pgrep -x OnlyAudios >/dev/null; then
    echo "Quit OnlyAudios before installing an update." >&2
    exit 1
fi

if [[ ! -d "${install_dir}" || ! -w "${install_dir}" ]]; then
    echo "Installation directory is unavailable or not writable: ${install_dir}" >&2
    echo "Set ONLYAUDIOS_INSTALL_DIR to a writable Applications directory." >&2
    exit 1
fi

stage_dir="$(mktemp -d "${install_dir}/.onlyaudios-install.XXXXXX")"
staged_app="${stage_dir}/OnlyAudios.app"
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

# Register the new bundle with macOS, then ask Spotlight to index it.
lsregister=/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister
if [[ -x "${lsregister}" ]]; then
    if ! "${lsregister}" -f "${destination}"; then
        echo "Warning: macOS did not register the app automatically." >&2
    fi
    # Keep the installed copy as the only registered OnlyAudios bundle.
    "${lsregister}" -u "${source_app}" >/dev/null 2>&1 || true
fi
mdimport "${destination}" >/dev/null 2>&1 || true
echo "Installed ${destination}"
