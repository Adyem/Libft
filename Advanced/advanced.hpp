#ifndef ADVANCED_HPP
# define ADVANCED_HPP

#include <cstddef>
#include <cstdarg>
#include "../Basic/basic.hpp"

class ft_string;

ft_string *adv_to_string(long number);
ft_string *adv_to_string(unsigned long number);
ft_string *adv_to_string(unsigned long long number);
ft_string *adv_to_string(double number);
ft_string *adv_to_string(int number);
ft_string *adv_to_string(unsigned int number);
ft_string *adv_to_string(float number);

ft_string *adv_utf16_to_utf8(const char16_t *input, size_t input_length);
ft_string *adv_utf32_to_utf8(const char32_t *input, size_t input_length);
ft_string *adv_locale_casefold(const char *input, const char *locale_name);
ft_string *adv_span_to_string(const char *buffer, size_t length);

int *adv_atoi(const char *string);
char *adv_strdup(const char *string);
char *adv_strndup(const char *string, size_t maximum_length);
char *adv_strjoin(char const *string_1, char const *string_2);
char *adv_strjoin_multiple(int count, ...);
char *adv_substr(const char *source, unsigned int start, size_t length);
char *adv_strtrim(const char *input_string, const char *set);
char **adv_split(char const *string, char delimiter);
void *adv_memdup(const void *source, size_t size);
void *adv_calloc(ft_size_t count, ft_size_t size);

#endif
