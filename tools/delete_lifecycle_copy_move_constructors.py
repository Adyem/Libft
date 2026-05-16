#!/usr/bin/env python3
"""Delete copy and move constructors for lifecycle classes in headers.

A lifecycle class is defined for this migration pass as a class whose class body
declares an initialize(...) or initialise(...) method.  The script only edits
.hpp files and verifies every detected lifecycle class after rewriting.
"""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence


EXCLUDED_DIRS = {".git", ".cache", "Demo", "Docs", "Test"}
HEADER_PATTERN = "*.hpp"
CLASS_PATTERN = re.compile(
    r"\bclass\s+([A-Za-z_][A-Za-z0-9_]*)(?:\s*[^;{}]*)?\{",
    re.MULTILINE,
)


@dataclass
class ClassBlock:
    name: str
    open_brace: int
    close_brace: int


@dataclass
class RewriteResult:
    path: Path
    lifecycle_classes: list[str]
    changed: bool
    errors: list[str]


def collect_headers(root: Path) -> Sequence[Path]:
    headers: list[Path] = []
    for candidate in root.rglob(HEADER_PATTERN):
        if not candidate.is_file():
            continue
        if EXCLUDED_DIRS & set(candidate.relative_to(root).parts):
            continue
        headers.append(candidate)
    return sorted(headers)


def mask_comments_and_strings(text: str) -> str:
    characters = list(text)
    index = 0
    while index < len(characters):
        if text.startswith("//", index):
            end_index = text.find("\n", index)
            if end_index == -1:
                end_index = len(characters)
            while index < end_index:
                characters[index] = " "
                index += 1
            continue
        if text.startswith("/*", index):
            end_index = text.find("*/", index + 2)
            if end_index == -1:
                end_index = len(characters) - 2
            end_index += 2
            while index < end_index:
                if characters[index] != "\n":
                    characters[index] = " "
                index += 1
            continue
        if characters[index] == '"' or characters[index] == "'":
            quote = characters[index]
            index += 1
            while index < len(characters):
                if characters[index] == "\\":
                    characters[index] = " "
                    index += 2
                    continue
                if characters[index] == quote:
                    index += 1
                    break
                if characters[index] != "\n":
                    characters[index] = " "
                index += 1
            continue
        index += 1
    return "".join(characters)


def find_matching_brace(masked_text: str, open_brace: int) -> int:
    depth = 1
    index = open_brace + 1
    while index < len(masked_text):
        if masked_text[index] == "{":
            depth += 1
        elif masked_text[index] == "}":
            depth -= 1
            if depth == 0:
                return index
        index += 1
    return -1


def enumerate_class_blocks(text: str) -> Iterable[ClassBlock]:
    masked_text = mask_comments_and_strings(text)
    for match in CLASS_PATTERN.finditer(masked_text):
        open_brace = match.end() - 1
        close_brace = find_matching_brace(masked_text, open_brace)
        if close_brace == -1:
            continue
        yield ClassBlock(match.group(1), open_brace, close_brace)


def direct_class_body(masked_body: str) -> str:
    characters = list(masked_body)
    depth = 0
    index = 0
    while index < len(characters):
        if characters[index] == "{":
            depth += 1
            characters[index] = " "
            index += 1
            continue
        if characters[index] == "}":
            if depth > 0:
                depth -= 1
            characters[index] = " "
            index += 1
            continue
        if depth > 0 and characters[index] != "\n":
            characters[index] = " "
        index += 1
    return "".join(characters)


def class_declares_initialize(text: str, block: ClassBlock) -> bool:
    masked_text = mask_comments_and_strings(text)
    body = masked_text[block.open_brace + 1:block.close_brace]
    direct_body = direct_class_body(body)
    return re.search(r"\binitiali[sz]e\s*\(", direct_body) is not None


