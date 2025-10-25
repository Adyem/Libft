#ifndef CLASS_FILE_STREAM_HPP
# define CLASS_FILE_STREAM_HPP

#include "class_file.hpp"
#include <cstddef>

size_t  ft_file_default_buffer_size(void) noexcept;
int     ft_file_stream_copy(ft_file &source_file, ft_file &destination_file, size_t buffer_size) noexcept;

#endif
