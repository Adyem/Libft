#include <stdlib.h>
#include <unistd.h>
#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "readline_internal.hpp"

int rl_resize_buffer(char **buffer_pointer, int *current_size_pointer, int new_size)
{
    char *old_buffer;
    int current_size;
    size_t copy_size;
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
        copy_size = static_cast<size_t>(current_size);
        if (current_size > new_size)
            copy_size = static_cast<size_t>(new_size);
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
    if (copy_size < static_cast<size_t>(new_size))
        ft_bzero(new_buffer + copy_size, static_cast<size_t>(new_size) - copy_size);
    *buffer_pointer = new_buffer;
    *current_size_pointer = new_size;
    return (FT_ERR_SUCCESSS);
}

int rl_clear_line(const char *prompt, const char *buffer)
{
    int prompt_length;
    int buffer_length;
    int total_length;

    if (prompt == ft_nullptr || buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    int prompt_columns_error = rl_utf8_compute_columns(prompt, &prompt_length);
    if (prompt_columns_error != FT_ERR_SUCCESSS)
        return (prompt_columns_error);
    int buffer_columns_error = rl_utf8_compute_columns(buffer, &buffer_length);
    if (buffer_columns_error != FT_ERR_SUCCESSS)
        return (buffer_columns_error);
    total_length = prompt_length + buffer_length;
    pf_printf("\r");
    int term_width = rl_get_terminal_width();
    if (term_width <= 0)
        term_width = 1;
    int line_count = (total_length / term_width) + 1;
    int index = 0;
    while (index < line_count)
    {
        pf_printf("\033[2K");
        if (index < line_count - 1)
            pf_printf("\033[A");
        index++;
    }
    pf_printf("\r");
    return (FT_ERR_SUCCESSS);
}

int rl_read_key(char *character_out)
{
    ssize_t bytes_read;
    char character;

    if (character_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);

    while (1)
    {
        bytes_read = su_read(0, &character, 1);
        if (bytes_read == 1)
        {
            *character_out = character;
            return (FT_ERR_SUCCESSS);
        }
        if (bytes_read == 0)
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
        duplicated_entry = cma_strdup(buffer);
        if (duplicated_entry == ft_nullptr)
            return ;
        history[history_count] = duplicated_entry;
        history_count++;
        rl_history_notify_updated();
        return ;
    }
    duplicated_entry = cma_strdup(buffer);
    if (duplicated_entry == ft_nullptr)
        return ;
    cma_free(history[0]);
    ft_memmove(&history[0], &history[1], sizeof(char *) * (MAX_HISTORY - 1));
    history[MAX_HISTORY - 1] = duplicated_entry;
    rl_history_notify_updated();
    return ;
}

static int rl_history_utf32_contains(const char32_t *haystack,
        size_t haystack_length, const char32_t *needle,
        size_t needle_length)
{
    size_t haystack_index;

    if (haystack == ft_nullptr || needle == ft_nullptr)
        return (-1);
    if (needle_length == 0)
        return (1);
    if (haystack_length < needle_length)
        return (0);
    haystack_index = 0;
    while (haystack_index + needle_length <= haystack_length)
    {
        size_t comparison_index;

        comparison_index = 0;
        while (comparison_index < needle_length
            && haystack[haystack_index + comparison_index]
            == needle[comparison_index])
            comparison_index += 1;
        if (comparison_index == needle_length)
            return (1);
        haystack_index += 1;
    }
    return (0);
}

int rl_history_search(const char *query, int start_index,
        bool search_backward, int *match_index)
{
    char32_t *query_code_points;
    size_t query_length;
    int current_index;

    if (query == ft_nullptr || match_index == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    *match_index = -1;
    query_code_points = ft_utf8_to_utf32(query, 0, &query_length);
    int conversion_error = ft_global_error_stack_drop_last_error();
        if (query_code_points == ft_nullptr)
        {
            if (conversion_error == FT_ERR_SUCCESSS)
                return (FT_ERR_INVALID_ARGUMENT);
            return (conversion_error);
        }
    if (query_length == 0)
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
            size_t entry_length;
            int contains_result;
            int entry_error;

            history_entry = history[current_index];
            if (history_entry == ft_nullptr)
            {
                current_index -= 1;
                continue ;
            }
            entry_code_points = ft_utf8_to_utf32(history_entry, 0, &entry_length);
            entry_error = ft_global_error_stack_drop_last_error();
            if (entry_code_points == ft_nullptr)
            {
                cma_free(query_code_points);
                if (entry_error == FT_ERR_SUCCESSS)
                    return (FT_ERR_INVALID_ARGUMENT);
                return (entry_error);
            }
            contains_result = rl_history_utf32_contains(entry_code_points,
                    entry_length, query_code_points, query_length);
            cma_free(entry_code_points);
            if (contains_result < 0)
            {
                cma_free(query_code_points);
                return (FT_ERR_INVALID_ARGUMENT);
            }
            if (contains_result == 1)
            {
                *match_index = current_index;
                cma_free(query_code_points);
                return (FT_ERR_SUCCESSS);
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
            size_t entry_length;
            int contains_result;
            int entry_error;

            history_entry = history[current_index];
            if (history_entry == ft_nullptr)
            {
                current_index += 1;
                continue ;
            }
            entry_code_points = ft_utf8_to_utf32(history_entry, 0, &entry_length);
            entry_error = ft_global_error_stack_drop_last_error();
            if (entry_code_points == ft_nullptr)
            {
                cma_free(query_code_points);
                if (entry_error == FT_ERR_SUCCESSS)
                    return (FT_ERR_INVALID_ARGUMENT);
                return (entry_error);
            }
            contains_result = rl_history_utf32_contains(entry_code_points,
                    entry_length, query_code_points, query_length);
            cma_free(entry_code_points);
            if (contains_result < 0)
            {
                cma_free(query_code_points);
                return (FT_ERR_INVALID_ARGUMENT);
            }
            if (contains_result == 1)
            {
                *match_index = current_index;
                cma_free(query_code_points);
                return (FT_ERR_SUCCESSS);
            }
            current_index += 1;
        }
    }
    cma_free(query_code_points);
    return (FT_ERR_NOT_FOUND);
}
