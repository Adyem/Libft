#ifndef ENCODING_HPP
# define ENCODING_HPP

#include "../Errno/errno.hpp"
#include <cstdint>

char        *encoding_base64_encode(const uint8_t *input, ft_size_t input_size);
uint8_t     *encoding_base64_decode(const char *input, ft_size_t input_size,
                ft_size_t *output_size);
char        *encoding_base32_encode(const uint8_t *input, ft_size_t input_size);
uint8_t     *encoding_base32_decode(const char *input, ft_size_t input_size,
                ft_size_t *output_size);
char        *encoding_base64url_encode(const uint8_t *input,
                ft_size_t input_size, ft_bool padded);
uint8_t     *encoding_base64url_decode(const char *input, ft_size_t input_size,
                ft_size_t *output_size);
char        *encoding_hex_encode(const uint8_t *input, ft_size_t input_size,
                ft_bool uppercase);
uint8_t     *encoding_hex_decode(const char *input, ft_size_t input_size,
                ft_size_t *output_size);
char        *encoding_percent_encode(const uint8_t *input, ft_size_t input_size);
uint8_t     *encoding_percent_decode(const char *input, ft_size_t input_size,
                ft_size_t *output_size);
int32_t     encoding_utf8_validate(const char *input, ft_size_t input_size);
int32_t     encoding_utf8_next_codepoint(const char *input, ft_size_t input_size,
                ft_size_t *index_pointer, uint32_t *code_point_pointer);
int32_t     encoding_get_error(void) noexcept;
const char  *encoding_get_error_str(void) noexcept;

#endif
