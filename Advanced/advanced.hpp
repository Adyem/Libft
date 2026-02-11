#ifndef ADVANCED_HPP
# define ADVANCED_HPP

#include <cstddef>
#include <cstdarg>
#include "../Basic/basic.hpp"

class ft_string;

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
