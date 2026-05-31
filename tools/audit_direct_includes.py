#!/usr/bin/env python3
"""Audit direct includes for C++ translation units.

The script uses Clang's JSON AST dump to find repository headers referenced by
each translation unit and compares those headers with the file's direct
`#include "..."` list. Missing direct includes are reported and can be inserted
with `--apply`.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, Iterator, List, Optional, Sequence, Set, Tuple
from functools import lru_cache


REPO_ROOT = Path(__file__).resolve().parent.parent
HEADER_EXTENSIONS = {".h", ".hh", ".hpp", ".ipp", ".tpp"}
SOURCE_EXTENSIONS = {".cpp", ".cc", ".cxx", ".c"}
DEFAULT_COMPILER = "clang++"


@dataclass
class AuditFinding:
    source_path: Path
    missing_headers: List[Path]
    unused_headers: List[Path]
    cycle_causing_headers: List[Tuple[Path, List[str]]] = field(default_factory=list)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*", "", text)
    return text


def collect_source_files(inputs: Sequence[Path]) -> List[Path]:
    source_files: List[Path] = []
    for input_path in inputs:
        resolved_path = input_path.resolve()
        if resolved_path.is_file() and resolved_path.suffix.lower() in SOURCE_EXTENSIONS:
            source_files.append(resolved_path)
            continue
        if resolved_path.is_dir():
            for child in resolved_path.rglob("*"):
                if child.is_file() and child.suffix.lower() in SOURCE_EXTENSIONS:
                    source_files.append(child.resolve())
    unique_files = sorted({path for path in source_files if REPO_ROOT in path.parents})
    return unique_files


def collect_header_files(inputs: Sequence[Path]) -> List[Path]:
    header_files: List[Path] = []
    for input_path in inputs:
        resolved_path = input_path.resolve()
        if resolved_path.is_file() and resolved_path.suffix.lower() in HEADER_EXTENSIONS:
            header_files.append(resolved_path)
            continue
        if resolved_path.is_dir():
            for child in resolved_path.rglob("*"):
                if child.is_file() and child.suffix.lower() in HEADER_EXTENSIONS:
                    header_files.append(child.resolve())
    unique_files = sorted({path for path in header_files if REPO_ROOT in path.parents})
    return unique_files


def module_name_for_path(path: Path) -> Optional[str]:
    try:
        relative_parts = path.resolve().relative_to(REPO_ROOT).parts
    except ValueError:
        return None
    if len(relative_parts) < 2:
        return None
    if relative_parts[0] != "Modules":
        return None
    return relative_parts[1]


@lru_cache(maxsize=1)
def load_module_dependency_graph() -> Dict[str, Set[str]]:
    graph_path = REPO_ROOT / "Docs" / "module_dependency_graph.md"
    adjacency: Dict[str, Set[str]] = {}
    if not graph_path.exists():
        return adjacency

    row_pattern = re.compile(r"^\|\s*`([^`]+)`\s*\|\s*([^|]+?)\s*\|")
    lines = graph_path.read_text(encoding="utf-8").splitlines()
    for line in lines:
        match = row_pattern.match(line)
        if match is None:
            continue
        module_name = match.group(1).strip()
        dependency_text = match.group(2).strip()
        dependencies: Set[str] = set()
        if dependency_text not in {"", "—", "-"}:
            for dependency_name in re.findall(r"`([^`]+)`", dependency_text):
                dependencies.add(dependency_name.strip())
        adjacency.setdefault(module_name, set()).update(dependencies)
    return adjacency


def module_reaches_target(start_module: str, target_module: str) -> bool:
    return find_module_path(start_module, target_module) is not None


def find_module_path(start_module: str, target_module: str) -> Optional[List[str]]:
    if start_module == target_module:
        return [start_module]
    graph = load_module_dependency_graph()
    if start_module not in graph:
        return None

    visited_modules: Set[str] = set()
    worklist: List[str] = [start_module]
    predecessor_map: Dict[str, Optional[str]] = {start_module: None}
    while worklist:
        current_module = worklist.pop(0)
        if current_module in visited_modules:
            continue
        visited_modules.add(current_module)
        if current_module == target_module:
            path: List[str] = []
            walker = current_module
            while walker is not None:
                path.append(walker)
                walker = predecessor_map.get(walker)
            path.reverse()
            return path
        for dependency_module in graph.get(current_module, set()):
            if dependency_module not in visited_modules:
                if dependency_module not in predecessor_map:
                    predecessor_map[dependency_module] = current_module
                worklist.append(dependency_module)
    return None


def format_module_cycle(source_module: str, path_from_header_to_source: List[str]) -> List[str]:
    cycle_path = [source_module]
    cycle_path.extend(path_from_header_to_source)
    return cycle_path


def parse_direct_repo_includes(source_path: Path) -> Set[Path]:
    included_headers: Set[Path] = set()
    raw_text = source_path.read_text(encoding="utf-8")
    stripped_text = strip_comments(raw_text)
    include_pattern = re.compile(r'^\s*#\s*include\s+"([^"]+)"', flags=re.MULTILINE)

    for match in include_pattern.finditer(stripped_text):
        include_text = match.group(1).strip()
        candidate_paths = [
            (source_path.parent / include_text).resolve(),
            (REPO_ROOT / include_text).resolve(),
        ]
        for candidate_path in candidate_paths:
            if candidate_path.exists() and candidate_path.is_file():
                if candidate_path.suffix.lower() in HEADER_EXTENSIONS:
                    included_headers.add(candidate_path)
                break

    return included_headers


def run_clang_ast_dump(source_path: Path, compiler: str, extra_flags: Sequence[str]) -> dict:
    command = [
        compiler,
        *extra_flags,
        "-fsyntax-only",
        "-Xclang",
        "-ast-dump=json",
        source_path.as_posix(),
    ]
    process = subprocess.run(
        command,
        cwd=REPO_ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if process.returncode != 0:
        raise RuntimeError(
            f"clang failed for {source_path}:\n{process.stderr.strip() or process.stdout.strip()}"
    )
    return json.loads(process.stdout)


def run_clang_ast_dump_for_header(header_path: Path, compiler: str, extra_flags: Sequence[str]) -> dict:
    include_text = header_path.relative_to(REPO_ROOT).as_posix()
    with tempfile.TemporaryDirectory() as temporary_directory:
        temporary_source_path = Path(temporary_directory) / "audit_header.cpp"
        temporary_source_path.write_text(
            f'#include "{include_text}"\n',
            encoding="utf-8",
        )
        return run_clang_ast_dump(temporary_source_path, compiler, extra_flags)


def is_repo_header(path_value: str) -> bool:
    header_path = Path(path_value)
    if not header_path.is_absolute():
        header_path = (REPO_ROOT / header_path).resolve()
    else:
        header_path = header_path.resolve()
    if REPO_ROOT not in header_path.parents and header_path != REPO_ROOT:
        return False
    return header_path.suffix.lower() in HEADER_EXTENSIONS


def normalize_repo_path(path_value: str) -> Path:
    header_path = Path(path_value)
    if not header_path.is_absolute():
        return (REPO_ROOT / header_path).resolve()
    return header_path.resolve()


def build_decl_index(node: object, decl_index: Dict[str, Path]) -> None:
    if isinstance(node, dict):
        node_id = node.get("id")
        location = node.get("loc")
        if isinstance(node_id, str) and isinstance(location, dict):
            file_name = location.get("file")
            if isinstance(file_name, str):
                decl_index[node_id] = normalize_repo_path(file_name)
        for value in node.values():
            build_decl_index(value, decl_index)
    elif isinstance(node, list):
        for value in node:
            build_decl_index(value, decl_index)


def collect_referenced_decl_ids(node: object, referenced_ids: Set[str]) -> None:
    if isinstance(node, dict):
        for key in ("referencedDecl", "decl"):
            nested = node.get(key)
            if isinstance(nested, dict):
                nested_id = nested.get("id")
                if isinstance(nested_id, str):
                    referenced_ids.add(nested_id)
        for value in node.values():
            collect_referenced_decl_ids(value, referenced_ids)
    elif isinstance(node, list):
        for value in node:
            collect_referenced_decl_ids(value, referenced_ids)


def collect_referenced_headers(ast_root: dict) -> Set[Path]:
    decl_index: Dict[str, Path] = {}
    referenced_ids: Set[str] = set()
    build_decl_index(ast_root, decl_index)
    collect_referenced_decl_ids(ast_root, referenced_ids)

    referenced_headers: Set[Path] = set()
    for decl_id in referenced_ids:
        decl_path = decl_index.get(decl_id)
        if decl_path is None:
            continue
        if not is_repo_header(decl_path.as_posix()):
            continue
        referenced_headers.add(decl_path)
    return referenced_headers


def relative_include_text(source_path: Path, header_path: Path) -> str:
    return os.path.relpath(header_path, start=source_path.parent).replace(os.sep, "/")


def find_missing_direct_includes(
    source_path: Path,
    compiler: str,
    extra_flags: Sequence[str],
    allow_cycle_causing_includes: bool,
) -> AuditFinding:
    ast_root = run_clang_ast_dump(source_path, compiler, extra_flags)
    direct_headers = parse_direct_repo_includes(source_path)
    referenced_headers = collect_referenced_headers(ast_root)
    source_module_name = module_name_for_path(source_path)
    missing_headers = []
    cycle_causing_headers: List[Tuple[Path, List[str]]] = []
    for header_path in sorted(referenced_headers):
        if header_path in direct_headers:
            continue
        header_module_name = module_name_for_path(header_path)
        if source_module_name is not None and header_module_name is not None and source_module_name != header_module_name:
            path = find_module_path(header_module_name, source_module_name)
            if path is not None:
                cycle_path = format_module_cycle(source_module_name, path)
                if not allow_cycle_causing_includes:
                    cycle_causing_headers.append((header_path, cycle_path))
                    continue
        missing_headers.append(header_path)
    unused_headers = sorted(
        header_path for header_path in direct_headers if header_path not in referenced_headers
    )
    return AuditFinding(
        source_path=source_path,
        missing_headers=missing_headers,
        unused_headers=unused_headers,
        cycle_causing_headers=cycle_causing_headers,
    )


def find_header_include_audit(
    header_path: Path,
    compiler: str,
    extra_flags: Sequence[str],
    allow_cycle_causing_includes: bool,
) -> AuditFinding:
    ast_root = run_clang_ast_dump_for_header(header_path, compiler, extra_flags)
    direct_headers = parse_direct_repo_includes(header_path)
    referenced_headers = collect_referenced_headers(ast_root)
    source_module_name = module_name_for_path(header_path)
    missing_headers = []
    cycle_causing_headers: List[Tuple[Path, List[str]]] = []
    for referenced_header_path in sorted(referenced_headers):
        if referenced_header_path in direct_headers:
            continue
        header_module_name = module_name_for_path(referenced_header_path)
        if source_module_name is not None and header_module_name is not None and source_module_name != header_module_name:
            path = find_module_path(header_module_name, source_module_name)
            if path is not None:
                cycle_path = format_module_cycle(source_module_name, path)
                if not allow_cycle_causing_includes:
                    cycle_causing_headers.append((referenced_header_path, cycle_path))
                    continue
        missing_headers.append(referenced_header_path)
    unused_headers = sorted(
        referenced_header_path for referenced_header_path in direct_headers if referenced_header_path not in referenced_headers
    )
    return AuditFinding(
        source_path=header_path,
        missing_headers=missing_headers,
        unused_headers=unused_headers,
        cycle_causing_headers=cycle_causing_headers,
    )


def find_include_block_end(lines: List[str]) -> int:
    include_end = 0
    for index, line in enumerate(lines):
        stripped_line = line.strip()
        if stripped_line.startswith("#include "):
            include_end = index + 1
            continue
        if index == 0 and stripped_line == "":
            continue
        if stripped_line == "" or stripped_line.startswith("//") or stripped_line.startswith("/*"):
            if include_end != 0:
                continue
            continue
        if stripped_line.startswith("#") and include_end == 0:
            return index
        if stripped_line.startswith("#") and include_end != 0:
            return index
        if include_end != 0:
            return include_end
    return include_end


def apply_missing_includes(source_path: Path, missing_headers: Sequence[Path]) -> bool:
    if not missing_headers:
        return False

    lines = source_path.read_text(encoding="utf-8").splitlines(keepends=True)
    include_end = find_include_block_end(lines)
    inserted_lines = []
    existing_include_texts = {
        line.strip()[len("#include "):].strip('"')
        for line in lines
        if line.strip().startswith('#include "')
    }
    ordered_missing_texts = sorted(
        relative_include_text(source_path, header_path)
        for header_path in missing_headers
    )
    for include_text in ordered_missing_texts:
        if include_text in existing_include_texts:
            continue
        inserted_lines.append(f'#include "{include_text}"\n')

    if not inserted_lines:
        return False

    # Keep the new includes with the existing include block.
    lines[include_end:include_end] = inserted_lines
    source_path.write_text("".join(lines), encoding="utf-8")
    return True


def parse_arguments(argv: Optional[Sequence[str]]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Find repository headers that are used by .cpp files but not included directly."
    )
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        default=[REPO_ROOT / "Modules"],
        help="Files or directories to scan. Defaults to Modules/.",
    )
    parser.add_argument(
        "--compiler",
        default=DEFAULT_COMPILER,
        help=f"Compiler to use for AST dumping (default: {DEFAULT_COMPILER}).",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Insert missing direct includes into the source files.",
    )
    parser.add_argument(
        "--report-cycle-causing-includes",
        action="store_true",
        help="Report missing direct includes even when they would add a module dependency cycle.",
    )
    parser.add_argument(
        "--scan-headers",
        action="store_true",
        help="Also scan header files by compiling a temporary translation unit that includes them.",
    )
    parser.add_argument(
        "--extra-flag",
        action="append",
        default=[],
        help="Extra compiler flag to pass to Clang. May be repeated.",
    )
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    arguments = parse_arguments(argv)
    if arguments.report_cycle_causing_includes:
        load_module_dependency_graph.cache_clear()
    source_files = collect_source_files(arguments.paths)
    header_files = collect_header_files(arguments.paths) if arguments.scan_headers else []
    if not source_files:
        if not header_files:
            print("No source or header files found.")
            return 0

    if source_files:
        print(f"Scanning {len(source_files)} source file(s).")
    if header_files:
        print(f"Scanning {len(header_files)} header file(s).")

    if not source_files and not header_files:
        print("No source or header files found.")
        return 0

    compiler_flags = [
        "-std=c++17",
        "-DLIBFT_INTERNAL_HEADERS",
        "-pthread",
        "-w",
        "-I",
        ".",
        "-include",
        "Modules/Basic/basic.hpp",
        "-include",
        "Modules/Errno/errno.hpp",
        *arguments.extra_flag,
    ]

    exit_code = 0
    findings: List[AuditFinding] = []
    for source_path in source_files:
        try:
            finding = find_missing_direct_includes(
                source_path,
                arguments.compiler,
                compiler_flags,
                arguments.report_cycle_causing_includes,
            )
        except Exception as error:
            print(f"error: {source_path}: {error}", file=sys.stderr)
            exit_code = 2
            continue
        if finding.missing_headers or finding.unused_headers or finding.cycle_causing_headers:
            findings.append(finding)
            print(source_path.relative_to(REPO_ROOT).as_posix())
            for header_path in finding.missing_headers:
                include_text = relative_include_text(source_path, header_path)
                print(f"  missing: {include_text}")
            if finding.unused_headers:
                for header_path in finding.unused_headers:
                    include_text = relative_include_text(source_path, header_path)
                    print(f"  unused: {include_text}")
            for blocked_header_path, cycle_path in finding.cycle_causing_headers:
                include_text = relative_include_text(source_path, blocked_header_path)
                cycle_text = " -> ".join(cycle_path)
                print(f"  cycle: {include_text}  ({cycle_text})")
            if arguments.apply:
                if apply_missing_includes(source_path, finding.missing_headers):
                    print("  applied")

    for header_path in header_files:
        try:
            finding = find_header_include_audit(
                header_path,
                arguments.compiler,
                compiler_flags,
                arguments.report_cycle_causing_includes,
            )
        except Exception as error:
            print(f"error: {header_path}: {error}", file=sys.stderr)
            exit_code = 2
            continue
        if finding.missing_headers or finding.unused_headers or finding.cycle_causing_headers:
            findings.append(finding)
            print(header_path.relative_to(REPO_ROOT).as_posix())
            for referenced_header_path in finding.missing_headers:
                print(f"  missing: {referenced_header_path.relative_to(REPO_ROOT).as_posix()}")
            for referenced_header_path in finding.unused_headers:
                print(f"  unused: {referenced_header_path.relative_to(REPO_ROOT).as_posix()}")
            for blocked_header_path, cycle_path in finding.cycle_causing_headers:
                cycle_text = " -> ".join(cycle_path)
                print(
                    f"  cycle: {blocked_header_path.relative_to(REPO_ROOT).as_posix()}  ({cycle_text})"
                )

    if not findings and exit_code == 0:
        print("No missing direct includes detected.")

    return exit_code


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
