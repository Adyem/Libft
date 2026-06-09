#ifndef REGEX_HPP
# define REGEX_HPP

#include "../Basic/basic.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"

typedef struct regex_capture_group
{
    ft_bool     matched;
    ft_size_t   start_index;
    ft_size_t   length;
} regex_capture_group;

int32_t     regex_validate_pattern(const char *pattern) noexcept;
int32_t     regex_is_match(const char *pattern, const char *text,
                ft_bool *is_match) noexcept;
int32_t     regex_search(const char *pattern, const char *text,
                ft_bool *is_found) noexcept;
int32_t     regex_replace_all(const char *pattern, const char *replacement,
                const char *text, ft_string *output) noexcept;
int32_t     regex_capture_count(const char *pattern,
                ft_size_t *capture_count) noexcept;
int32_t     regex_capture_groups(const char *pattern, const char *text,
                regex_capture_group *captures, ft_size_t capture_capacity,
                ft_size_t *capture_count) noexcept;

#endif
