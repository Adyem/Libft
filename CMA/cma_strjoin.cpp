#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

static char    *allocate_new_string(const char *string_1, const char *string_2)
{
    int        total_length;
    char    *new_string;

    total_length = 0;
    if (string_1)
        total_length += ft_strlen(string_1);
    if (string_2)
        total_length += ft_strlen(string_2);
    new_string = static_cast<char *>(cma_malloc(total_length + 1));
    if (!new_string)
        return (ft_nullptr);
    return (new_string);
}

char    *cma_strjoin(char const *string_1, char const *string_2)
{
    char    *result;
    int        index;

    if (!string_1 && !string_2)
        return (ft_nullptr);
    result = allocate_new_string(string_1, string_2);
    if (!result)
        return (ft_nullptr);
    index = 0;
    if (string_1)
        while (string_1[0])
            result[index++] = *string_1++;
    if (string_2)
        while (string_2[0])
            result[index++] = *string_2++;
    result[index] = '\0';
    return (result);
}
