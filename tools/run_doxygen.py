#!/usr/bin/env python3
"""Helper script to validate and run the Libft Doxygen configuration."""
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
DOXYFILE = REPO_ROOT / "Docs" / "Doxyfile"
EXPECTED_INPUTS = (
    "API",
    "CMA",
    "Compression",
    "Config",
    "Errno",
    "Libft",
    "Logger",
    "Networking",
    "Printf",
    "Storage",
    "System_utils",
    "Template",
    "Time",
)


def ensure_requirements() -> None:
    if not DOXYFILE.exists():
        raise SystemExit(f"missing Doxyfile at {DOXYFILE}")
    docs_folder = DOXYFILE.parent
    for relative in EXPECTED_INPUTS:
        candidate = REPO_ROOT / relative
        if not candidate.exists():
            raise SystemExit(f"expected input path '{relative}' was not found at {candidate}")
    api_folder = docs_folder / "api"
    api_folder.mkdir(parents=True, exist_ok=True)


def run_doxygen() -> int:
    doxygen_path = shutil.which("doxygen")
    if doxygen_path is None:
        raise SystemExit("doxygen executable was not found in PATH")
    process = subprocess.run((doxygen_path, str(DOXYFILE)), cwd=DOXYFILE.parent)
    return process.returncode


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="Validate configuration paths without invoking Doxygen.",
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    arguments = parse_args(argv)
    ensure_requirements()
    if arguments.check:
        print("Doxygen configuration looks valid.")
        return 0
    return run_doxygen()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
