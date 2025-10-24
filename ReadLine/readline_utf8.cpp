#include "readline_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Libft/libft_utf8.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstddef>
#include <cstdint>

static int rl_utf8_code_point_width(uint32_t code_point)
{
    if (ft_utf8_is_combining_code_point(code_point))
        return (0);
    if (code_point == 0)
        return (0);
    if (code_point < 0x20)
        return (0);
    if (code_point >= 0x7F && code_point < 0xA0)
        return (0);
    if (code_point >= 0x1100 && code_point <= 0x115F)
        return (2);
    if (code_point == 0x2329 || code_point == 0x232A)
        return (2);
    if (code_point >= 0x2E80 && code_point <= 0xA4CF)
        return (2);
    if (code_point >= 0xAC00 && code_point <= 0xD7A3)
        return (2);
    if (code_point >= 0xF900 && code_point <= 0xFAFF)
        return (2);
    if (code_point >= 0xFE10 && code_point <= 0xFE19)
        return (2);
    if (code_point >= 0xFE30 && code_point <= 0xFE6F)
        return (2);
    if (code_point >= 0xFF00 && code_point <= 0xFF60)
        return (2);
    if (code_point >= 0xFFE0 && code_point <= 0xFFE6)
        return (2);
    if (code_point >= 0x1F300 && code_point <= 0x1F64F)
        return (2);
    if (code_point >= 0x1F900 && code_point <= 0x1F9FF)
        return (2);
    if (code_point >= 0x20000 && code_point <= 0x3FFFD)
        return (2);
    return (1);
}

