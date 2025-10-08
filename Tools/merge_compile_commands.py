#!/usr/bin/env python3

"""Merge compiler invocation logs into compile_commands.json."""

from __future__ import annotations

import json
import pathlib
import sys


def _load_entries(staging: pathlib.Path) -> list[dict[str, str]]:
    entries: dict[tuple[str, str], dict[str, str]] = {}
    if not staging.exists():
        return []

    for path in sorted(staging.glob("entry-*.json")):
        try:
            entry = json.loads(path.read_text())
        except (OSError, json.JSONDecodeError):
            continue

        key = (entry.get("directory", ""), entry.get("file", ""))
        if key[0] and key[1]:
            entries[key] = {
                "directory": entry["directory"],
                "command": entry.get("command", ""),
                "file": entry["file"],
            }

    return [entries[key] for key in sorted(entries)]


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print("Usage: merge_compile_commands.py <staging-dir> <output>", file=sys.stderr)
        return 1

    staging = pathlib.Path(argv[1])
    output = pathlib.Path(argv[2])

    entries = _load_entries(staging)

    try:
        output.write_text(json.dumps(entries, indent=2) + "\n")
    except OSError as exc:
        print(f"Failed to write {output}: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

