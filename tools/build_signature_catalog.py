#!/usr/bin/env python3
"""Generate a document summarizing function, constructor, destructor, and operator signatures."""

from __future__ import annotations

import argparse
import re
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import DefaultDict, Dict, Iterable, Mapping, Sequence, Set, Tuple


SOURCE_EXTENSIONS: Set[str] = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".hh",
    ".hpp",
    ".h",
    ".ipp",
}

EXCLUDED_DIRS: Set[str] = {"Docs", "tools", ".git"}

CONTROL_KEYWORDS: Set[str] = {
    "if",
    "for",
    "while",
    "switch",
    "catch",
    "return",
    "else",
    "sizeof",
    "throw",
    "do",
    "static_assert",
    "using",
    "typedef",
    "namespace",
}


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*", "", text)
    return text


def collapse_whitespace(fragment: str) -> str:
    return re.sub(r"\s+", " ", fragment.strip())


def collect_source_files(root: Path) -> Sequence[Path]:
    files: list[Path] = []
    for path in root.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix.lower() not in SOURCE_EXTENSIONS:
            continue
        if EXCLUDED_DIRS & set(path.parts):
            continue
        files.append(path)
    return sorted(files)


def is_control_structure(snippet: str) -> bool:
    trimmed = snippet.strip()
    first_word_match = re.match(r"([A-Za-z_]\w*)", trimmed)
    if not first_word_match:
        return False
    return first_word_match.group(1) in CONTROL_KEYWORDS


def find_class_bodies(text: str) -> Iterable[Tuple[str, str]]:
    pattern = re.compile(
        r"\b(class|struct)\s+([A-Za-z_]\w*)(?:\s*<[^>]*>)?(?:\s*[^;{]+)?\{",
        flags=re.DOTALL,
    )
    index = 0
    while True:
        match = pattern.search(text, index)
        if not match:
            break
        class_name = match.group(2)
        body_start = match.end() - 1
        brace_depth = 1
        cursor = body_start
        while cursor < len(text) and brace_depth > 0:
            character = text[cursor]
            if character == "{":
                brace_depth += 1
            elif character == "}":
                brace_depth -= 1
            cursor += 1
        body = text[body_start:cursor]
        yield class_name, body
        index = cursor


def collect_inline_constructors(
    class_name: str, body: str
) -> Tuple[Set[str], Set[str]]:
    constructors: Set[str] = set()
    destructors: Set[str] = set()
    pattern = re.compile(
        rf"\b(~?{re.escape(class_name)})\s*\([^;{{}}]*\)[^;{{}}]*[;{{}}]",
        flags=re.DOTALL,
    )
    for match in pattern.finditer(body):
        snippet = collapse_whitespace(match.group())
        if snippet.startswith(f"{class_name}") and "~" not in snippet.split("(", 1)[0]:
            constructors.add(snippet)
        if snippet.startswith(f"~{class_name}"):
            destructors.add(snippet)
    return constructors, destructors


def iter_top_level_candidates(text: str) -> Iterable[str]:
    depth = 0
    buffer: list[str] = []
    class_scope_depths: list[int] = []
    function_scope_depths: list[int] = []
    class_header_pattern = re.compile(r"(?:template\s*<[^>]+>\s*)?(class|struct|union)\b")
    namespace_pattern = re.compile(r"(?:template\s*<[^>]+>\s*)?namespace\b")

    def is_function_definition(candidate: str) -> bool:
        stripped = candidate.lstrip()
        if "(" not in stripped:
            return False
        if is_control_structure(stripped):
            return False
        if stripped.startswith(("class ", "struct ", "union ", "namespace ")):
            return False
        return True

    for character in text:
        buffer.append(character)
        if character == "{":
            candidate = "".join(buffer[:-1]).strip()
            if candidate and depth <= 1 and not (class_scope_depths and depth >= class_scope_depths[-1]):
                yield collapse_whitespace(candidate)
            if class_header_pattern.match(candidate):
                class_scope_depths.append(depth + 1)
            elif namespace_pattern.match(candidate):
                class_scope_depths.append(depth + 1)
            elif is_function_definition(candidate):
                function_scope_depths.append(depth + 1)
            depth += 1
            buffer = []
        elif character == ";":
            if (
                depth <= 1
                and not (class_scope_depths and depth >= class_scope_depths[-1])
                and not (function_scope_depths and depth >= function_scope_depths[-1])
            ):
                candidate = "".join(buffer).strip()
                if candidate:
                    yield collapse_whitespace(candidate)
            buffer = []
        elif character == "}":
            depth -= 1
            while class_scope_depths and depth < class_scope_depths[-1]:
                class_scope_depths.pop()
            while function_scope_depths and depth < function_scope_depths[-1]:
                function_scope_depths.pop()
            buffer = []


