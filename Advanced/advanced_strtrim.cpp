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

static bool is_in_set(char character, const char *set)
{
    if (set == ft_nullptr)
        return (false);
    ft_size_t index = 0;
    while (set[index] != '\0')
    {
        if (set[index] == character)
            return (true);
        index += 1;
    }
    return (false);
}

char *adv_strtrim(const char *input_string, const char *set)
{
    if (input_string == ft_nullptr || set == ft_nullptr)
        return (ft_nullptr);
    ft_size_t length = safe_strlen(input_string);
    ft_size_t start = 0;
    while (start < length && is_in_set(input_string[start], set))
        start += 1;
    ft_size_t end = length;
    while (end > start && is_in_set(input_string[end - 1], set))
        end -= 1;
    ft_size_t result_length = end - start;
    char *result = static_cast<char *>(cma_malloc(result_length + 1));
    if (result == ft_nullptr)
        return (ft_nullptr);
    ft_size_t index = 0;
    while (index < result_length)
    {
        result[index] = input_string[start + index];
        ++index;
    }
    result[result_length] = '\0';
    return (result);
}