static int rl_utf8_measure_grapheme_width(const char *buffer, size_t grapheme_length, int *display_width)
{
    size_t offset;
    int total_width;

    if (display_width == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    offset = 0;
    total_width = 0;
    while (offset < grapheme_length)
    {
        size_t decode_index;
        uint32_t code_point;
        size_t sequence_length;

        decode_index = offset;
        code_point = 0;
        sequence_length = 0;
        if (ft_utf8_next(buffer, grapheme_length, &decode_index,
                &code_point, &sequence_length) != FT_SUCCESS)
        {
            total_width = static_cast<int>(grapheme_length);
            ft_errno = ER_SUCCESS;
            *display_width = total_width;
            return (0);
        }
        total_width += rl_utf8_code_point_width(code_point);
        offset = decode_index;
    }
    *display_width = total_width;
    ft_errno = ER_SUCCESS;
    return (0);
}

static int rl_utf8_extract_grapheme(const char *buffer, size_t buffer_length,
        size_t start_index, size_t *end_index, size_t *byte_length,
        int *display_width)
{
    size_t local_index;
    size_t grapheme_length;
    int width;

    if (end_index == ft_nullptr || byte_length == ft_nullptr
        || display_width == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (start_index >= buffer_length)
    {
        *end_index = start_index;
        *byte_length = 0;
        *display_width = 0;
        ft_errno = ER_SUCCESS;
        return (1);
    }
    local_index = start_index;
    grapheme_length = 0;
    width = 0;
    if (ft_utf8_next_grapheme(buffer, buffer_length, &local_index,
            &grapheme_length) != FT_SUCCESS)
    {
        local_index = start_index + 1;
        grapheme_length = 1;
        width = 1;
        ft_errno = ER_SUCCESS;
    }
    else
    {
        if (rl_utf8_measure_grapheme_width(buffer + start_index,
                grapheme_length, &width) != 0)
            width = static_cast<int>(grapheme_length);
    }
    *end_index = local_index;
    *byte_length = grapheme_length;
    *display_width = width;
    ft_errno = ER_SUCCESS;
    return (0);
}

static int rl_utf8_compute_columns_range_internal(const char *string,
        size_t buffer_length, size_t start_index, size_t end_index,
        int *columns)
{
    size_t index;
    int total_columns;

    if (columns == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (start_index > buffer_length)
        start_index = buffer_length;
    if (end_index > buffer_length)
        end_index = buffer_length;
    index = start_index;
    total_columns = 0;
    while (index < end_index)
    {
        size_t next_index;
        size_t grapheme_length;
        int display_width;

        next_index = index;
        grapheme_length = 0;
        display_width = 0;
        if (rl_utf8_extract_grapheme(string, buffer_length, index,
                &next_index, &grapheme_length, &display_width) == 1)
            break ;
        if (next_index > end_index)
        {
            size_t remaining_bytes;

            remaining_bytes = end_index - index;
            total_columns += static_cast<int>(remaining_bytes);
            index = end_index;
        }
        else
        {
            total_columns += display_width;
            index = next_index;
        }
    }
    *columns = total_columns;
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_utf8_compute_columns(const char *string, int *columns)
{
    size_t string_length;

    if (string == ft_nullptr || columns == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    string_length = ft_strlen_size_t(string);
    if (rl_utf8_compute_columns_range_internal(string, string_length,
            0, string_length, columns) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_utf8_find_previous_grapheme(const char *buffer, int cursor_pos,
        int *start_byte, int *end_byte, int *display_width)
{
    size_t buffer_length;
    size_t index;

    if (buffer == ft_nullptr || start_byte == ft_nullptr
        || end_byte == ft_nullptr || display_width == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (cursor_pos <= 0)
    {
        ft_errno = ER_SUCCESS;
        return (1);
    }
    buffer_length = ft_strlen_size_t(buffer);
    if (static_cast<size_t>(cursor_pos) > buffer_length)
        cursor_pos = static_cast<int>(buffer_length);
    index = 0;
    while (index < static_cast<size_t>(cursor_pos))
    {
        size_t next_index;
        size_t grapheme_length;
        int width;

        if (rl_utf8_extract_grapheme(buffer, buffer_length, index,
                &next_index, &grapheme_length, &width) == 1)
            break ;
        if (static_cast<int>(next_index) >= cursor_pos)
        {
            if (static_cast<int>(next_index) > cursor_pos)
            {
                *start_byte = cursor_pos - 1;
                *end_byte = cursor_pos;
                *display_width = 1;
                ft_errno = ER_SUCCESS;
                return (0);
            }
            *start_byte = static_cast<int>(index);
            *end_byte = static_cast<int>(next_index);
            *display_width = width;
            ft_errno = ER_SUCCESS;
            return (0);
        }
        index = next_index;
    }
    ft_errno = ER_SUCCESS;
    return (1);
}

int rl_utf8_find_next_grapheme(const char *buffer, int cursor_pos,
        int *start_byte, int *end_byte, int *display_width)
{
    size_t buffer_length;
    size_t index;

    if (buffer == ft_nullptr || start_byte == ft_nullptr
        || end_byte == ft_nullptr || display_width == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (cursor_pos < 0)
        cursor_pos = 0;
    buffer_length = ft_strlen_size_t(buffer);
    if (static_cast<size_t>(cursor_pos) >= buffer_length)
    {
        ft_errno = ER_SUCCESS;
        return (1);
    }
    index = 0;
    while (index < buffer_length)
    {
        size_t next_index;
        size_t grapheme_length;
        int width;

        if (rl_utf8_extract_grapheme(buffer, buffer_length, index,
                &next_index, &grapheme_length, &width) == 1)
            break ;
        if (static_cast<int>(index) >= cursor_pos)
        {
            *start_byte = static_cast<int>(index);
            *end_byte = static_cast<int>(next_index);
            *display_width = width;
            ft_errno = ER_SUCCESS;
            return (0);
        }
        index = next_index;
    }
    ft_errno = ER_SUCCESS;
    return (1);
}

int rl_update_display_metrics(readline_state_t *state)
{
    size_t buffer_length;
    int total_columns;
    int prefix_columns;

    if (state == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (state->buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    buffer_length = ft_strlen_size_t(state->buffer);
    if (state->pos < 0)
        state->pos = 0;
    if (static_cast<size_t>(state->pos) > buffer_length)
        state->pos = static_cast<int>(buffer_length);
    if (rl_utf8_compute_columns_range_internal(state->buffer, buffer_length,
            0, buffer_length, &total_columns) != 0)
        total_columns = static_cast<int>(buffer_length);
    if (rl_utf8_compute_columns_range_internal(state->buffer, buffer_length,
            0, static_cast<size_t>(state->pos), &prefix_columns) != 0)
        prefix_columns = state->pos;
    state->display_pos = prefix_columns;
    state->prev_display_columns = total_columns;
    state->prev_buffer_length = static_cast<int>(buffer_length);
    ft_errno = ER_SUCCESS;
    return (0);
}

