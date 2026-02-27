#include <cstddef>
#include "basic.hpp"
#include "utf8.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

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

int ft_utf8_next_grapheme(const char *string, ft_size_t string_length,
        ft_size_t *index_pointer, ft_size_t *grapheme_length_pointer)
{
    if (!string || !index_pointer || !grapheme_length_pointer)
        return (FT_FAILURE);
    if (*index_pointer >= string_length)
        return (FT_FAILURE);
    ft_size_t local_index = *index_pointer;
    uint32_t code_point = 0;
    ft_size_t sequence_length = 0;
    if (ft_utf8_next(string, string_length, &local_index, &code_point, &sequence_length) != FT_SUCCESS)
        return (FT_FAILURE);
    ft_size_t grapheme_end = local_index;
    while (true)
    {
        if (grapheme_end >= string_length)
            break ;
        uint32_t lookahead_code_point = 0;
        ft_size_t lookahead_length = 0;
        ft_size_t lookahead_index = grapheme_end;
        if (ft_utf8_next(string, string_length, &lookahead_index,
                &lookahead_code_point, &lookahead_length) != FT_SUCCESS)
            break ;
        if (!ft_utf8_is_combining_code_point(lookahead_code_point))
            break ;
        grapheme_end = lookahead_index;
    }
    *grapheme_length_pointer = grapheme_end - *index_pointer;
    *index_pointer = grapheme_end;
    return (FT_SUCCESS);
}

int ft_utf8_duplicate_grapheme(const char *string, ft_size_t string_length,
        ft_size_t *index_pointer, char **grapheme_pointer)
{
    if (!string || !index_pointer || !grapheme_pointer)
        return (FT_FAILURE);
    *grapheme_pointer = ft_nullptr;
    ft_size_t grapheme_length = 0;
    ft_size_t start_index = *index_pointer;
    if (ft_utf8_next_grapheme(string, string_length, index_pointer, &grapheme_length) != FT_SUCCESS)
        return (FT_FAILURE);
    char *allocated_grapheme = static_cast<char *>(cma_malloc(grapheme_length + 1));
    if (!allocated_grapheme)
        return (FT_FAILURE);
    ft_size_t copy_index = 0;
    while (copy_index < grapheme_length)
    {
        allocated_grapheme[copy_index] = string[start_index + copy_index];
        ++copy_index;
    }
    allocated_grapheme[grapheme_length] = '\0';
    *grapheme_pointer = allocated_grapheme;
    return (FT_SUCCESS);
}
