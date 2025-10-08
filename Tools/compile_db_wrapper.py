#!/usr/bin/env python3

"""Compiler wrapper that records invocations for compile_commands.json."""

from __future__ import annotations

import json
import os
import pathlib
import shlex
import subprocess
import sys
import uuid


def _detect_source(arguments: list[str]) -> str | None:
    """Return the translation unit path from compiler arguments."""

    next_is_source = False
    for argument in arguments:
        if next_is_source:
            return argument
        if argument == "-c":
            next_is_source = True
            continue
        if argument.endswith(".c") or argument.endswith(".cc") or argument.endswith(".cpp"):
            return argument
    return None


def _format_command(command: list[str]) -> str:
    """Join a compiler command for json emission."""

    return " ".join(shlex.quote(part) for part in command)


def _write_entry(command: list[str], source: str) -> None:
    staging_dir = os.environ.get("FT_COMPILE_DB_DIR")
    if not staging_dir:
        return

    try:
        path = pathlib.Path(staging_dir)
        path.mkdir(parents=True, exist_ok=True)
    except OSError:
        return

    entry = {
        "directory": os.getcwd(),
        "command": _format_command(command),
        "file": str(pathlib.Path(source).resolve()),
    }

    target = path / f"entry-{uuid.uuid4().hex}.json"
    try:
        target.write_text(json.dumps(entry))
    except OSError:
        pass


def main() -> int:
    if len(sys.argv) < 2:
        print("compile_db_wrapper.py requires the compiler command as the first argument.", file=sys.stderr)
        return 1

    compiler = sys.argv[1]
    arguments = sys.argv[2:]
    command = [compiler, *arguments]

    process = subprocess.run(command)

    if process.returncode == 0 and os.environ.get("FT_GENERATE_COMPILE_DB") == "1":
        source = _detect_source(arguments)
        if source is not None:
            _write_entry(command, source)

    return process.returncode


if __name__ == "__main__":
    sys.exit(main())

