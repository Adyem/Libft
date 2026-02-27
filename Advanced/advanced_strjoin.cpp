#include <cstddef>
#include "advanced.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

static ft_size_t safe_strlen(const char *string)
{
    if (string == ft_nullptr)
        return (0);
    ft_size_t length = 0;
    while (string[length] != '\0')
    {
        if (length >= FT_SYSTEM_SIZE_MAX)
            return (FT_SYSTEM_SIZE_MAX);
        length += 1;
    }
    return (length);
}

char *adv_strjoin(char const *string_1, char const *string_2)
{
    if (string_1 == ft_nullptr && string_2 == ft_nullptr)
        return (ft_nullptr);
    ft_size_t len1 = safe_strlen(string_1);
    ft_size_t len2 = safe_strlen(string_2);
    if (len1 >= FT_SYSTEM_SIZE_MAX || len2 >= FT_SYSTEM_SIZE_MAX)
        return (ft_nullptr);
    if (len1 + len2 >= FT_SYSTEM_SIZE_MAX)
        return (ft_nullptr);
    ft_size_t total_length = len1 + len2;
    char *result = static_cast<char *>(cma_malloc(total_length + 1));
    if (result == ft_nullptr)
        return (ft_nullptr);
    ft_size_t write_index = 0;
    if (string_1)
    {
        ft_size_t index = 0;
        while (index < len1)
        {
            result[write_index++] = string_1[index];
            ++index;
        }
    }
    if (string_2)
    {
        ft_size_t index = 0;
        while (index < len2)
        {
            result[write_index++] = string_2[index];
            ++index;
        }
    }
    result[write_index] = '\0';
    return (result);
}
