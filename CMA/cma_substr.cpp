#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

char    *cma_substr(const char *s, unsigned int start, size_t len)
{
    size_t  s_len;
    size_t  i;
    char    *sub;

    if (!s)
        return (ft_nullptr);
    s_len = ft_strlen(s);
    if (start >= s_len)
        return (cma_strdup(""));
    if (len > s_len - start)
        len = s_len - start;
    sub = static_cast<char *>(cma_malloc(len + 1));
    if (!sub)
        return (ft_nullptr);
    i = 0;
    while (i < len && s[start + i])
    {
        sub[i] = s[start + i];
        i++;
    }
    sub[i] = '\0';
    return (sub);
}
