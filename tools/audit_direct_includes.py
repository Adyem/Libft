#!/usr/bin/env python3
"""Audit direct includes for C++ translation units.

The script uses Clang's JSON AST dump to find repository headers referenced by
each translation unit and compares those headers with the file's direct
`#include "..."` list. Missing direct includes are reported and can be inserted
with `--apply`, and unused direct includes can be removed with the same flag.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shlex
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
DEFAULT_FLAGS_MAKEFILE = REPO_ROOT / "mk" / "compiler_flags.mk"
CLANG_INCOMPATIBLE_FLAGS = {
    "-Wmaybe-uninitialized",
    "-Wuseless-cast",
}


@dataclass
class AuditFinding:
    source_path: Path
    missing_headers: List[Path]
    unused_headers: List[Path]
    defined_function_names: Set[str] = field(default_factory=set)
    defined_class_names: Set[str] = field(default_factory=set)
    cycle_causing_headers: List[Tuple[Path, List[str]]] = field(default_factory=list)


@dataclass
class ModuleDependencyGraph:
    adjacency: Dict[str, Set[str]]
    source_files_scanned: int
    header_files_scanned: int


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


def collect_module_dependency_graph(inputs: Sequence[Path]) -> ModuleDependencyGraph:
    source_files = collect_source_files(inputs)
    header_files = collect_header_files(inputs)
    graph: Dict[str, Set[str]] = {}

    for file_path in source_files:
        module_name = module_name_for_path(file_path)
        if module_name is None:
            continue
        graph.setdefault(module_name, set())
        for header_path in parse_direct_repo_includes(file_path):
            dependency_module_name = module_name_for_path(header_path)
            if dependency_module_name is None:
                continue
            if dependency_module_name == module_name:
                continue
            graph[module_name].add(dependency_module_name)

    for header_path in header_files:
        module_name = module_name_for_path(header_path)
        if module_name is None:
            continue
        graph.setdefault(module_name, set())
        for included_header_path in parse_direct_repo_includes(header_path):
            dependency_module_name = module_name_for_path(included_header_path)
            if dependency_module_name is None:
                continue
            if dependency_module_name == module_name:
                continue
            graph[module_name].add(dependency_module_name)

    return ModuleDependencyGraph(
        adjacency=graph,
        source_files_scanned=len(source_files),
        header_files_scanned=len(header_files),
    )


def render_module_dependency_graph(graph: ModuleDependencyGraph) -> str:
    module_names = sorted(graph.adjacency)
    lines: List[str] = []
    lines.append("# Module Dependency Graph")
    lines.append("")
    lines.append("## Purpose")
    lines.append(
        "This document is generated from direct `#include` relationships in `Modules/`."
    )
    lines.append(
        "It tracks the direct module-to-module dependencies that are visible from the source tree."
    )
    lines.append(
        "The arrow direction always flows from the dependent module to the module it uses."
    )
    lines.append(
        "For a broader architecture view grouped by layer, see [Docs/module_layering.md](module_layering.md)."
    )
    lines.append("")
    lines.append("## Legend")
    lines.append("- `A --> B` means module `A` depends on module `B`.")
    lines.append("")
    lines.append("## Direct dependencies")
    lines.append("")
    lines.append("| Module | Direct dependencies |")
    lines.append("| --- | --- |")
    for module_name in module_names:
        dependency_names = sorted(graph.adjacency.get(module_name, set()))
        if dependency_names:
            dependency_text = ", ".join(f"`{dependency_name}`" for dependency_name in dependency_names)
        else:
            dependency_text = "—"
        lines.append(f"| `{module_name}` | {dependency_text} |")
    lines.append("")
    lines.append("## Visual summary")
    lines.append("This is a simplified renderable view of the dependency flow.")
    lines.append("")
    lines.append("```mermaid")
    lines.append("graph TD")
    for module_name in module_names:
        dependency_names = sorted(graph.adjacency.get(module_name, set()))
        for dependency_name in dependency_names:
            lines.append(f"    {module_name} --> {dependency_name}")
    lines.append("```")
    lines.append("")
    lines.append("## Maintenance rule")
    lines.append(
        "If a new module is added or existing includes change module-to-module dependencies, regenerate this document from the source tree in the same change set."
    )
    lines.append("")
    lines.append(
        f"Generated from {graph.source_files_scanned} source file(s) and {graph.header_files_scanned} header file(s)."
    )
    return "\n".join(lines)


def write_module_dependency_graph(output_path: Path, inputs: Sequence[Path]) -> None:
    graph = collect_module_dependency_graph(inputs)
    output_path.write_text(render_module_dependency_graph(graph), encoding="utf-8")


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


def collect_preprocessor_macro_references(text: str) -> Set[str]:
    macro_references: Set[str] = set()
    stripped_text = strip_comments(text)
    directive_pattern = re.compile(r"^\s*#\s*(if|elif|ifdef|ifndef)\b(.*)$", flags=re.MULTILINE)
    macro_pattern = re.compile(r"\b[A-Z_][A-Z0-9_]*\b")

    for match in directive_pattern.finditer(stripped_text):
        directive_body = match.group(2)
        for macro_name in macro_pattern.findall(directive_body):
            macro_references.add(macro_name)
    return macro_references


@lru_cache(maxsize=None)
def collect_header_defined_macros(header_path: Path) -> Set[str]:
    macro_names: Set[str] = set()
    try:
        raw_text = header_path.read_text(encoding="utf-8")
    except OSError:
        return macro_names
    stripped_text = strip_comments(raw_text)
    define_pattern = re.compile(
        r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\b",
        flags=re.MULTILINE,
    )
    for match in define_pattern.finditer(stripped_text):
        macro_names.add(match.group(1))
    return macro_names


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


def run_syntax_check(source_path: Path, compiler: str, extra_flags: Sequence[str]) -> bool:
    command = [
        compiler,
        *extra_flags,
        "-fsyntax-only",
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
    return process.returncode == 0


def load_compile_flags(makefile_path: Path, variable_name: str) -> List[str]:
    print_target_name = f"print_{variable_name.lower()}"
    make_expression = (
        f'{print_target_name}: ; @printf "%s\\n" "$({variable_name})"'
    )
    command = [
        "make",
        "-f",
        makefile_path.as_posix(),
        "--no-print-directory",
        "--silent",
        "--eval",
        make_expression,
        print_target_name,
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
            f"failed to read {variable_name} from {makefile_path}:\n{process.stderr.strip() or process.stdout.strip()}"
        )
    return shlex.split(process.stdout.strip())


def sanitize_compile_flags(compiler: str, compile_flags: Sequence[str]) -> List[str]:
    compiler_name = Path(compiler).name.lower()
    if "clang" not in compiler_name:
        return list(compile_flags)

    sanitized_flags = [
        flag for flag in compile_flags if flag not in CLANG_INCOMPATIBLE_FLAGS
    ]
    sanitized_flags.extend(
        [
            "-Wno-error",
            "-Wno-error=unknown-warning-option",
            "-Wno-unknown-warning-option",
        ]
    )
    return sanitized_flags


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


def collect_defined_function_names(ast_root: object, source_path: Path) -> Set[str]:
    defined_function_names: Set[str] = set()

    def walk(node: object) -> None:
        if isinstance(node, dict):
            node_kind = node.get("kind")
            location = node.get("loc")
            if isinstance(node_kind, str) and isinstance(location, dict):
                file_name = location.get("file")
                node_name = node.get("name")
                if (
                    isinstance(file_name, str)
                    and normalize_repo_path(file_name) == source_path
                    and isinstance(node_name, str)
                    and node_kind in {
                        "FunctionDecl",
                        "CXXMethodDecl",
                        "CXXConstructorDecl",
                        "CXXDestructorDecl",
                    }
                ):
                    defined_function_names.add(node_name)
            for value in node.values():
                walk(value)
        elif isinstance(node, list):
            for value in node:
                walk(value)

    walk(ast_root)
    return defined_function_names


def collect_qualified_class_names(source_text: str) -> Set[str]:
    class_names: Set[str] = set()
    qualified_pattern = re.compile(
        r"\b([A-Za-z_][A-Za-z0-9_]*)::\s*(?:~?[A-Za-z_][A-Za-z0-9_]*|operator\b)",
        flags=re.MULTILINE,
    )
    for match in qualified_pattern.finditer(strip_comments(source_text)):
        class_names.add(match.group(1))
    return class_names


def header_defines_source_symbol(
    header_path: Path,
    source_function_names: Set[str],
    source_class_names: Set[str],
) -> bool:
    try:
        header_text = header_path.read_text(encoding="utf-8")
    except OSError:
        return False

    stripped_text = strip_comments(header_text)
    for function_name in source_function_names:
        function_pattern = re.compile(rf"\b{re.escape(function_name)}\s*\(")
        if function_pattern.search(stripped_text):
            return True
    for class_name in source_class_names:
        class_pattern = re.compile(
            rf"\b(?:class|struct)\s+{re.escape(class_name)}\b"
        )
        if class_pattern.search(stripped_text):
            return True
    return False


def header_is_protected_from_removal(
    header_path: Path,
    source_text: str,
    source_function_names: Set[str],
    source_class_names: Set[str],
) -> bool:
    defined_macros = collect_header_defined_macros(header_path)
    if defined_macros & collect_preprocessor_macro_references(source_text):
        return True
    if header_defines_source_symbol(
        header_path,
        source_function_names,
        source_class_names,
    ):
        return True
    return False


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
    source_text = source_path.read_text(encoding="utf-8")
    defined_function_names = collect_defined_function_names(ast_root, source_path)
    defined_class_names = collect_qualified_class_names(source_text)
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
        header_path
        for header_path in direct_headers
        if header_path not in referenced_headers
        and not header_is_protected_from_removal(
            header_path,
            source_text,
            defined_function_names,
            defined_class_names,
        )
    )
    return AuditFinding(
        source_path=source_path,
        missing_headers=missing_headers,
        unused_headers=unused_headers,
        defined_function_names=sorted(defined_function_names),
        defined_class_names=sorted(defined_class_names),
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
    source_text = header_path.read_text(encoding="utf-8")
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
        referenced_header_path
        for referenced_header_path in direct_headers
        if referenced_header_path not in referenced_headers
        and not header_is_protected_from_removal(
            referenced_header_path,
            source_text,
            set(),
            set(),
        )
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


def apply_include_changes(
    source_path: Path,
    missing_headers: Sequence[Path],
    unused_headers: Sequence[Path],
    defined_function_names: Sequence[str],
    defined_class_names: Sequence[str],
    validation_compiler: str,
    validation_flags: Sequence[str],
) -> bool:
    if not missing_headers and not unused_headers:
        return False

    raw_text = source_path.read_text(encoding="utf-8")
    lines = raw_text.splitlines(keepends=True)
    include_pattern = re.compile(r'^(\s*#\s*include\s+")([^"]+)("\s*)$', flags=re.MULTILINE)
    missing_include_texts = {
        relative_include_text(source_path, header_path)
        for header_path in missing_headers
    }
    macro_references = collect_preprocessor_macro_references(raw_text)
    unused_header_paths = {header_path.resolve() for header_path in unused_headers}
    source_function_names = set(defined_function_names)
    source_class_names = set(defined_class_names)

    line_indexes_to_remove: Set[int] = set()
    working_lines = list(lines)

    def build_text_without_line_index(remove_index: int) -> str:
        return "".join(
            line for index, line in enumerate(working_lines) if index != remove_index
        )

    for index, line in enumerate(lines):
        match = include_pattern.match(line)
        if match is None:
            continue
        include_text = match.group(2).strip()
        candidate_paths = [
            (source_path.parent / include_text).resolve(),
            (REPO_ROOT / include_text).resolve(),
        ]
        for candidate_path in candidate_paths:
            if candidate_path in unused_header_paths:
                defined_macros = collect_header_defined_macros(candidate_path)
                if defined_macros & macro_references:
                    continue
                if header_defines_source_symbol(
                    candidate_path,
                    source_function_names,
                    source_class_names,
                ):
                    continue
                temporary_text = build_text_without_line_index(index)
                temporary_source_path = source_path.parent / f".audit_{source_path.name}"
                try:
                    temporary_source_path.write_text(temporary_text, encoding="utf-8")
                    if run_syntax_check(
                        temporary_source_path,
                        validation_compiler,
                        validation_flags,
                    ):
                        line_indexes_to_remove.add(index)
                        working_lines[index] = ""
                finally:
                    if temporary_source_path.exists():
                        temporary_source_path.unlink()
                break

    retained_lines = [
        line for index, line in enumerate(lines) if index not in line_indexes_to_remove
    ]

    inserted_lines: List[str] = []
    ordered_missing_texts = sorted(missing_include_texts)
    retained_include_texts = {
        line.strip()[len("#include "):].strip('"')
        for line in retained_lines
        if line.strip().startswith('#include "')
    }
    for include_text in ordered_missing_texts:
        if include_text in retained_include_texts:
            continue
        inserted_lines.append(f'#include "{include_text}"\n')

    if not inserted_lines and not line_indexes_to_remove:
        return False

    retained_include_end = find_include_block_end(retained_lines)
    retained_lines[retained_include_end:retained_include_end] = inserted_lines
    source_path.write_text("".join(retained_lines), encoding="utf-8")
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
        "--validation-compiler",
        default="g++",
        help="Compiler to use for validation syntax checks (default: g++).",
    )
    parser.add_argument(
        "--flags-makefile",
        type=Path,
        default=DEFAULT_FLAGS_MAKEFILE,
        help=(
            "Makefile used to fetch COMPILE_FLAGS before auditing. "
            f"Default: {DEFAULT_FLAGS_MAKEFILE.as_posix()}."
        ),
    )
    parser.add_argument(
        "--flags-variable",
        default="COMPILE_FLAGS",
        help="Makefile variable to read for compiler flags (default: COMPILE_FLAGS).",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Insert missing direct includes and remove unused direct includes.",
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
    parser.add_argument(
        "--write-module-dependency-graph",
        action="store_true",
        help="Regenerate Docs/module_dependency_graph.md from direct repo includes.",
    )
    parser.add_argument(
        "--module-dependency-graph-output",
        type=Path,
        default=REPO_ROOT / "Docs" / "module_dependency_graph.md",
        help="Output path for --write-module-dependency-graph.",
    )
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    arguments = parse_arguments(argv)
    if arguments.write_module_dependency_graph:
        write_module_dependency_graph(arguments.module_dependency_graph_output, arguments.paths)
        print(
            f"Wrote module dependency graph to {arguments.module_dependency_graph_output.relative_to(REPO_ROOT).as_posix()}."
        )
        return 0
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

    analysis_flags = sanitize_compile_flags(
        arguments.compiler,
        load_compile_flags(arguments.flags_makefile, arguments.flags_variable),
    )
    validation_flags = list(analysis_flags)
    if any(path.parts and path.parts[0] == "Test" for path in arguments.paths):
        if "-DLIBFT_TEST_BUILD" not in analysis_flags:
            analysis_flags.append("-DLIBFT_TEST_BUILD")
        if "-DLIBFT_TEST_BUILD" not in validation_flags:
            validation_flags.append("-DLIBFT_TEST_BUILD")
    analysis_flags.extend(arguments.extra_flag)
    validation_flags.extend(arguments.extra_flag)

    exit_code = 0
    findings: List[AuditFinding] = []
    for source_path in source_files:
        try:
            finding = find_missing_direct_includes(
                source_path,
                arguments.compiler,
                analysis_flags,
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
                if apply_include_changes(
                    source_path,
                    finding.missing_headers,
                    finding.unused_headers,
                    finding.defined_function_names,
                    finding.defined_class_names,
                    arguments.validation_compiler,
                    validation_flags,
                ):
                    print("  applied")

    for header_path in header_files:
        try:
            finding = find_header_include_audit(
                header_path,
                arguments.compiler,
                analysis_flags,
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
            if arguments.apply:
                if apply_include_changes(
                    header_path,
                    finding.missing_headers,
                    finding.unused_headers,
                    finding.defined_function_names,
                    finding.defined_class_names,
                    arguments.validation_compiler,
                    validation_flags,
                ):
                    print("  applied")

    if not findings and exit_code == 0:
        print("No missing direct includes detected.")

    return exit_code


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
