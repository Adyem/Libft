# Encoding

The `Encoding` module provides allocation-returning codecs for binary/text transformations plus UTF-8 validation helpers. Encoding and decoding functions return allocated buffers; callers own returned data and receive decoded sizes through output parameters where needed.

## Base64

- `encoding_base64_encode(const uint8_t *input, ft_size_t input_size)` - Encodes binary input as a padded Base64 C string.
- `encoding_base64_decode(const char *input, ft_size_t input_size, ft_size_t *output_size)` - Decodes Base64 text into an allocated byte buffer and writes the decoded length.
- `encoding_base32_encode(const uint8_t *input, ft_size_t input_size)` - Encodes binary input as a padded Base32 C string.
- `encoding_base32_decode(const char *input, ft_size_t input_size, ft_size_t *output_size)` - Decodes Base32 text into an allocated byte buffer and writes the decoded length.
- `encoding_base64url_encode(const uint8_t *input, ft_size_t input_size, ft_bool padded)` - Encodes binary input using the URL-safe Base64 alphabet, optionally retaining padding.
- `encoding_base64url_decode(const char *input, ft_size_t input_size, ft_size_t *output_size)` - Decodes URL-safe Base64 into an allocated byte buffer.

## Hex and Percent Encoding

- `encoding_hex_encode(const uint8_t *input, ft_size_t input_size, ft_bool uppercase)` - Encodes binary input as hexadecimal text.
- `encoding_hex_decode(const char *input, ft_size_t input_size, ft_size_t *output_size)` - Decodes hexadecimal text into an allocated byte buffer.
- `encoding_percent_encode(const uint8_t *input, ft_size_t input_size)` - Percent-encodes arbitrary bytes for URI-style text transport.
- `encoding_percent_decode(const char *input, ft_size_t input_size, ft_size_t *output_size)` - Decodes percent-encoded text into an allocated byte buffer.

## UTF-8

- `encoding_utf8_validate(const char *input, ft_size_t input_size)` - Validates that a byte span is well-formed UTF-8.
- `encoding_utf8_next_codepoint(const char *input, ft_size_t input_size, ft_size_t *index_pointer, uint32_t *code_point_pointer)` - Reads the next UTF-8 code point at `index_pointer`, advances the index, and writes the decoded code point.

## Error State

- `encoding_get_error()` - Returns the module's last error code.
- `encoding_get_error_str()` - Returns readable text for the module's last error.
