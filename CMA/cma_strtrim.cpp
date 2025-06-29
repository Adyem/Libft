#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

static bool is_in_set(char c, const char *set)
{
    while (set && *set)
    {
        if (*set == c)
            return (true);
        ++set;
    }
    return (false);
}

char    *cma_strtrim(const char *s1, const char *set)
{
    if (!s1 || !set)
        return (ft_nullptr);
    size_t start = 0;
    size_t end = ft_strlen_size_t(s1);
    while (s1[start] && is_in_set(s1[start], set))
        ++start;
    while (end > start && is_in_set(s1[end - 1], set))
        --end;
    size_t len = end - start;
    char *trimmed = static_cast<char *>(cma_malloc(len + 1));
    if (!trimmed)
        return (ft_nullptr);
    size_t i = 0;
    while (i < len)
    {
        trimmed[i] = s1[start + i];
        ++i;
    }
    trimmed[i] = '\0';
    return (trimmed);
}
