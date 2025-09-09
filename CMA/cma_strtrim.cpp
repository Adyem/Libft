#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

static bool is_in_set(char character, const char *set)
{
    while (set && *set)
    {
        if (*set == character)
            return (true);
        ++set;
    }
    return (false);
}

char    *cma_strtrim(const char *input_string, const char *set)
{
    if (!input_string || !set)
        return (ft_nullptr);
    size_t start = 0;
    size_t end = ft_strlen_size_t(input_string);
    while (input_string[start] && is_in_set(input_string[start], set))
        ++start;
    while (end > start && is_in_set(input_string[end - 1], set))
        --end;
    size_t length = end - start;
    char *trimmed = static_cast<char *>(cma_malloc(length + 1));
    if (!trimmed)
        return (ft_nullptr);
    size_t index = 0;
    while (index < length)
    {
        trimmed[index] = input_string[start + index];
        ++index;
    }
    trimmed[index] = '\0';
    return (trimmed);
}
