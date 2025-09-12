#ifndef YAML_READER_UTILS_HPP
#define YAML_READER_UTILS_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"

size_t      yaml_find_char(const ft_string &string, char character) noexcept;
ft_string   yaml_substr(const ft_string &string, size_t start, size_t length) noexcept;
ft_string   yaml_substr_from(const ft_string &string, size_t start) noexcept;
size_t      yaml_count_indent(const ft_string &line) noexcept;
void        yaml_trim(ft_string &string) noexcept;
void        yaml_split_lines(const ft_string &content, ft_vector<ft_string> &lines) noexcept;

#endif

