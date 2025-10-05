#!/bin/sh
set -eu
script_dir="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"

if rg --files-with-matches --hidden \
    --glob '!.git/**' \
    --glob '!Tools/**' \
    --glob '!Test/**' \
    --glob '!README.md' \
    --glob '!**/README.md' \
    --glob '!**/*.md' \
    'std::string' "${repo_root}"; then
    echo "std::string usage found above"
    exit 1
fi

echo "No std::string usages found outside excluded files"
