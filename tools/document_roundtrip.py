#!/usr/bin/env python3
"""Round-trip documents and highlight differences.

This utility helps regression testing by parsing supported document formats,
serialising them back into a canonical representation, and showing a unified
diff between the original text and the canonicalised output. It can optionally
write the canonical form back to disk.
"""

from __future__ import annotations

import argparse
import difflib
import html
import json
import sys
from dataclasses import dataclass
from html.parser import HTMLParser
from pathlib import Path
from typing import Callable, Dict, Iterable, List, Optional, Sequence, Tuple, Union

try:
    import yaml  # type: ignore
except ImportError:  # pragma: no cover - optional dependency
    yaml = None

import xml.dom.minidom


@dataclass
class RoundTripResult:
    canonical_text: str
    diff: str


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def write_text(path: Path, content: str) -> None:
    path.write_text(content, encoding="utf-8")


def detect_format(path: Path, forced_format: Optional[str]) -> str:
    if forced_format is not None:
        return forced_format

    extension = path.suffix.lower()
    mapping: Dict[str, str] = {
        ".json": "json",
        ".yaml": "yaml",
        ".yml": "yaml",
        ".xml": "xml",
        ".html": "html",
        ".htm": "html",
    }
    format_name = mapping.get(extension)
    if format_name is None:
        raise ValueError(f"Cannot infer document format from extension '{extension}'.")
    return format_name


def round_trip_json(text: str) -> str:
    parsed = json.loads(text)
    return json.dumps(parsed, indent=2, sort_keys=True, ensure_ascii=False) + "\n"


def round_trip_yaml(text: str) -> str:
    if yaml is None:
        raise RuntimeError("PyYAML is required for YAML round-tripping but is not installed.")
    parsed = yaml.safe_load(text)
    return yaml.safe_dump(parsed, sort_keys=True, allow_unicode=True)


def canonicalise_dom(dom: xml.dom.minidom.Document) -> str:
    for node in dom.childNodes:
        if node.nodeType == node.TEXT_NODE:
            node.data = node.data.strip()
    return dom.toprettyxml(indent="  ", encoding="utf-8").decode("utf-8")


def round_trip_xml(text: str) -> str:
    dom = xml.dom.minidom.parseString(text)
    return canonicalise_dom(dom)


@dataclass
class HtmlElement:
    tag: str
    attributes: Sequence[Tuple[str, Optional[str]]]
    children: List["HtmlNode"]
    self_closing: bool = False


@dataclass
class HtmlText:
    content: str


@dataclass
class HtmlComment:
    content: str


@dataclass
class HtmlDeclaration:
    content: str


HtmlNode = Union[HtmlElement, HtmlText, HtmlComment, HtmlDeclaration]


class CanonicalHtmlParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self._root = HtmlElement(tag="#document", attributes=(), children=[])
        self._stack: List[HtmlElement] = [self._root]

    @property
    def root(self) -> HtmlElement:
        return self._root

    def handle_starttag(self, tag: str, attrs: List[Tuple[str, Optional[str]]]) -> None:
        element = HtmlElement(tag=tag, attributes=sorted(attrs), children=[])
        self._stack[-1].children.append(element)
        self._stack.append(element)

    def handle_endtag(self, tag: str) -> None:
        for index in range(len(self._stack) - 1, 0, -1):
            if self._stack[index].tag == tag:
                del self._stack[index:]
                break

    def handle_startendtag(self, tag: str, attrs: List[Tuple[str, Optional[str]]]) -> None:
        element = HtmlElement(tag=tag, attributes=sorted(attrs), children=[], self_closing=True)
        self._stack[-1].children.append(element)

    def handle_data(self, data: str) -> None:
        if data.strip() == "":
            return
        self._stack[-1].children.append(HtmlText(content=data.strip()))

    def handle_comment(self, data: str) -> None:
        self._stack[-1].children.append(HtmlComment(content=data.strip()))

    def handle_decl(self, decl: str) -> None:
        self._stack[-1].children.append(HtmlDeclaration(content=decl.strip()))

    def error(self, message: str) -> None:  # pragma: no cover - interface requirement
        raise RuntimeError(message)


