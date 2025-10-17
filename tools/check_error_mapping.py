#!/usr/bin/env python3
"""Ensure OS error codes are mapped to Libft error space before use."""
from __future__ import annotations

import pathlib
import re
import sys
from typing import Iterable, Tuple

ROOT = pathlib.Path(__file__).resolve().parents[1]

PatternRule = Tuple[re.Pattern[str], Iterable[str], Iterable[str]]

def path_in_prefixes(path: pathlib.Path, prefixes: Iterable[str]) -> bool:
    rel = path.relative_to(ROOT)
    rel_str = rel.as_posix()
    for prefix in prefixes:
        if rel_str == prefix:
            return True
        if rel_str.startswith(prefix.rstrip('/') + '/'):
            return True
    return False

def scan_file(path: pathlib.Path, rules: Iterable[PatternRule]) -> list[tuple[int, str, str]]:
    try:
        text = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return []
    failures: list[tuple[int, str, str]] = []
    lines = text.splitlines()
    for index, line in enumerate(lines, start=1):
        for pattern, allowed_dirs, allowed_files in rules:
            if pattern.search(line):
                rel_path = path.relative_to(ROOT)
                if path_in_prefixes(path, allowed_dirs) or path_in_prefixes(path, allowed_files):
                    continue
                failures.append((index, rel_path.as_posix(), line.strip()))
    return failures

def main() -> int:
    rules: list[PatternRule] = [
        (re.compile(r"ERRNO_OFFSET"),
         ("Compatebility", "Errno", "Test", "Docs"),
         ()),
        (re.compile(r"ft_errno\s*=\s*(?:::)?(?:std::)?errno"),
         ("Test",),
         ()),
        (re.compile(r"return\s*\(?\s*(?:::)?(?:std::)?errno"),
         (),
         ())
    ]
    failures: list[tuple[int, str, str]] = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix not in {".c", ".cpp", ".h", ".hpp", ".cc", ".hh", ".ipp"}:
            continue
        failures.extend(scan_file(path, rules))
    if failures:
        for line_no, rel_path, snippet in failures:
            print(f"{rel_path}:{line_no}: {snippet}")
        print("Found OS error mapping violations.", file=sys.stderr)
        return 1
    return 0

if __name__ == "__main__":
    sys.exit(main())