def scan_file(path: Path) -> Mapping[str, Set[str]]:
    text = strip_comments(path.read_text(encoding="utf-8"))
    signatures: DefaultDict[str, Set[str]] = defaultdict(set)

    for snippet in iter_top_level_candidates(text):
        if "(" not in snippet:
            continue
        if snippet.lstrip().startswith(("class ", "struct ", "union ")):
            continue
        if is_control_structure(snippet):
            continue
        if "static_assert" in snippet or snippet.startswith("#"):
            continue
        prefix = snippet.split("(", 1)[0]
        if "::" not in prefix and "=" in prefix:
            continue
        if "::" not in prefix and ("->" in prefix or "." in prefix):
            continue
        if "::" not in prefix and not re.search(r"\s", prefix):
            continue
        if "operator" in snippet:
            signatures["operators"].add(snippet)
            continue
        qualified_match = re.search(
            r"([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*::\s*(~?[A-Za-z_]\w*)\s*\(", snippet
        )
        if qualified_match:
            class_full = qualified_match.group(1)
            method_name = qualified_match.group(2)
            base_name = class_full.rsplit("::", 1)[-1]
            prefix_before_qualifier = prefix.split("::", 1)[0]
            has_return_type = bool(re.search(r"\s", prefix_before_qualifier))
            if method_name == base_name:
                signatures["constructors"].add(snippet)
                continue
            if method_name == f"~{base_name}":
                signatures["destructors"].add(snippet)
                continue
            if not has_return_type:
                continue
        signatures["functions"].add(snippet)

    for class_name, body in find_class_bodies(text):
        constructors, destructors = collect_inline_constructors(class_name, body)
        signatures["constructors"].update(constructors)
        signatures["destructors"].update(destructors)

    return signatures


def build_document(
    root: Path, scanned: Mapping[Path, Mapping[str, Set[str]]], script_path: Path
) -> str:
    lines = [
        "# Function Signature Catalog",
        "",
        "Generated by `tools/build_signature_catalog.py`.",
        "",
        "The catalog lists the function declarations and definitions that appear in the"
        " C/C++ sources so maintainers can quickly find overloads, constructors, destructors,"
        " and other helpers without reading every file manually.",
        "",
    ]

    sections: Sequence[Tuple[str, str]] = (
        ("Functions", "functions"),
        ("Constructors", "constructors"),
        ("Destructors", "destructors"),
        ("Operator Overloads", "operators"),
    )

    for title, key in sections:
        lines.append(f"## {title}")
        lines.append("")
        entries_exist = False
        for path in sorted(scanned):
            entries = scanned[path].get(key, set())
            if not entries:
                continue
            entries_exist = True
            rel_path = path.relative_to(root)
            lines.append(f"### {rel_path.as_posix()}")
            lines.append("")
            for snippet in sorted(entries):
                lines.append(f"- `{snippet}`")
            lines.append("")
        if not entries_exist:
            lines.append("_None yet recorded._")
            lines.append("")
    lines.append(f"Document regenerated via `{script_path.name}`.")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build a document listing function, constructor, destructor, and operator signatures."
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("Docs").resolve() / "function_signature_catalog.md",
        help="Destination document path.",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    source_files = collect_source_files(repo_root)
    scanned: Dict[Path, Mapping[str, Set[str]]] = {}

    for source in source_files:
        result = scan_file(source)
        if any(result.values()):
            scanned[source] = result

    doc_text = build_document(repo_root, scanned, Path(__file__))
    args.output.write_text(doc_text, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
