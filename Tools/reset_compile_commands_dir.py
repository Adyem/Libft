#!/usr/bin/env python3

"""Reset the compile commands staging directory."""

from __future__ import annotations

import pathlib
import shutil
import sys


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("Usage: reset_compile_commands_dir.py <path>", file=sys.stderr)
        return 1

    target = pathlib.Path(argv[1])
    try:
        shutil.rmtree(target, ignore_errors=True)
        target.mkdir(parents=True, exist_ok=True)
    except OSError as exc:
        print(f"Failed to reset {target}: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

