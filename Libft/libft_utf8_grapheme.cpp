#include "libft.hpp"
#include "libft_utf8.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

static int ft_utf8_code_point_in_range(uint32_t code_point, uint32_t start_value,
        uint32_t end_value)
{
    if (code_point >= start_value && code_point <= end_value)
        return (1);
    return (0);
}

int ft_utf8_is_combining_code_point(uint32_t code_point)
{
    if (ft_utf8_code_point_in_range(code_point, 0x0300, 0x036F))
        return (1);
    if (ft_utf8_code_point_in_range(code_point, 0x1AB0, 0x1AFF))
        return (1);
    if (ft_utf8_code_point_in_range(code_point, 0x1DC0, 0x1DFF))
        return (1);
    if (ft_utf8_code_point_in_range(code_point, 0x20D0, 0x20FF))
        return (1);
    if (ft_utf8_code_point_in_range(code_point, 0xFE20, 0xFE2F))
        return (1);
    return (0);
}

int ft_utf8_next_grapheme(const char *string, size_t string_length,
        size_t *index_pointer, size_t *grapheme_length_pointer)
{
    size_t current_index;
    size_t grapheme_end_index;
    uint32_t code_point_value;
    size_t sequence_length;
    int error_code;

    if (string == ft_nullptr || index_pointer == ft_nullptr
        || grapheme_length_pointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_FAILURE);
    }
    if (*index_pointer >= string_length)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_FAILURE);
    }
    current_index = *index_pointer;
    code_point_value = 0;
    sequence_length = 0;
    if (ft_utf8_next(string, string_length, &current_index,
            &code_point_value, &sequence_length) != FT_SUCCESS)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    grapheme_end_index = current_index;
    while (1)
    {
        size_t lookahead_index;
        uint32_t lookahead_code_point;
        size_t lookahead_length;

        lookahead_index = grapheme_end_index;
        if (lookahead_index >= string_length)
            break ;
        lookahead_code_point = 0;
        lookahead_length = 0;
        if (ft_utf8_next(string, string_length, &lookahead_index,
                &lookahead_code_point, &lookahead_length) != FT_SUCCESS)
        {
            error_code = ft_global_error_stack_pop_newest();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_INVALID_ARGUMENT;
            ft_global_error_stack_push(error_code);
            return (FT_FAILURE);
        }
        error_code = ft_global_error_stack_pop_newest();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (FT_FAILURE);
        }
        if (!ft_utf8_is_combining_code_point(lookahead_code_point))
            break ;
        grapheme_end_index = lookahead_index;
    }
    *grapheme_length_pointer = grapheme_end_index - *index_pointer;
    *index_pointer = grapheme_end_index;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}

int ft_utf8_duplicate_grapheme(const char *string, size_t string_length,
        size_t *index_pointer, char **grapheme_pointer)
{
    size_t start_index;
    size_t local_index;
    size_t grapheme_length;
    char *allocated_grapheme;
    size_t copy_index;
    int error_code;

    if (string == ft_nullptr || index_pointer == ft_nullptr
        || grapheme_pointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_FAILURE);
    }
    *grapheme_pointer = ft_nullptr;
    start_index = *index_pointer;
    local_index = *index_pointer;
    grapheme_length = 0;
    if (ft_utf8_next_grapheme(string, string_length, &local_index,
            &grapheme_length) != FT_SUCCESS)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    allocated_grapheme = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(grapheme_length + 1)));
    if (allocated_grapheme == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (FT_FAILURE);
    }
    copy_index = 0;
    while (copy_index < grapheme_length)
    {
        allocated_grapheme[copy_index] = string[start_index + copy_index];
        copy_index++;
    }
    allocated_grapheme[grapheme_length] = '\0';
    *index_pointer = local_index;
    *grapheme_pointer = allocated_grapheme;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}
