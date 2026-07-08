#!/bin/sh
set -eu

dirname="$(dirname "$0")"
repo_root="$(cd "${dirname}/.." && pwd)"

echo "Installing pre-commit hooks in ${repo_root}" >&2
if ! command -v pre-commit >/dev/null 2>&1; then
    echo "Error: pre-commit is not installed. Install it with \"pip install pre-commit\"." >&2
    exit 1
fi
cd "${repo_root}"
pre-commit install --install-hooks
