#ifndef URI_INTERNAL_HPP
# define URI_INTERNAL_HPP

#include "uri.hpp"
#include "../Basic/class_nullptr.hpp"

int32_t     uri_set_error(int32_t error_code) noexcept;
ft_bool     uri_mul_overflows(ft_size_t left, ft_size_t right) noexcept;
ft_bool     uri_is_alpha(char character) noexcept;
ft_bool     uri_is_digit(char character) noexcept;
ft_bool     uri_is_scheme_character(char character) noexcept;
ft_bool     uri_is_unreserved(uint8_t value) noexcept;
char        uri_lower_character(char character) noexcept;
char        uri_hex_character(uint8_t value) noexcept;
int32_t     uri_hex_value(char character) noexcept;
char        *uri_duplicate_span(const char *input, ft_size_t start,
                ft_size_t end) noexcept;
int32_t     uri_assign_component(char **destination, const char *input,
                ft_size_t start, ft_size_t end) noexcept;
void        uri_lower_string(char *string) noexcept;
ft_size_t   uri_component_length(const char *component) noexcept;
void        uri_append_text(char *output, ft_size_t *output_index,
                const char *text) noexcept;
void        uri_delete_components(uri_components *components) noexcept;

#endif
