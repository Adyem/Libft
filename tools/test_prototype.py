#!/usr/bin/env python3
"""Utility for compiling and running a narrow slice of the Libft tests.

The script accepts either test source file names or FT_TEST identifiers and
only builds the objects that are required for the selected tests.  The regular
Full_Libft archive is still used for linking, so the core library build is
reused while the test-side build cost is restricted to the provided files.
"""
from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from functools import lru_cache
from pathlib import Path
from typing import Dict, Iterable, List, Sequence, Set, Tuple

REPO_ROOT = Path(__file__).resolve().parents[1]
TEST_DIR = REPO_ROOT / "Test"
TEST_SUBDIRS = [TEST_DIR / "Test", TEST_DIR / "API"]

TEST_REGEX = re.compile(r"FT_TEST\s*\(\s*([A-Za-z0-9_]+)")


def _read_text(path: Path) -> str:
    try:
        return path.read_text()
    except UnicodeDecodeError:
        return path.read_text(encoding="utf-8", errors="ignore")


def iter_test_sources() -> Iterable[Path]:
    for directory in TEST_SUBDIRS:
        if not directory.exists():
            continue
        for source in directory.rglob("*.cpp"):
            yield source


@lru_cache(maxsize=1)
def build_test_index() -> Dict[str, List[Path]]:
    index: Dict[str, List[Path]] = {}
    for source in iter_test_sources():
        matches = TEST_REGEX.findall(_read_text(source))
        if not matches:
            continue
        for test_name in matches:
            index.setdefault(test_name, []).append(source)
    return index


def discover_source_by_name(name: str) -> Path:
    """Find a test source file by its basename."""
    matches: List[Path] = []
    candidate_names = [name]
    if not name.endswith(".cpp"):
        candidate_names.append(f"{name}.cpp")
    for directory in TEST_SUBDIRS:
        if not directory.exists():
            continue
        for candidate_name in candidate_names:
            matches.extend(directory.rglob(candidate_name))
    if not matches:
        raise FileNotFoundError(f"Unable to find test source named '{name}'.")
    if len(matches) > 1:
        raise RuntimeError(
            f"The name '{name}' is ambiguous. Use a relative path instead."
        )
    return matches[0]


def normalize_source_path(value: str) -> Path:
    candidate = Path(value)
    if not candidate.is_absolute():
        relative_to_repo = (REPO_ROOT / value).resolve()
        if relative_to_repo.exists():
            candidate = relative_to_repo
        else:
            return discover_source_by_name(value)
    if not candidate.exists():
        raise FileNotFoundError(f"Source '{value}' does not exist.")
    try:
        candidate.relative_to(TEST_DIR)
    except ValueError as error:
        raise ValueError(
            f"Source '{value}' is outside the Test directory."
        ) from error
    return candidate


def find_test_file_for_name(test_name: str) -> Path:
    index = build_test_index()
    matches = index.get(test_name, [])
    if not matches:
        raise FileNotFoundError(
            f"Unable to locate a test named '{test_name}'."
        )
    if len(matches) > 1:
        raise RuntimeError(
            f"Multiple test files contain '{test_name}'. Please disambiguate using --file."
        )
    return matches[0]


def make_relative_to_test_dir(paths: Iterable[Path]) -> List[str]:
    relatives: List[str] = []
    for path in paths:
        relatives.append(str(path.relative_to(TEST_DIR)))
    return relatives


def format_relative(path: Path) -> str:
    return str(path.relative_to(TEST_DIR))


def run_make(selected_files: Sequence[str], debug: bool, opt_level: str | None) -> None:
    env = os.environ.copy()
    env["FILES"] = " ".join(selected_files)
    if opt_level is not None:
        env["OPT_LEVEL"] = opt_level
    target = "debug" if debug else "all"
    cmd = ["make", "-C", str(TEST_DIR), target]
    print("Building tests:", " ".join(selected_files))
    subprocess.run(cmd, check=True, env=env)


def summarize_selected_tests(resolved: Sequence[Tuple[str, Path]]) -> None:
    if not resolved:
        return
    print("Selected FT_TEST targets:")
    for name, path in resolved:
        print(f"  {name} -> {format_relative(path)}")


def run_binary(debug: bool, filtered_names: Sequence[str]) -> int:
    binary = TEST_DIR / ("libft_tests_debug" if debug else "libft_tests")
    env = os.environ.copy()
    if filtered_names:
        env["FT_TEST_NAME_FILTER"] = ",".join(filtered_names)
    print(f"Running {binary.name}...")
    completed = subprocess.run([str(binary)], env=env)
    return completed.returncode


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Compile and run a reduced set of tests. Provide --file to compile an "
            "entire source file or --test to run a specific FT_TEST by name."
        )
    )
    parser.add_argument(
        "--file",
        action="append",
        dest="files",
        default=[],
        help="Relative path or basename of a test source file.",
    )
    parser.add_argument(
        "--test",
        action="append",
        dest="tests",
        default=[],
        help="Exact FT_TEST identifier to build and execute.",
    )
    parser.add_argument(
        "targets",
        nargs="*",
        help=(
            "Positional shorthand. Entries ending in .cpp or containing a path are "
            "treated as sources while the rest are FT_TEST names."
        ),
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="Build the debug variant (matches 'make debug').",
    )
    parser.add_argument(
        "--opt-level",
        choices=["0", "1", "2", "3"],
        help="Override OPT_LEVEL for the underlying make invocation.",
    )
    args = parser.parse_args(argv)
    positional_files, positional_tests = partition_positional_targets(args.targets)
    args.files.extend(positional_files)
    args.tests.extend(positional_tests)
    if not args.files and not args.tests:
        parser.error("Provide at least one --file, --test, or positional target.")
    return args


def looks_like_source_target(value: str) -> bool:
    if not value:
        return False
    if value.endswith(".cpp"):
        return True
    if "/" in value or value.startswith("."):
        return True
    candidate = Path(value)
    if candidate.is_file():
        return True
    repo_candidate = (REPO_ROOT / value)
    if repo_candidate.exists():
        return True
    return False


def partition_positional_targets(values: Sequence[str]) -> tuple[List[str], List[str]]:
    file_like: List[str] = []
    test_like: List[str] = []
    for entry in values:
        if looks_like_source_target(entry):
            file_like.append(entry)
        else:
            test_like.append(entry)
    return file_like, test_like


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    selected: Set[Path] = set()
    resolved_tests: List[Tuple[str, Path]] = []
    for entry in args.files:
        selected.add(normalize_source_path(entry))
    for test_name in args.tests:
        owner = find_test_file_for_name(test_name)
        selected.add(owner)
        resolved_tests.append((test_name, owner))
    relative_files = make_relative_to_test_dir(sorted(selected))
    summarize_selected_tests(resolved_tests)
    run_make(relative_files, args.debug, args.opt_level)
    return run_binary(args.debug, args.tests)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
