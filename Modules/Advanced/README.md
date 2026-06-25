# Advanced

The `Advanced` module contains allocation-returning convenience helpers built on top of `Basic`, `CPP_class`, and CMA-style ownership. Functions that return `char *`, `void *`, `int32_t *`, or `ft_string *` allocate a new result; callers own the returned pointer and must release it with the matching project allocator or object lifecycle.

## String and Number Conversion

- `adv_to_string(int64_t number)` - Allocates an `ft_string` containing a signed 64-bit decimal representation.
- `adv_to_string(uint64_t number)` - Allocates an `ft_string` containing an unsigned 64-bit decimal representation.
- `adv_to_string(double number)` - Allocates an `ft_string` containing a decimal representation of a `double`.
- `adv_to_string(int32_t number)` - Allocates an `ft_string` containing a signed 32-bit decimal representation.
- `adv_to_string(uint32_t number)` - Allocates an `ft_string` containing an unsigned 32-bit decimal representation.
- `adv_to_string(float number)` - Allocates an `ft_string` containing a decimal representation of a `float`.
- `adv_atoi(const char *string)` - Parses a C string into an allocated `int32_t` result.
- `adv_itoa(int32_t number)` - Allocates a decimal C string for an integer.
- `adv_itoa_base(int32_t number, int32_t base)` - Allocates a C string for an integer in the requested numeric base.

## Text Helpers

- `ft_hash_string31(const char *string)` - Computes the module's 31-based hash for a C string.
- `ft_hash_string31(const ft_string &string)` - Computes the same 31-based hash for an `ft_string`.
- `ft_span_dup(const char *buffer, ft_size_t length)` - Allocates and copies a non-null-terminated character span.
- `ft_strmapi(const char *string, char (*function)(uint32_t, char))` - Allocates a mapped copy of a string using an index-aware callback.
- `adv_str_to_title_case(const char *input_string)` - Allocates a title-cased copy of the input string.
- `adv_str_to_snake_case(const char *input_string)` - Allocates a lowercase snake_case copy of the input string.
- `adv_str_to_camel_case(const char *input_string)` - Allocates a camelCase copy of the input string.
- `adv_str_normalize_whitespace(const char *input_string)` - Allocates a copy with whitespace collapsed to single spaces and trimmed at the ends.
- `adv_utf16_to_utf8(const char16_t *input, ft_size_t input_length)` - Converts a UTF-16 span into an allocated UTF-8 `ft_string`.
- `adv_utf32_to_utf8(const char32_t *input, ft_size_t input_length)` - Converts a UTF-32 span into an allocated UTF-8 `ft_string`.
- `adv_locale_casefold(const char *input, const char *locale_name)` - Allocates a locale-aware folded string for comparisons.
- `adv_span_to_string(const char *buffer, ft_size_t length)` - Copies a byte span into an allocated `ft_string`.
- `adv_strdup(const char *string)` - Duplicates a null-terminated C string.
- `adv_strndup(const char *string, ft_size_t maximum_length)` - Duplicates at most `maximum_length` bytes from a C string.
- `adv_strjoin(char const *string_1, char const *string_2)` - Allocates the concatenation of two C strings.
- `adv_strjoin_multiple(int32_t count, ...)` - Allocates the concatenation of `count` C strings from varargs.
- `adv_strjoin_vector(const ft_vector<ft_string> &strings, const char *separator)` - Allocates a joined `ft_string` from a vector of strings.
- `adv_format_string(const char *format, ...)` - Allocates an `ft_string` using `printf`-style formatting.
- `adv_substr(const char *source, uint32_t start, ft_size_t length)` - Allocates a substring beginning at `start`.
- `adv_strtrim(const char *input_string, const char *set)` - Allocates a copy with characters from `set` trimmed from both ends.
- `adv_split(char const *string, char delimiter)` - Splits a string by delimiter into a null-terminated array of allocated strings.

## UTF Helpers

- `ft_utf8_case_ascii_lower(uint32_t code_point)` - Lowercases ASCII code points and leaves others unchanged.
- `ft_utf8_case_ascii_upper(uint32_t code_point)` - Uppercases ASCII code points and leaves others unchanged.
- `ft_utf8_is_combining_code_point(uint32_t code_point)` - Reports whether a code point is a combining mark handled by this module.
- `ft_utf8_transform(const char *input, ft_size_t input_length, char *output_buffer, ft_size_t output_buffer_size, ft_utf8_case_hook case_hook)` - Applies a code-point transform into a caller-owned buffer.
- `ft_utf8_transform_alloc(const char *input, char **output_pointer, ft_utf8_case_hook case_hook)` - Allocates a transformed UTF-8 string.
- `ft_utf8_next_grapheme(const char *string, ft_size_t string_length, ft_size_t *index_pointer, ft_size_t *grapheme_length_pointer)` - Finds the next grapheme span and advances the index.
- `ft_utf8_duplicate_grapheme(const char *string, ft_size_t string_length, ft_size_t *index_pointer, char **grapheme_pointer)` - Allocates a copy of the next grapheme and advances the index.

## Memory Helpers

- `adv_memdup(const void *source, ft_size_t size)` - Allocates and copies `size` bytes from `source`.
- `adv_calloc(ft_size_t count, ft_size_t size)` - Allocates zero-initialized storage for `count * size` bytes.
