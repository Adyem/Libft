# Regex

The `Regex` module provides ECMAScript-style pattern validation, full-match and search helpers, replacement helpers, and capture-span extraction for text processing workflows.

## Public API

- `regex_validate_pattern(const char *pattern)` - Compiles a pattern and reports whether it is valid.
- `regex_is_match(const char *pattern, const char *text, ft_bool *is_match)` - Reports whether the entire text matches the pattern.
- `regex_search(const char *pattern, const char *text, ft_bool *is_found)` - Reports whether any substring match exists.
- `regex_replace_all(const char *pattern, const char *replacement, const char *text, ft_string *output)` - Writes the fully replaced text into an initialized `ft_string`.
- `regex_capture_count(const char *pattern, ft_size_t *capture_count)` - Reports the total capture slots, including the full match at index `0`.
- `regex_capture_groups(const char *pattern, const char *text, regex_capture_group *captures, ft_size_t capture_capacity, ft_size_t *capture_count)` - Fills capture spans with matched flags, start indices, and lengths.

## Types

- `regex_capture_group` - Span description for one regex result entry. `matched` reports whether the group participated, `start_index` is the byte offset in the input text, and `length` is the matched span length.
- `regex_count_matches(...)` - Counts non-overlapping matches without requiring capture storage.
