#!/usr/bin/env python3

import os
import re
import shlex
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TEST_DIR = ROOT / "Test"
LIBFT_DIR = ROOT
SINGLE_TEST_DIR = TEST_DIR / "single_tests"


def run_command(command, *, cwd=None, check=True, capture_output=False):
    return subprocess.run(
        command,
        cwd=cwd,
        check=check,
        text=True,
        capture_output=capture_output,
    )


def read_make_value(target):
    result = run_command(
        ["make", "-s", target],
        cwd=TEST_DIR,
        capture_output=True,
    )
    return result.stdout.strip()


def resolve_test_path(argument):
    candidate = Path(argument)
    if candidate.is_absolute():
        resolved = candidate
    else:
        resolved = (TEST_DIR / candidate).resolve()
        if not resolved.exists():
            matches = list((TEST_DIR / "Test").glob(argument))
            if len(matches) == 1:
                resolved = matches[0].resolve()
            else:
                matches = list((TEST_DIR / "Test").glob(f"*{argument}*"))
                if len(matches) == 1:
                    resolved = matches[0].resolve()
    if not resolved.exists():
        raise SystemExit(f"Test source not found: {argument}")
    if resolved.suffix != ".cpp":
        raise SystemExit(f"Expected a .cpp test source, got: {resolved}")
    return resolved


def module_sources():
    sources = []
    for source in LIBFT_DIR.rglob("*.cpp"):
        if "/objs" in source.as_posix():
            continue
        if source.is_relative_to(TEST_DIR):
            continue
        if source.parent == LIBFT_DIR:
            continue
        sources.append(source)
    return sources


def existing_module_objects(objdir):
    objects = []
    for object_path in LIBFT_DIR.glob(f"*/{objdir}/*.o"):
        if object_path.is_relative_to(TEST_DIR):
            continue
        objects.append(object_path.resolve())
    return objects


def build_test_object(test_source, objdir):
    main_object = TEST_DIR / objdir / "main.o"
    test_object = TEST_DIR / objdir / test_source.relative_to(TEST_DIR).with_suffix(".o")
    run_command(
        ["make", str(main_object.relative_to(TEST_DIR))],
        cwd=TEST_DIR,
    )
    run_command(
        ["make", str(test_object.relative_to(TEST_DIR))],
        cwd=TEST_DIR,
    )
    return [main_object, test_object]


def symbol_search_patterns(symbol_name):
    stripped = symbol_name.strip()
    function_prefix = stripped.split("(")[0].strip()
    patterns = [function_prefix + "("]
    if "::" not in function_prefix:
        patterns.append(function_prefix)
    return patterns


def grep_candidate_sources(symbol_name, source_list):
    matches = []
    seen_paths = set()
    for pattern in symbol_search_patterns(symbol_name):
        command = ["rg", "-F", "-l", pattern]
        command.extend(str(source) for source in source_list)
        result = run_command(command, capture_output=True, check=False)
        for line in result.stdout.splitlines():
            path = Path(line).resolve()
            if path not in seen_paths:
                seen_paths.add(path)
                matches.append(path)
        if matches:
            break
    return matches


def module_object_path(source_path, objdir):
    relative_path = source_path.relative_to(LIBFT_DIR)
    module_dir = relative_path.parts[0]
    object_name = source_path.stem + ".o"
    return LIBFT_DIR / module_dir / objdir / object_name


def build_module_object(source_path, objdir, compile_flags):
    relative_path = source_path.relative_to(LIBFT_DIR)
    module_dir = relative_path.parts[0]
    object_path = module_object_path(source_path, objdir)
    result = run_command(
        [
            "make",
            str(object_path.relative_to(LIBFT_DIR / module_dir)),
            f"COMPILE_FLAGS={compile_flags}",
        ],
        cwd=LIBFT_DIR / module_dir,
        check=False,
        capture_output=True,
    )
    if result.returncode != 0:
        return None
    return object_path


def object_defines_symbol(object_path, symbol_name):
    result = run_command(
        ["nm", "-C", str(object_path)],
        capture_output=True,
        check=False,
    )
    for line in result.stdout.splitlines():
        if f" {symbol_name}" in line and " U " not in line:
            return True
    return False


def add_object_symbols(provider_index, object_path):
    result = run_command(
        ["nm", "-C", str(object_path)],
        capture_output=True,
        check=False,
    )
    for line in result.stdout.splitlines():
        stripped = line.strip()
        if not stripped or " U " in stripped:
            continue
        parts = stripped.split()
        if len(parts) < 2:
            continue
        symbol_name = " ".join(parts[2:]) if len(parts) > 2 else parts[-1]
        if not symbol_name:
            continue
        providers = provider_index.setdefault(symbol_name, [])
        if object_path not in providers:
            providers.append(object_path)
    return


