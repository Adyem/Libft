#include "readline_internal.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/utf8.hpp"
#include "../Advanced/advanced.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstddef>
#include <cstdint>

static int32_t rl_utf8_code_point_width(uint32_t code_point)
{
    if (ft_utf8_is_combining_code_point(code_point))
        return (FT_ERR_SUCCESS);
    if (code_point == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
    if (code_point < 0x20)
        return (FT_ERR_SUCCESS);
    if (code_point >= 0x7F && code_point < 0xA0)
        return (FT_ERR_SUCCESS);
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

static int32_t rl_utf8_measure_grapheme_width(const char *buffer, ft_size_t grapheme_length, int32_t *display_width)
{
    ft_size_t offset;
    int32_t total_width;

    if (display_width == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    offset = 0;
    total_width = 0;
    while (offset < grapheme_length)
    {
        ft_size_t decode_index;
        uint32_t code_point;
        ft_size_t sequence_length;

        decode_index = offset;
        code_point = 0;
        sequence_length = 0;
        if (ft_utf8_next(buffer, grapheme_length, &decode_index,
                &code_point, &sequence_length) != FT_ERR_SUCCESS)
        {
            total_width = static_cast<int32_t>(grapheme_length);
            *display_width = total_width;
            return (FT_ERR_SUCCESS);
        }
        total_width += rl_utf8_code_point_width(code_point);
        offset = decode_index;
    }
    *display_width = total_width;
    return (FT_ERR_SUCCESS);
}

static int32_t rl_utf8_extract_grapheme(const char *buffer, ft_size_t buffer_length,
        ft_size_t start_index, ft_size_t *end_index, ft_size_t *byte_length,
        int32_t *display_width)
{
    ft_size_t local_index;
    ft_size_t grapheme_length;
    int32_t width;

    if (end_index == ft_nullptr || byte_length == ft_nullptr
        || display_width == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    if (buffer == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    if (start_index >= buffer_length)
    {
        *end_index = start_index;
        *byte_length = 0;
        *display_width = 0;
        return (FT_ERR_NOT_FOUND);
    }
    local_index = start_index;
    grapheme_length = 0;
    width = 0;
    if (ft_utf8_next_grapheme(buffer, buffer_length, &local_index,
            &grapheme_length) != FT_ERR_SUCCESS)
    {
        local_index = start_index + 1;
        grapheme_length = 1;
        width = 1;
    }
    else
    {
        if (rl_utf8_measure_grapheme_width(buffer + start_index,
                grapheme_length, &width) != FT_ERR_SUCCESS)
            width = static_cast<int32_t>(grapheme_length);
    }
    *end_index = local_index;
    *byte_length = grapheme_length;
    *display_width = width;
    return (FT_ERR_SUCCESS);
}

static int32_t rl_utf8_compute_columns_range_internal(const char *string,
        ft_size_t buffer_length, ft_size_t start_index, ft_size_t end_index,
        int32_t *columns)
{
    ft_size_t index;
    int32_t total_columns;

    if (columns == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    if (start_index > buffer_length)
        start_index = buffer_length;
    if (end_index > buffer_length)
        end_index = buffer_length;
    index = start_index;
    total_columns = 0;
    while (index < end_index)
    {
        ft_size_t next_index;
        ft_size_t grapheme_length;
        int32_t display_width;

        next_index = index;
        grapheme_length = 0;
        display_width = 0;
        if (rl_utf8_extract_grapheme(string, buffer_length, index,
                &next_index, &grapheme_length, &display_width) == FT_ERR_NOT_FOUND)
            break ;
        if (next_index > end_index)
        {
            ft_size_t remaining_bytes;

            remaining_bytes = end_index - index;
            total_columns += static_cast<int32_t>(remaining_bytes);
            index = end_index;
        }
        else
        {
            total_columns += display_width;
            index = next_index;
        }
    }
    *columns = total_columns;
    return (FT_ERR_SUCCESS);
}

int32_t rl_utf8_compute_columns(const char *string, int32_t *columns)
{
    ft_size_t string_length;

    if (string == ft_nullptr || columns == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    string_length = ft_strlen_size_t(string);
    if (rl_utf8_compute_columns_range_internal(string, string_length,
            0, string_length, columns) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

int32_t rl_utf8_find_previous_grapheme(const char *buffer, int32_t cursor_pos,
        int32_t *start_byte, int32_t *end_byte, int32_t *display_width)
{
    ft_size_t buffer_length;
    ft_size_t index;

    if (buffer == ft_nullptr || start_byte == ft_nullptr
        || end_byte == ft_nullptr || display_width == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    if (cursor_pos <= 0)
    {
        return (FT_ERR_NOT_FOUND);
    }
    buffer_length = ft_strlen_size_t(buffer);
    if (static_cast<ft_size_t>(cursor_pos) > buffer_length)
        cursor_pos = static_cast<int32_t>(buffer_length);
    index = 0;
    while (index < static_cast<ft_size_t>(cursor_pos))
    {
        ft_size_t next_index;
        ft_size_t grapheme_length;
        int32_t width;

        if (rl_utf8_extract_grapheme(buffer, buffer_length, index,
                &next_index, &grapheme_length, &width) == FT_ERR_NOT_FOUND)
            break ;
        if (static_cast<int32_t>(next_index) >= cursor_pos)
        {
            if (static_cast<int32_t>(next_index) > cursor_pos)
            {
                *start_byte = cursor_pos - 1;
                *end_byte = cursor_pos;
                *display_width = 1;
                return (FT_ERR_SUCCESS);
            }
            *start_byte = static_cast<int32_t>(index);
            *end_byte = static_cast<int32_t>(next_index);
            *display_width = width;
            return (FT_ERR_SUCCESS);
        }
        index = next_index;
    }
    return (FT_ERR_NOT_FOUND);
}

int32_t rl_utf8_find_next_grapheme(const char *buffer, int32_t cursor_pos,
        int32_t *start_byte, int32_t *end_byte, int32_t *display_width)
{
    ft_size_t buffer_length;
    ft_size_t index;

    if (buffer == ft_nullptr || start_byte == ft_nullptr
        || end_byte == ft_nullptr || display_width == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    if (cursor_pos < 0)
        cursor_pos = 0;
    buffer_length = ft_strlen_size_t(buffer);
    if (static_cast<ft_size_t>(cursor_pos) >= buffer_length)
    {
        return (FT_ERR_NOT_FOUND);
    }
    index = 0;
    while (index < buffer_length)
    {
        ft_size_t next_index;
        ft_size_t grapheme_length;
        int32_t width;

        if (rl_utf8_extract_grapheme(buffer, buffer_length, index,
                &next_index, &grapheme_length, &width) == FT_ERR_NOT_FOUND)
            break ;
        if (static_cast<int32_t>(index) >= cursor_pos)
        {
            *start_byte = static_cast<int32_t>(index);
            *end_byte = static_cast<int32_t>(next_index);
            *display_width = width;
            return (FT_ERR_SUCCESS);
        }
        index = next_index;
    }
    return (FT_ERR_NOT_FOUND);
}

int32_t rl_update_display_metrics(readline_state_t *state)
{
    ft_size_t buffer_length;
    int32_t total_columns;
    int32_t prefix_columns;

    if (state == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    if (state->buffer == ft_nullptr)
    {
        return (FT_ERR_INTERNAL);
    }
    buffer_length = ft_strlen_size_t(state->buffer);
    if (state->position < 0)
        state->position = 0;
    if (static_cast<ft_size_t>(state->position) > buffer_length)
        state->position = static_cast<int32_t>(buffer_length);
    if (rl_utf8_compute_columns_range_internal(state->buffer, buffer_length,
            0, buffer_length, &total_columns) != FT_ERR_SUCCESS)
        total_columns = static_cast<int32_t>(buffer_length);
    if (rl_utf8_compute_columns_range_internal(state->buffer, buffer_length,
            0, static_cast<ft_size_t>(state->position), &prefix_columns) != FT_ERR_SUCCESS)
        prefix_columns = state->position;
    state->display_pos = prefix_columns;
    state->prev_display_columns = total_columns;
    state->prev_buffer_length = static_cast<int32_t>(buffer_length);
    return (FT_ERR_SUCCESS);
}
