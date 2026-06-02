#include "filesystem.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static ft_bool filesystem_glob_is_separator(char character) noexcept
{
    if (character == '/' || character == '\\')
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool filesystem_path_has_wildcards(const char *path) noexcept
{
    ft_size_t index;

    if (path == ft_nullptr)
        return (FT_FALSE);
    index = 0;
    while (path[index] != '\0')
    {
        if (path[index] == '*' || path[index] == '?')
            return (FT_TRUE);
        index++;
    }
    return (FT_FALSE);
}

static ft_bool filesystem_glob_match_recursive(const char *pattern,
    ft_size_t pattern_index, const char *path, ft_size_t path_index) noexcept
{
    ft_size_t star_count;
    ft_size_t next_pattern_index;

    while (pattern[pattern_index] != '\0')
    {
        if (pattern[pattern_index] == '*')
        {
            star_count = 1;
            next_pattern_index = pattern_index + 1;
            while (pattern[next_pattern_index] == '*')
            {
                star_count++;
                next_pattern_index++;
            }
            if (star_count > 1)
            {
                if (pattern[next_pattern_index] == '\0')
                    return (FT_TRUE);
                if (filesystem_glob_is_separator(pattern[next_pattern_index]) == FT_TRUE)
                {
                    while (filesystem_glob_is_separator(pattern[next_pattern_index]) == FT_TRUE)
                        next_pattern_index++;
                    if (filesystem_glob_match_recursive(pattern, next_pattern_index,
                            path, path_index) == FT_TRUE)
                        return (FT_TRUE);
                }
                while (path[path_index] != '\0')
                {
                    if (filesystem_glob_match_recursive(pattern,
                            next_pattern_index, path, path_index) == FT_TRUE)
                        return (FT_TRUE);
                    path_index++;
                }
                return (filesystem_glob_match_recursive(pattern,
                        next_pattern_index, path, path_index));
            }
            while (path[path_index] != '\0'
                && filesystem_glob_is_separator(path[path_index]) == FT_FALSE)
            {
                if (filesystem_glob_match_recursive(pattern,
                        next_pattern_index, path, path_index) == FT_TRUE)
                    return (FT_TRUE);
                path_index++;
            }
            return (filesystem_glob_match_recursive(pattern, next_pattern_index,
                    path, path_index));
        }
        if (pattern[pattern_index] == '?')
        {
            if (path[path_index] == '\0'
                || filesystem_glob_is_separator(path[path_index]) == FT_TRUE)
                return (FT_FALSE);
            pattern_index++;
            path_index++;
            continue ;
        }
        if (filesystem_glob_is_separator(pattern[pattern_index]) == FT_TRUE)
        {
            if (filesystem_glob_is_separator(path[path_index]) == FT_FALSE)
                return (FT_FALSE);
            while (filesystem_glob_is_separator(pattern[pattern_index]) == FT_TRUE)
                pattern_index++;
            while (filesystem_glob_is_separator(path[path_index]) == FT_TRUE)
                path_index++;
            continue ;
        }
        if (pattern[pattern_index] != path[path_index])
            return (FT_FALSE);
        pattern_index++;
        path_index++;
    }
    while (filesystem_glob_is_separator(path[path_index]) == FT_TRUE)
        path_index++;
    if (path[path_index] == '\0')
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool filesystem_match_glob(const char *pattern, const char *path) noexcept
{
    if (pattern == ft_nullptr || path == ft_nullptr)
        return (FT_FALSE);
    return (filesystem_glob_match_recursive(pattern, 0, path, 0));
}