def add_batched_object_symbols(provider_index, object_paths):
    batch_size = 100
    index = 0
    while index < len(object_paths):
        batch = object_paths[index:index + batch_size]
        command = ["nm", "-C", "-A"]
        command.extend(str(path) for path in batch)
        result = run_command(command, capture_output=True, check=False)
        for line in result.stdout.splitlines():
            stripped = line.strip()
            if not stripped:
                continue
            if ": " not in stripped:
                continue
            path_text, symbol_text = stripped.split(": ", 1)
            if " U " in symbol_text:
                continue
            parts = symbol_text.split()
            if len(parts) < 2:
                continue
            symbol_name = " ".join(parts[2:]) if len(parts) > 2 else parts[-1]
            if not symbol_name:
                continue
            object_path = Path(path_text).resolve()
            providers = provider_index.setdefault(symbol_name, [])
            if object_path not in providers:
                providers.append(object_path)
        index += batch_size
    return


def build_provider_index(objdir):
    provider_index = {}
    add_batched_object_symbols(provider_index, existing_module_objects(objdir))
    return provider_index


def undefined_symbols(object_paths):
    command = ["nm", "-uC"]
    command.extend(str(path) for path in object_paths)
    result = run_command(command, capture_output=True, check=False)
    symbols = []
    seen = set()
    for line in result.stdout.splitlines():
        stripped = line.strip()
        if stripped.startswith("U "):
            symbol_name = stripped[2:].strip()
            if symbol_name not in seen:
                seen.add(symbol_name)
                symbols.append(symbol_name)
    return symbols


def should_resolve_symbol(symbol_name):
    project_prefixes = (
        "ft_",
        "pt_",
        "cma_",
        "scma_",
        "rng_",
        "cmp_",
        "su_",
    )
    if symbol_name.startswith(project_prefixes):
        return True
    if "::" in symbol_name and symbol_name.startswith("ft_"):
        return True
    return False


def extract_linker_undefined_symbols(stderr_text):
    pattern = re.compile(r"undefined reference to `(.*?)'")
    return pattern.findall(stderr_text)


def link_single_test(binary_path, object_paths, ldflags):
    command = ["g++", "-o", str(binary_path)]
    command.extend(str(path) for path in object_paths)
    command.extend(shlex.split(ldflags))
    return run_command(command, capture_output=True, check=False)


def resolve_objects(test_source):
    compile_flags = read_make_value("print_test_compile_flags")
    ldflags = read_make_value("print_test_ldflags")
    objdir = read_make_value("print_test_objdir")
    object_paths = build_test_object(test_source, objdir)
    built_sources = {}
    provider_index = build_provider_index(objdir)
    source_list = module_sources()
    progress_made = True
    while progress_made:
        progress_made = False
        current_undefined = undefined_symbols(object_paths)
        if not current_undefined:
            break
        for symbol_name in current_undefined:
            if not should_resolve_symbol(symbol_name):
                continue
            providers = provider_index.get(symbol_name, [])
            for provider in providers:
                if provider not in object_paths:
                    object_paths.append(provider)
                    progress_made = True
                    break
            if progress_made:
                continue
            candidates = grep_candidate_sources(symbol_name, source_list)
            for candidate in candidates:
                if candidate in built_sources:
                    object_path = built_sources[candidate]
                else:
                    object_path = build_module_object(candidate, objdir, compile_flags)
                    built_sources[candidate] = object_path
                if object_path is None:
                    continue
                add_object_symbols(provider_index, object_path)
                if object_path not in object_paths and object_defines_symbol(object_path, symbol_name):
                    object_paths.append(object_path)
                    progress_made = True
                    break
        if not progress_made:
            break
    return object_paths, ldflags


def ensure_binary(test_source):
    SINGLE_TEST_DIR.mkdir(parents=True, exist_ok=True)
    object_paths, ldflags = resolve_objects(test_source)
    binary_path = SINGLE_TEST_DIR / test_source.stem
    link_result = link_single_test(binary_path, object_paths, ldflags)
    if link_result.returncode == 0:
        return binary_path
    unresolved = extract_linker_undefined_symbols(link_result.stderr)
    if unresolved:
        missing = "\n".join(unresolved)
        raise SystemExit(f"Failed to resolve all object dependencies for {test_source.name}:\n{missing}")
    raise SystemExit(link_result.stderr)


def run_binary(binary_path, test_source):
    command = [str(binary_path)]
    result = subprocess.run(command, cwd=TEST_DIR, text=True)
    return result.returncode


def main():
    if len(sys.argv) != 2:
        raise SystemExit("Usage: build_single_test.py Test/test_name.cpp")
    test_source = resolve_test_path(sys.argv[1])
    binary_path = ensure_binary(test_source)
    raise SystemExit(run_binary(binary_path, test_source))


if __name__ == "__main__":
    main()
