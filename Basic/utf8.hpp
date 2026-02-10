#ifndef LIBFT_UTF8_HPP
# define LIBFT_UTF8_HPP

#include <stddef.h>
#include <stdint.h>

typedef uint32_t (*ft_utf8_case_hook)(uint32_t code_point);

int         ft_utf8_next(const char *string, ft_size_t string_length,
                ft_size_t *index_pointer, uint32_t *code_point_pointer,
                ft_size_t *sequence_length_pointer);
int         ft_utf8_count(const char *string, ft_size_t *code_point_count_pointer);
int         ft_utf8_encode(uint32_t code_point, char *buffer, ft_size_t buffer_size,
                ft_size_t *encoded_length_pointer);
int         ft_utf8_transform(const char *input, ft_size_t input_length,
                char *output_buffer, ft_size_t output_buffer_size,
                ft_utf8_case_hook case_hook);
int         ft_utf8_transform_alloc(const char *input, char **output_pointer,
                ft_utf8_case_hook case_hook);
uint32_t    ft_utf8_case_ascii_lower(uint32_t code_point);
uint32_t    ft_utf8_case_ascii_upper(uint32_t code_point);
int         ft_utf8_is_combining_code_point(uint32_t code_point);
int         ft_utf8_next_grapheme(const char *string, ft_size_t string_length,
                ft_size_t *index_pointer, ft_size_t *grapheme_length_pointer);
int         ft_utf8_duplicate_grapheme(const char *string, ft_size_t string_length,
                ft_size_t *index_pointer, char **grapheme_pointer);

#endif
