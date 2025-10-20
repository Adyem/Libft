# Canonical Formatting Guide for Structured Document Modules

This guide defines the canonical formatting conventions used by the JSON, YAML, XML, and HTML modules. Apply these rules when producing serialized output, updating fixtures, or reviewing patches so diffs remain predictable and easy to audit across teams.

## Shared principles

- **UTF-8 everywhere.** Emit UTF-8 encoded text with explicit BOMs disabled. Modules that ingest alternate encodings must normalize to UTF-8 before serialization.
- **Deterministic key ordering.** When a format supports mappings (objects, dictionaries, attributes), sort keys lexicographically using byte-wise comparisons. Preserve stable ordering for arrays/lists.
- **Two-space indentation.** Indent nested structures using two spaces. Never mix tabs and spaces. Align wrapped attributes or tag arguments under the first character following the delimiter.
- **Trailing newline.** Terminate every document with a single newline to avoid churn when appending content.
- **Normalized whitespace.** Collapse multiple blank lines to a single blank line within logical sections and strip trailing spaces at line ends.
- **Stable numeric formatting.** Serialize integers without leading zeroes (except `0` itself) and floats using lowercase scientific notation when magnitudes exceed `1e6` or drop below `1e-6`.
- **Boolean and null casing.** Emit the canonical lowercase tokens (`true`, `false`, `null`) regardless of input casing when the target format allows freeform text.

## JSON

- **Object layout.** Place the opening brace on the same line as the property name when nested inside an object or array. Close braces on their own line aligned with the property that opened the scope.
- **String quoting.** Always use double quotes and escape control characters with `\u` sequences. Prefer explicit escaping over relying on readers to normalize.
- **Arrays.** Break arrays across multiple lines when they contain complex objects or exceed 80 characters. Keep primitive arrays (numbers, strings) on a single line when the rendered length is under 80 characters.
- **Comments.** JSON output must be comment-free. Inline metadata belongs in sibling keys.

## YAML

- **Document marker.** Emit the `---` marker for multi-document streams. Omit the terminating `...` unless required by downstream tooling.
- **Indentation.** Use two spaces per level; nested lists must indent by two spaces relative to their parent key.
- **Quoting.** Prefer plain scalars. Use double quotes when strings contain colon-space (`: `), leading/trailing whitespace, or Unicode that is easier to read than escaped sequences. Use single quotes to avoid interpreting escape sequences when strings already contain backslashes.
- **Anchors and aliases.** Declare anchors immediately before first use and keep alias names descriptive (e.g., `&base_config`).
- **Multiline strings.** Default to the folded style (`>`) for prose and the literal style (`|`) for code snippets to preserve formatting fidelity.

## XML

- **Declaration.** Start documents with `<?xml version="1.0" encoding="UTF-8"?>`.
- **Attribute ordering.** Sort attributes lexicographically. Group namespace declarations (`xmlns`) ahead of regular attributes.
- **Empty elements.** Prefer self-closing tags (`<tag />`) when an element has no content.
- **Wrapping.** Wrap text content at 100 columns where possible. Insert line breaks before closing tags when wrapping mixed content to keep diffs stable.
- **Comments.** Use comments sparingly. When present, dedicate a full line and ensure there is a single leading space inside the delimiters: `<!-- comment -->`.

## HTML

- **Doctype.** Emit `<!DOCTYPE html>` for HTML5 documents.
- **Tag casing.** Use lowercase tag names and attribute keys. Boolean attributes must be serialized without values (`disabled`, `checked`).
- **Void elements.** Do not add closing tags for void elements (`<br>`, `<img>`, `<hr>`). Do not self-close with `/>`.
- **Inline vs. block.** Keep inline tags on the same line as surrounding text. Block-level tags (e.g., `<div>`, `<section>`) open and close on their own lines.
- **Whitespace control.** Normalize sequences of spaces to a single space in text nodes unless semantics require preservation (e.g., `<pre>`). Maintain indentation inside `<script>` and `<style>` blocks using two spaces.
- **Attribute quoting.** Always use double quotes. Escape embedded quotes and angle brackets using HTML entities.

## Validation checklist

Before merging changes that touch serialized fixtures or templates:

1. Round-trip the document through its parser and serializer to confirm stability.
2. Verify that structural diffs (`tools/json_diff.py`, `tools/yaml_diff.py`, `tools/xml_diff.py`, `tools/html_diff.py`) report no semantic changes beyond the intended edits.
3. Confirm that automated formatters (where available) produce identical output.
4. Re-run unit tests covering the affected module to ensure schema expectations remain valid.

Following these conventions keeps repository diffs focused on meaningful changes and prevents downstream churn in generated assets.
