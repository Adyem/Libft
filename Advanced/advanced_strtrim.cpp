#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
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

char    *adv_strtrim(const char *input_string, const char *set)
{
    int error_code;

    if (!input_string || !set)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    size_t start = 0;
    size_t end = ft_strlen_size_t(input_string);
    error_code = ft_global_error_stack_drop_last_error();
    while (input_string[start] && is_in_set(input_string[start], set))
        ++start;
    while (end > start && is_in_set(input_string[end - 1], set))
        --end;
    size_t length = end - start;
    char *trimmed = static_cast<char *>(cma_malloc(length + 1));
    error_code = ft_global_error_stack_drop_last_error();
    if (!trimmed)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    size_t index = 0;
    while (index < length)
    {
        trimmed[index] = input_string[start + index];
        ++index;
    }
    trimmed[index] = '\0';
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (trimmed);
}
