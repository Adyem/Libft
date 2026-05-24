#ifndef FILESYSTEM_HPP
# define FILESYSTEM_HPP

#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"

typedef int32_t (*filesystem_walk_callback)(const char *path, ft_bool is_directory,
    void *user_context);

ft_string   *filesystem_normalize_path(const char *path);
ft_string   *filesystem_join_path(const char *path_left, const char *path_right);
ft_string   *filesystem_safe_join_path(const char *root_path,
                const char *relative_path);
ft_string   *filesystem_canonical_path(const char *path);
ft_string   *filesystem_basename(const char *path);
ft_string   *filesystem_dirname(const char *path);
ft_string   *filesystem_extension(const char *path);
ft_string   *filesystem_stem(const char *path);
ft_bool     filesystem_is_absolute(const char *path) noexcept;
ft_bool     filesystem_is_relative(const char *path) noexcept;
ft_bool     filesystem_is_safe_relative_path(const char *path) noexcept;
ft_bool     filesystem_is_inside_root(const char *root_path,
                const char *candidate_path) noexcept;
int32_t     filesystem_validate_inside_root(const char *root_path,
                const char *candidate_path);
ft_bool     filesystem_has_extension(const char *path, const char *extension) noexcept;
int32_t     filesystem_temp_path(const char *prefix, const char *extension,
                ft_string *output);
int32_t     filesystem_atomic_write(const char *path, const void *data,
                ft_size_t size);
int32_t     filesystem_walk_recursive(const char *root_path,
                filesystem_walk_callback callback, void *user_context);

#endif
