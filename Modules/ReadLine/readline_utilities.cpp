#include <stdlib.h>
#include <unistd.h>
#include "../Advanced/advanced.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "readline_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t rl_resize_buffer(char **buffer_pointer, int32_t *current_size_pointer, int32_t new_size)
{
    char *old_buffer;
    int32_t current_size;
    ft_size_t copy_size;
    char *new_buffer;

    if (buffer_pointer == ft_nullptr || current_size_pointer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (new_size <= 0)
        return (FT_ERR_INVALID_ARGUMENT);
    old_buffer = *buffer_pointer;
    current_size = *current_size_pointer;
    copy_size = 0;
    if (current_size > 0)
    {
        copy_size = static_cast<ft_size_t>(current_size);
        if (current_size > new_size)
            copy_size = static_cast<ft_size_t>(new_size);
    }
    new_buffer = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(new_size)));
    if (new_buffer == ft_nullptr)
    {
        su_write(2, "Allocation error\n", 17);
        return (FT_ERR_NO_MEMORY);
    }
    if (copy_size > 0 && old_buffer != ft_nullptr)
        ft_memcpy(new_buffer, old_buffer, copy_size);
    if (old_buffer != ft_nullptr)
        cma_free(old_buffer);
    if (copy_size < static_cast<ft_size_t>(new_size))
        ft_bzero(new_buffer + copy_size, static_cast<ft_size_t>(new_size) - copy_size);
    *buffer_pointer = new_buffer;
    *current_size_pointer = new_size;
    return (FT_ERR_SUCCESS);
}

int32_t rl_clear_line(const char *prompt, const char *buffer)
{
    int32_t prompt_length;
    int32_t buffer_length;
    int32_t total_length;

    if (prompt == ft_nullptr || buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    int32_t prompt_columns_error = rl_utf8_compute_columns(prompt, &prompt_length);
    if (prompt_columns_error != FT_ERR_SUCCESS)
        return (prompt_columns_error);
    int32_t buffer_columns_error = rl_utf8_compute_columns(buffer, &buffer_length);
    if (buffer_columns_error != FT_ERR_SUCCESS)
        return (buffer_columns_error);
    total_length = prompt_length + buffer_length;
    pf_printf("\r");
    int32_t term_width;
    int32_t terminal_width_error;

    terminal_width_error = rl_get_terminal_width(&term_width);
    if (terminal_width_error != FT_ERR_SUCCESS || term_width <= 0)
        term_width = 1;
    int32_t line_count = (total_length / term_width) + 1;
    int32_t index = 0;
    while (index < line_count)
    {
        pf_printf("\033[2K");
        if (index < line_count - 1)
            pf_printf("\033[A");
        index++;
    }
    pf_printf("\r");
    return (FT_ERR_SUCCESS);
}

int32_t rl_read_key(char *character_out)
{
    int64_t bytes_read;
    char character;

    if (character_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);

    while (1)
    {
        bytes_read = su_read(0, &character, 1);
        if (bytes_read == 1)
        {
            *character_out = character;
            return (FT_ERR_SUCCESS);
        }
        if (bytes_read == FT_ERR_SUCCESS)
        {
            return (FT_ERR_TERMINATED);
        }
        if (bytes_read < 0)
        {
            return (FT_ERR_TERMINATED);
        }
    }
}

void rl_update_history(const char *buffer)
{
    char *duplicated_entry;

    if (history_count < MAX_HISTORY)
    {
        duplicated_entry = adv_strdup(buffer);
        if (duplicated_entry == ft_nullptr)
            return ;
        history[history_count] = duplicated_entry;
        history_count++;
        rl_history_notify_updated();
        return ;
    }
    duplicated_entry = adv_strdup(buffer);
    if (duplicated_entry == ft_nullptr)
        return ;
    cma_free(history[0]);
    ft_memmove(&history[0], &history[1], sizeof(char *) * (MAX_HISTORY - 1));
    history[MAX_HISTORY - 1] = duplicated_entry;
    rl_history_notify_updated();
    return ;
}

static ft_bool rl_history_utf32_contains(const char32_t *haystack,
        ft_size_t haystack_length, const char32_t *needle,
        ft_size_t needle_length)
{
    ft_size_t haystack_index;

    if (haystack == ft_nullptr || needle == ft_nullptr)
        return (FT_FALSE);
    if (needle_length == FT_ERR_SUCCESS)
        return (FT_TRUE);
    if (haystack_length < needle_length)
        return (FT_FALSE);
    haystack_index = 0;
    while (haystack_index + needle_length <= haystack_length)
    {
        ft_size_t comparison_index;

        comparison_index = 0;
        while (comparison_index < needle_length
            && haystack[haystack_index + comparison_index]
            == needle[comparison_index])
            comparison_index += 1;
        if (comparison_index == needle_length)
            return (FT_TRUE);
        haystack_index += 1;
    }
    return (FT_FALSE);
}

int32_t rl_history_search(const char *query, int32_t start_index,
        ft_bool search_backward, int32_t *match_index)
{
    char32_t *query_code_points;
    ft_size_t query_length;
    int32_t current_index;

    if (query == ft_nullptr || match_index == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    *match_index = -1;
    query_code_points = ft_utf8_to_utf32(query, 0, &query_length);
    if (query_code_points == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (query_length == FT_ERR_SUCCESS)
    {
        cma_free(query_code_points);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (history_count <= 0)
    {
        cma_free(query_code_points);
        return (FT_ERR_NOT_FOUND);
    }
    current_index = 0;
    if (search_backward)
    {
        if (start_index < 0 || start_index >= history_count)
            current_index = history_count - 1;
        else
            current_index = start_index;
        while (current_index >= 0)
        {
            char *history_entry;
            char32_t *entry_code_points;
            ft_size_t entry_length;
            ft_bool contains_result;

            history_entry = history[current_index];
            if (history_entry == ft_nullptr)
            {
                current_index -= 1;
                continue ;
            }
            entry_code_points = ft_utf8_to_utf32(history_entry, 0, &entry_length);
            if (entry_code_points == ft_nullptr)
            {
                cma_free(query_code_points);
                return (FT_ERR_INVALID_ARGUMENT);
            }
            contains_result = rl_history_utf32_contains(entry_code_points,
                    entry_length, query_code_points, query_length);
            cma_free(entry_code_points);
            if (contains_result == FT_TRUE)
            {
                *match_index = current_index;
                cma_free(query_code_points);
                return (FT_ERR_SUCCESS);
            }
            current_index -= 1;
        }
    }
    else
    {
        if (start_index < 0 || start_index >= history_count)
            current_index = 0;
        else
            current_index = start_index;
        while (current_index < history_count)
        {
            char *history_entry;
            char32_t *entry_code_points;
            ft_size_t entry_length;
            ft_bool contains_result;

            history_entry = history[current_index];
            if (history_entry == ft_nullptr)
            {
                current_index += 1;
                continue ;
            }
            entry_code_points = ft_utf8_to_utf32(history_entry, 0, &entry_length);
            if (entry_code_points == ft_nullptr)
            {
                cma_free(query_code_points);
                return (FT_ERR_INVALID_ARGUMENT);
            }
            contains_result = rl_history_utf32_contains(entry_code_points,
                    entry_length, query_code_points, query_length);
            cma_free(entry_code_points);
            if (contains_result == FT_TRUE)
            {
                *match_index = current_index;
                cma_free(query_code_points);
                return (FT_ERR_SUCCESS);
            }
            current_index += 1;
        }
    }
    cma_free(query_code_points);
    return (FT_ERR_NOT_FOUND);
}