def serialize_html_node(node: HtmlNode, indent: int = 0) -> str:
    prefix = "  " * indent
    if isinstance(node, HtmlText):
        return f"{prefix}{node.content}\n"
    if isinstance(node, HtmlComment):
        return f"{prefix}<!--{node.content}-->\n"
    if isinstance(node, HtmlDeclaration):
        return f"{prefix}<!{node.content}>\n"
    attributes = "".join(
        f" {name}" if value is None else f" {name}=\"{html.escape(value, quote=True)}\""
        for name, value in node.attributes
    )
    if node.self_closing:
        return f"{prefix}<{node.tag}{attributes} />\n"
    if not node.children:
        return f"{prefix}<{node.tag}{attributes}></{node.tag}>\n"
    child_text = "".join(serialize_html_node(child, indent + 1) for child in node.children)
    return f"{prefix}<{node.tag}{attributes}>\n{child_text}{prefix}</{node.tag}>\n"


def round_trip_html(text: str) -> str:
    parser = CanonicalHtmlParser()
    parser.feed(text)
    child_text = "".join(serialize_html_node(child, 0) for child in parser.root.children)
    return child_text


ROUND_TRIPPERS: Dict[str, Callable[[str], str]] = {
    "json": round_trip_json,
    "yaml": round_trip_yaml,
    "xml": round_trip_xml,
    "html": round_trip_html,
}


def build_diff(original: str, canonical: str, path: Path) -> str:
    original_lines = original.splitlines()
    canonical_lines = canonical.splitlines()
    diff_lines = difflib.unified_diff(
        original_lines,
        canonical_lines,
        fromfile=str(path),
        tofile=f"{path} (canonical)",
        lineterm="",
    )
    return "\n".join(diff_lines)


def round_trip_path(path: Path, forced_format: Optional[str]) -> RoundTripResult:
    text = read_text(path)
    format_name = detect_format(path, forced_format)
    if format_name not in ROUND_TRIPPERS:
        raise ValueError(f"Unsupported format '{format_name}'.")
    canonical_text = ROUND_TRIPPERS[format_name](text)
    diff = build_diff(text, canonical_text, path)
    return RoundTripResult(canonical_text=canonical_text, diff=diff)


def parse_arguments(argv: Optional[Iterable[str]]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Round-trip documents and show diffs.")
    parser.add_argument("paths", nargs="+", type=Path, help="Document files to process.")
    parser.add_argument(
        "--format",
        choices=sorted(ROUND_TRIPPERS.keys()),
        help="Override format detection for all paths.",
    )
    parser.add_argument(
        "--write",
        action="store_true",
        help="Write the canonical form back to the original file.",
    )
    parser.add_argument(
        "--fail-on-diff",
        action="store_true",
        help="Exit with status 1 when a diff is detected.",
    )
    return parser.parse_args(argv)


def process_paths(args: argparse.Namespace) -> int:
    exit_code = 0
    for path in args.paths:
        if not path.exists():
            print(f"error: path '{path}' does not exist", file=sys.stderr)
            exit_code = 2
            continue
        if path.is_dir():
            print(f"error: '{path}' is a directory; please specify individual files", file=sys.stderr)
            exit_code = 2
            continue
        try:
            result = round_trip_path(path, args.format)
        except Exception as error:  # pragma: no cover - command line tool robustness
            print(f"error: failed to round-trip '{path}': {error}", file=sys.stderr)
            exit_code = 2
            continue
        if result.diff:
            print(result.diff)
            if args.write:
                write_text(path, result.canonical_text)
            if args.fail_on_diff and exit_code == 0:
                exit_code = 1
        else:
            print(f"no changes for {path}")
    return exit_code


def main(argv: Optional[Iterable[str]] = None) -> int:
    args = parse_arguments(argv)
    return process_paths(args)


if __name__ == "__main__":
    sys.exit(main())
