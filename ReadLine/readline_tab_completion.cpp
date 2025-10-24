#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Printf/printf.hpp"
#include "../Libft/limits.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "readline_internal.hpp"

static int rl_find_word_start_and_prefix(readline_state_t *state, char *prefix, int *prefix_len, int prefix_capacity)
{
    state->word_start = state->pos - 1;
    while (state->word_start >= 0 && state->buffer[state->word_start] != ' ')
        state->word_start--;
    state->word_start++;
    *prefix_len = state->pos - state->word_start;
    if (*prefix_len == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (*prefix_len >= prefix_capacity)
    {
        prefix[0] = '\0';
        *prefix_len = 0;
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    ft_memcpy(prefix, &state->buffer[state->word_start], *prefix_len);
    prefix[*prefix_len] = '\0';
    ft_errno = ER_SUCCESS;
    return (0);
}

static void rl_gather_matching_suggestions(readline_state_t *state, const char *prefix, int prefix_len)
{
    int dynamic_count;
    int dynamic_index;
    int index;

    state->current_match_count = 0;
    dynamic_count = rl_completion_get_dynamic_count();
    dynamic_index = 0;
    while (dynamic_index < dynamic_count)
    {
        char *candidate;

        candidate = rl_completion_get_dynamic_match(dynamic_index);
        if (candidate != ft_nullptr && ft_strncmp(candidate, prefix, prefix_len) == 0)
        {
            state->current_matches[state->current_match_count] = candidate;
            state->current_match_count += 1;
        }
        dynamic_index++;
    }
    index = 0;
    while (index < suggestion_count)
    {
        if (ft_strncmp(suggestions[index], prefix, prefix_len) == 0)
        {
            state->current_matches[state->current_match_count] = suggestions[index];
            state->current_match_count += 1;
        }
        index++;
    }
    return ;
}

static int rl_resize_buffer_if_needed(readline_state_t *state, int required_size)
{
    if (required_size >= state->bufsize)
    {
        char *resized_buffer;
        int new_bufsize;

        new_bufsize = state->bufsize;
        if (new_bufsize <= 0)
            new_bufsize = 1;
        while (required_size >= new_bufsize)
        {
            if (new_bufsize > FT_INT_MAX / 2)
            {
                ft_errno = FT_ERR_OUT_OF_RANGE;
                return (-1);
            }
            new_bufsize *= 2;
        }
        resized_buffer = rl_resize_buffer(state->buffer, state->bufsize, new_bufsize);
        if (resized_buffer == ft_nullptr)
            return (-1);
        state->buffer = resized_buffer;
        state->bufsize = new_bufsize;
    }
    return (0);
}

static int rl_apply_completion(readline_state_t *state, const char *completion)
{
    int completion_length;
    int original_position;
    int suffix_length;
    long long total_length;
    int required_size;

    completion_length = ft_strlen(completion);
    if (ft_errno != ER_SUCCESS)
        return (-1);
    original_position = state->pos;
    suffix_length = ft_strlen(&state->buffer[original_position]);
    if (ft_errno != ER_SUCCESS)
        return (-1);
    total_length = static_cast<long long>(state->word_start)
        + static_cast<long long>(completion_length)
        + static_cast<long long>(suffix_length);
    if (total_length > static_cast<long long>(FT_INT_MAX))
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    state->pos = state->word_start;
    required_size = static_cast<int>(total_length);
    if (rl_resize_buffer_if_needed(state, required_size) == -1)
        return (-1);
    ft_memmove(&state->buffer[state->pos + completion_length],
        &state->buffer[original_position],
        static_cast<size_t>(suffix_length) + 1);
    if (completion_length > 0)
        ft_memcpy(&state->buffer[state->pos], completion,
            static_cast<size_t>(completion_length));
    state->pos += completion_length;
    return (0);
}

static void rl_update_display(const char *prompt, readline_state_t *state)
{
    int columns_after_cursor;

    if (rl_update_display_metrics(state) != 0)
        return ;
    if (rl_clear_line(prompt, state->buffer) == -1)
        return ;
    pf_printf("%s%s", prompt, state->buffer);
    columns_after_cursor = state->prev_display_columns - state->display_pos;
    if (columns_after_cursor > 0)
        pf_printf("\033[%dD", columns_after_cursor);
    fflush(stdout);
    return ;
}

int rl_handle_tab_completion(readline_state_t *state, const char *prompt)
{
    bool        lock_acquired;
    int         result;
    const char *completion;

    if (state == ft_nullptr || prompt == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    result = 0;
    completion = ft_nullptr;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    if (!state->in_completion_mode)
    {
        char prefix[INITIAL_BUFFER_SIZE];
        int prefix_len;
        int prefix_status;

        prefix_status = rl_find_word_start_and_prefix(state, prefix, &prefix_len, INITIAL_BUFFER_SIZE);
        if (prefix_status != 0)
        {
            result = -1;
            goto cleanup;
        }
        if (prefix_len == 0)
            goto cleanup_success;
        if (rl_completion_prepare_candidates(state->buffer, state->pos, prefix, prefix_len) != 0)
        {
            result = -1;
            goto cleanup;
        }
        rl_gather_matching_suggestions(state, prefix, prefix_len);
        if (state->current_match_count == 0)
        {
            pf_printf("\a");
            fflush(stdout);
            goto cleanup_success;
        }
        state->in_completion_mode = 1;
        state->current_match_index = 0;
    }
    if (state->in_completion_mode && state->current_match_count > 0)
    {
        completion = state->current_matches[state->current_match_index];
        if (rl_apply_completion(state, completion))
        {
            result = -1;
            goto cleanup;
        }
        rl_update_display(prompt, state);
        state->current_match_index = (state->current_match_index + 1) % state->current_match_count;
    }
cleanup_success:
    ft_errno = ER_SUCCESS;
cleanup:
    rl_state_unlock(state, lock_acquired);
    return (result);
}
