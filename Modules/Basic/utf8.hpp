#ifndef LIBFT_UTF8_HPP
# define LIBFT_UTF8_HPP

#include <stddef.h>
#include <stdint.h>

int32_t     ft_utf8_next(const char *string, ft_size_t string_length,
                ft_size_t *index_pointer, uint32_t *code_point_pointer,
                ft_size_t *sequence_length_pointer);
int32_t     ft_utf8_count(const char *string, ft_size_t *code_point_count_pointer);
int32_t     ft_utf8_encode(uint32_t code_point, char *buffer, ft_size_t buffer_size,
                ft_size_t *encoded_length_pointer);

#endif