def has_deleted_constructor(body: str, class_name: str, move_constructor: bool) -> bool:
    self_type = rf"{re.escape(class_name)}(?:\s*<[^;{{}}()]*>)?"
    if move_constructor:
        parameter = rf"{self_type}\s*&&(?:\s*[A-Za-z_][A-Za-z0-9_]*)?"
    else:
        parameter = (
            rf"const\s+{self_type}\s*&"
            rf"(?:\s*[A-Za-z_][A-Za-z0-9_]*)?"
        )
    pattern = re.compile(
        rf"\b{re.escape(class_name)}\s*\(\s*{parameter}\s*\)"
        rf"(?P<suffix>[^;{{}}]*)=\s*delete\s*;",
        re.MULTILINE,
    )
    return pattern.search(body) is not None


def delete_existing_constructor(body: str, class_name: str, move_constructor: bool) -> tuple[str, bool]:
    self_type = rf"{re.escape(class_name)}(?:\s*<[^;{{}}()]*>)?"
    if move_constructor:
        parameter = rf"{self_type}\s*&&(?:\s*[A-Za-z_][A-Za-z0-9_]*)?"
    else:
        parameter = (
            rf"const\s+{self_type}\s*&"
            rf"(?:\s*[A-Za-z_][A-Za-z0-9_]*)?"
        )
    pattern = re.compile(
        rf"(?P<declaration>\b{re.escape(class_name)}\s*\(\s*{parameter}\s*\)"
        rf"(?P<suffix>[^;{{}}]*));",
        re.MULTILINE,
    )
    changed = False

    def replace(match: re.Match[str]) -> str:
        nonlocal changed
        declaration = match.group("declaration")
        suffix = match.group("suffix")
        if re.search(r"=\s*delete\s*$", suffix):
            return match.group(0)
        declaration = re.sub(r"\s*=\s*default\s*$", "", declaration)
        declaration = re.sub(r"\s*=\s*delete\s*$", "", declaration)
        changed = True
        return f"{declaration} = delete;"

    return pattern.sub(replace, body), changed


def remove_duplicate_deleted_constructors(body: str, class_name: str) -> tuple[str, bool]:
    self_type = rf"{re.escape(class_name)}(?:\s*<[^;{{}}()]*>)?"
    changed = False
    for parameter in (
        rf"const\s+{self_type}\s*&(?:\s*[A-Za-z_][A-Za-z0-9_]*)?",
        rf"{self_type}\s*&&(?:\s*[A-Za-z_][A-Za-z0-9_]*)?",
    ):
        pattern = re.compile(
            rf"(?P<line>[ \t]*\b{re.escape(class_name)}\s*\(\s*{parameter}\s*\)"
            rf"[^;{{}}]*=\s*delete\s*;[ \t]*(?:\n|$))",
            re.MULTILINE,
        )
        seen = False

        def replace(match: re.Match[str]) -> str:
            nonlocal seen
            nonlocal changed
            if not seen:
                seen = True
                return match.group("line")
            changed = True
            return ""

        body = pattern.sub(replace, body)
    return body, changed


def find_default_constructor_end(body: str, class_name: str) -> int:
    pattern = re.compile(
        rf"\b{re.escape(class_name)}\s*\(\s*\)(?:\s*[^;{{}}]*)?;",
        re.MULTILINE,
    )
    match = pattern.search(body)
    if match is None:
        return -1
    return match.end()


def constructor_indent(body: str, constructor_end: int) -> str:
    line_start = body.rfind("\n", 0, constructor_end)
    if line_start == -1:
        return ""
    line_start += 1
    match = re.match(r"[ \t]*", body[line_start:constructor_end])
    if match is None:
        return ""
    return match.group(0)


def insert_missing_constructors(body: str, class_name: str) -> tuple[str, bool, list[str]]:
    errors: list[str] = []
    changed = False
    needs_copy = not has_deleted_constructor(body, class_name, False)
    needs_move = not has_deleted_constructor(body, class_name, True)
    if not needs_copy and not needs_move:
        return body, changed, errors
    constructor_end = find_default_constructor_end(body, class_name)
    if constructor_end == -1:
        errors.append(f"{class_name}: no default constructor declaration found")
        return body, changed, errors
    indent = constructor_indent(body, constructor_end)
    additions: list[str] = []
    if needs_copy:
        additions.append(f"{indent}{class_name}(const {class_name} &other) = delete;")
    if needs_move:
        additions.append(f"{indent}{class_name}({class_name} &&other) = delete;")
    insertion = "\n" + "\n".join(additions)
    body = body[:constructor_end] + insertion + body[constructor_end:]
    changed = True
    return body, changed, errors


