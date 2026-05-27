# Basic

The `Basic` module provides C-style memory, string, character, numeric parsing, hashing, and UTF conversion helpers. Most functions operate on caller-owned buffers. Functions returning allocated text, such as UTF conversion helpers and `ft_span_dup`, transfer ownership to the caller.

## Core Types

- `ft_size_t` - Project-wide unsigned size type.
- `ft_bool` - Project-wide 8-bit boolean type.
- `FT_FALSE` / `FT_TRUE` - Boolean constants used across the library.

## String Length

- `ft_strlen_raw(const char *string)` - Returns the length of a non-null C string as `ft_size_t`.
- `ft_strlen_size_t(const char *string)` - Returns a C string length as `ft_size_t`, or `0` for null.
- `ft_strlen(const char *string)` - Returns a C string length clamped to `FT_INT32_MAX`, or `0` for null.
- `ft_strnlen(const char *string, ft_size_t maximum_length)` - Counts characters up to a maximum bound.
- `ft_wstrlen(const wchar_t *string)` - Returns the length of a wide-character string.

## String Search and Compare

- `ft_strchr(const char *string, int32_t char_to_find)` - Finds the first matching character.
- `ft_strrchr(const char *string, int32_t char_to_find)` - Finds the last matching character.
- `ft_strstr(const char *haystack, const char *needle)` - Finds a substring.
- `ft_strnstr(const char *haystack, const char *needle, ft_size_t maximum_length)` - Finds a substring inside a bounded prefix.
- `ft_strcmp(const char *string1, const char *string2)` - Compares two C strings.
- `ft_strncmp(const char *string_1, const char *string_2, ft_size_t maximum_length)` - Compares bounded prefixes of two C strings.
- `ft_locale_compare(const char *left, const char *right, const char *locale_name)` - Compares strings using locale-aware collation.

## String Copy, Append, and Transform

- `ft_strlcpy(char *destination, const char *source, ft_size_t buffer_size)` - Copies a C string with truncation reporting.
- `ft_strlcat(char *destination, const char *source, ft_size_t buffer_size)` - Appends a C string with truncation reporting.
- `ft_strncpy(char *destination, const char *source, ft_size_t number_of_characters)` - Copies at most a fixed number of characters.
- `ft_strcpy_s(char *destination, ft_size_t destination_size, const char *source)` - Bounds-checked string copy returning an error code.
- `ft_strncpy_s(char *destination, ft_size_t destination_size, const char *source, ft_size_t maximum_copy_length)` - Bounds-checked limited string copy.
- `ft_strcat_s(char *destination, ft_size_t destination_size, const char *source)` - Bounds-checked string append.
- `ft_strncat_s(char *destination, ft_size_t destination_size, const char *source, ft_size_t maximum_append_length)` - Bounds-checked limited string append.
- `ft_to_lower(char *string)` - Converts ASCII letters in a mutable C string to lowercase.
- `ft_to_upper(char *string)` - Converts ASCII letters in a mutable C string to uppercase.
- `ft_strmapi(const char *string, char (*function)(uint32_t, char))` - Allocates a mapped copy of a string using an index-aware callback.
- `ft_striteri(char *string, void (*function)(uint32_t, char *))` - Applies an index-aware callback to each mutable character.
- `ft_strtok(char *string, const char *delimiters)` - Tokenizes a string using delimiter characters.
- `ft_span_dup(const char *buffer, ft_size_t length)` - Allocates and copies a non-null-terminated character span.

## Memory Helpers

- `ft_bzero(void *string, ft_size_t size)` - Sets a memory range to zero.
- `ft_memset(void *destination, int32_t value, ft_size_t number_of_bytes)` - Fills memory with a byte value.
- `ft_memcpy(void *destination, const void *source, ft_size_t size)` - Copies non-overlapping memory.
- `ft_memcpy_s(void *destination, ft_size_t destination_size, const void *source, ft_size_t number_of_bytes)` - Bounds-checked memory copy returning an error code.
- `ft_memmove(void *destination, const void *source, ft_size_t size)` - Copies memory safely when ranges overlap.
- `ft_memmove_s(void *destination, ft_size_t destination_size, const void *source, ft_size_t number_of_bytes)` - Bounds-checked overlap-safe memory copy.
- `ft_memchr(const void *pointer, int32_t character, ft_size_t size)` - Searches memory for a byte value.
- `ft_memrchr(const void *pointer, int32_t character, ft_size_t size)` - Searches memory for the last matching byte value.
- `ft_memcmp(const void *pointer1, const void *pointer2, ft_size_t size)` - Compares two memory ranges.
- `ft_constant_time_equal(const void *pointer1, const void *pointer2, ft_size_t size)` - Compares two memory ranges without early exit for timing-sensitive checks.

