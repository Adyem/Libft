#ifndef ADVANCED_HPP
# define ADVANCED_HPP

#include <cstddef>
#include <cstdarg>
#include "../Basic/basic.hpp"

class ft_string;

typedef uint32_t (*ft_utf8_case_hook)(uint32_t code_point);

int64_t         ft_hash_string31(const char *string);
int64_t         ft_hash_string31(const ft_string &string);
char            *ft_span_dup(const char *buffer, ft_size_t length);
char            *ft_strmapi(const char *string, char (*function)(uint32_t, char));
uint32_t        ft_utf8_case_ascii_lower(uint32_t code_point);
uint32_t        ft_utf8_case_ascii_upper(uint32_t code_point);
int32_t         ft_utf8_is_combining_code_point(uint32_t code_point);
int32_t         ft_utf8_transform(const char *input, ft_size_t input_length,
                    char *output_buffer, ft_size_t output_buffer_size,
                    ft_utf8_case_hook case_hook);
int32_t         ft_utf8_transform_alloc(const char *input, char **output_pointer,
                    ft_utf8_case_hook case_hook);
int32_t         ft_utf8_next_grapheme(const char *string, ft_size_t string_length,
                    ft_size_t *index_pointer, ft_size_t *grapheme_length_pointer);
int32_t         ft_utf8_duplicate_grapheme(const char *string, ft_size_t string_length,
                    ft_size_t *index_pointer, char **grapheme_pointer);

ft_string *adv_to_string(int64_t number);
ft_string *adv_to_string(uint64_t number);
ft_string *adv_to_string(double number);
ft_string *adv_to_string(int32_t number);
ft_string *adv_to_string(uint32_t number);
ft_string *adv_to_string(float number);

ft_string *adv_utf16_to_utf8(const char16_t *input, ft_size_t input_length);
ft_string *adv_utf32_to_utf8(const char32_t *input, ft_size_t input_length);
ft_string *adv_locale_casefold(const char *input, const char *locale_name);
ft_string *adv_span_to_string(const char *buffer, ft_size_t length);

int32_t *adv_atoi(const char *string);
char *adv_itoa(int32_t number);
char *adv_strdup(const char *string);
char *adv_strndup(const char *string, ft_size_t maximum_length);
char *adv_strjoin(char const *string_1, char const *string_2);
char *adv_strjoin_multiple(int32_t count, ...);
char *adv_substr(const char *source, uint32_t start, ft_size_t length);
char *adv_strtrim(const char *input_string, const char *set);
char **adv_split(char const *string, char delimiter);
char *adv_itoa_base(int32_t number, int32_t base);
void *adv_memdup(const void *source, ft_size_t size);
void *adv_calloc(ft_size_t count, ft_size_t size);

#endif