def rewrite_class_body(body: str, class_name: str) -> tuple[str, bool, list[str]]:
    errors: list[str] = []
    changed = False
    body, constructor_changed = delete_existing_constructor(body, class_name, False)
    changed = changed or constructor_changed
    body, constructor_changed = delete_existing_constructor(body, class_name, True)
    changed = changed or constructor_changed
    body, duplicate_changed = remove_duplicate_deleted_constructors(body, class_name)
    changed = changed or duplicate_changed
    body, insertion_changed, insertion_errors = insert_missing_constructors(body, class_name)
    changed = changed or insertion_changed
    errors.extend(insertion_errors)
    if not has_deleted_constructor(body, class_name, False):
        errors.append(f"{class_name}: copy constructor is not deleted")
    if not has_deleted_constructor(body, class_name, True):
        errors.append(f"{class_name}: move constructor is not deleted")
    return body, changed, errors


def rewrite_header(path: Path) -> RewriteResult:
    original_text = path.read_text(encoding="utf-8")
    blocks = [
        block for block in enumerate_class_blocks(original_text)
        if class_declares_initialize(original_text, block)
    ]
    if not blocks:
        return RewriteResult(path, [], False, [])
    text = original_text
    changed = False
    errors: list[str] = []
    for block in sorted(blocks, key=lambda item: item.open_brace, reverse=True):
        body = text[block.open_brace + 1:block.close_brace]
        new_body, body_changed, body_errors = rewrite_class_body(body, block.name)
        if body_changed:
            text = text[:block.open_brace + 1] + new_body + text[block.close_brace:]
            changed = True
        errors.extend(body_errors)
    if changed:
        path.write_text(text, encoding="utf-8")
    verification_text = path.read_text(encoding="utf-8")
    for block in enumerate_class_blocks(verification_text):
        if not class_declares_initialize(verification_text, block):
            continue
        body = verification_text[block.open_brace + 1:block.close_brace]
        if not has_deleted_constructor(body, block.name, False):
            errors.append(f"{block.name}: verification failed for copy constructor")
        if not has_deleted_constructor(body, block.name, True):
            errors.append(f"{block.name}: verification failed for move constructor")
    lifecycle_classes = [block.name for block in blocks]
    return RewriteResult(path, lifecycle_classes, changed, errors)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Delete lifecycle class copy and move constructors in .hpp files.",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Verify headers without writing changes.",
    )
    args = parser.parse_args()
    repo_root = Path(__file__).resolve().parents[1]
    results: list[RewriteResult] = []
    for header in collect_headers(repo_root):
        if args.check:
            text = header.read_text(encoding="utf-8")
            errors: list[str] = []
            lifecycle_classes: list[str] = []
            for block in enumerate_class_blocks(text):
                if not class_declares_initialize(text, block):
                    continue
                lifecycle_classes.append(block.name)
                body = text[block.open_brace + 1:block.close_brace]
                if not has_deleted_constructor(body, block.name, False):
                    errors.append(f"{block.name}: copy constructor is not deleted")
                if not has_deleted_constructor(body, block.name, True):
                    errors.append(f"{block.name}: move constructor is not deleted")
            if lifecycle_classes or errors:
                results.append(RewriteResult(header, lifecycle_classes, False, errors))
            continue
        result = rewrite_header(header)
        if result.lifecycle_classes or result.changed or result.errors:
            results.append(result)
    changed_files = [result for result in results if result.changed]
    lifecycle_count = sum(len(result.lifecycle_classes) for result in results)
    for result in results:
        if result.changed:
            print(f"updated {result.path.relative_to(repo_root)}")
        for error in result.errors:
            print(f"error {result.path.relative_to(repo_root)}: {error}")
    print(f"lifecycle classes checked: {lifecycle_count}")
    print(f"headers updated: {len(changed_files)}")
    if any(result.errors for result in results):
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