## Character Checks

- `ft_isdigit(int32_t character)` - Tests for an ASCII digit.
- `ft_isalpha(int32_t character)` - Tests for an ASCII letter.
- `ft_isalnum(int32_t character)` - Tests for an ASCII letter or digit.
- `ft_isprint(int32_t character)` - Tests for a printable ASCII character.
- `ft_islower(int32_t character)` - Tests for a lowercase ASCII letter.
- `ft_isupper(int32_t character)` - Tests for an uppercase ASCII letter.
- `ft_isspace(int32_t character)` - Tests for an ASCII whitespace character.

## Numeric Parsing and Hashing

- `ft_atoi(const char *string)` - Parses a decimal string into `int32_t`.
- `ft_validate_int(const char *input)` - Validates that a string can be parsed as an integer.
- `ft_atol(const char *string)` - Parses a decimal string into `int64_t`.
- `ft_strtol(const char *input_string, char **end_pointer, int32_t numeric_base)` - Parses a signed integer with caller-selected base and end pointer reporting.
- `ft_strtoul(const char *input_string, char **end_pointer, int32_t numeric_base)` - Parses an unsigned integer with caller-selected base and end pointer reporting.
- `ft_hash_string31(const char *string)` - Computes the module's 31-based hash for a C string.
- `ft_hash_string31(const ft_string &string)` - Computes the same 31-based hash for an `ft_string`.

## UTF Helpers

- `ft_utf8_to_utf16(const char *input, ft_size_t input_length, ft_size_t *output_length_pointer)` - Converts UTF-8 text to an allocated UTF-16 buffer.
- `ft_utf8_to_utf32(const char *input, ft_size_t input_length, ft_size_t *output_length_pointer)` - Converts UTF-8 text to an allocated UTF-32 buffer.
- `ft_utf8_next(const char *string, ft_size_t string_length, ft_size_t *index_pointer, uint32_t *code_point_pointer, ft_size_t *sequence_length_pointer)` - Decodes the next code point and advances the index.
- `ft_utf8_count(const char *string, ft_size_t *code_point_count_pointer)` - Counts UTF-8 code points in a null-terminated string.
- `ft_utf8_encode(uint32_t code_point, char *buffer, ft_size_t buffer_size, ft_size_t *encoded_length_pointer)` - Encodes one Unicode code point into UTF-8.
- `ft_utf8_transform(const char *input, ft_size_t input_length, char *output_buffer, ft_size_t output_buffer_size, ft_utf8_case_hook case_hook)` - Applies a code-point transform into a caller-owned buffer.
- `ft_utf8_transform_alloc(const char *input, char **output_pointer, ft_utf8_case_hook case_hook)` - Allocates a transformed UTF-8 string.
- `ft_utf8_case_ascii_lower(uint32_t code_point)` - Lowercases ASCII code points and leaves others unchanged.
- `ft_utf8_case_ascii_upper(uint32_t code_point)` - Uppercases ASCII code points and leaves others unchanged.
- `ft_utf8_is_combining_code_point(uint32_t code_point)` - Reports whether a code point is a combining mark handled by this module.
- `ft_utf8_next_grapheme(const char *string, ft_size_t string_length, ft_size_t *index_pointer, ft_size_t *grapheme_length_pointer)` - Finds the next grapheme span and advances the index.
- `ft_utf8_duplicate_grapheme(const char *string, ft_size_t string_length, ft_size_t *index_pointer, char **grapheme_pointer)` - Allocates a copy of the next grapheme and advances the index.
