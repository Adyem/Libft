#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Printf/printf.hpp"
#include "../Basic/limits.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "readline_internal.hpp"

static int32_t rl_find_word_start_and_prefix(readline_state_t *state, char *prefix, int32_t *prefix_len, int32_t prefix_capacity)
{
    state->word_start = state->position - 1;
    while (state->word_start >= 0 && state->buffer[state->word_start] != ' ')
        state->word_start--;
    state->word_start++;
    *prefix_len = state->position - state->word_start;
    if (*prefix_len == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
    if (*prefix_len >= prefix_capacity)
    {
        prefix[0] = '\0';
        *prefix_len = 0;
        return (FT_ERR_OUT_OF_RANGE);
    }
    ft_memcpy(prefix, &state->buffer[state->word_start], *prefix_len);
    prefix[*prefix_len] = '\0';
    return (FT_ERR_SUCCESS);
}

static int32_t rl_gather_matching_suggestions(readline_state_t *state, const char *prefix, int32_t prefix_len)
{
    int32_t dynamic_count;
    int32_t dynamic_index;
    int32_t index;

    state->current_match_count = 0;
    if (rl_completion_get_dynamic_count(&dynamic_count) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    dynamic_index = 0;
    while (dynamic_index < dynamic_count)
    {
        char *candidate;

        candidate = rl_completion_get_dynamic_match(dynamic_index);
        if (candidate != ft_nullptr && ft_strncmp(candidate, prefix, prefix_len) == FT_ERR_SUCCESS)
        {
            state->current_matches[state->current_match_count] = candidate;
            state->current_match_count += 1;
        }
        dynamic_index++;
    }
    index = 0;
    while (index < suggestion_count)
    {
        if (ft_strncmp(suggestions[index], prefix, prefix_len) == FT_ERR_SUCCESS)
        {
            state->current_matches[state->current_match_count] = suggestions[index];
            state->current_match_count += 1;
        }
        index++;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t rl_resize_buffer_if_needed(readline_state_t *state, int32_t required_size)
{
    if (required_size >= state->buffer_size)
    {
        int32_t new_bufsize;

        new_bufsize = state->buffer_size;
        if (new_bufsize <= 0)
            new_bufsize = 1;
        while (required_size >= new_bufsize)
        {
            if (new_bufsize > FT_INT32_MAX / 2)
                return (FT_ERR_OUT_OF_RANGE);
            new_bufsize *= 2;
        }
        int32_t resize_error = rl_resize_buffer(&state->buffer, &state->buffer_size, new_bufsize);
        if (resize_error != FT_ERR_SUCCESS)
            return (resize_error);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t rl_apply_completion(readline_state_t *state, const char *completion)
{
    int32_t completion_length;
    int32_t original_position;
    int32_t suffix_length;
    int64_t total_length;
    int32_t required_size;

    completion_length = ft_strlen(completion);
    original_position = state->position;
    suffix_length = ft_strlen(&state->buffer[original_position]);
    total_length = static_cast<int64_t>(state->word_start)
        + static_cast<int64_t>(completion_length)
        + static_cast<int64_t>(suffix_length);
    if (total_length > static_cast<int64_t>(FT_INT32_MAX))
        return (FT_ERR_OUT_OF_RANGE);
    state->position = state->word_start;
    required_size = static_cast<int32_t>(total_length);
    int32_t resize_error = rl_resize_buffer_if_needed(state, required_size);
    if (resize_error != FT_ERR_SUCCESS)
        return (resize_error);
    ft_memmove(&state->buffer[state->position + completion_length],
        &state->buffer[original_position],
        static_cast<ft_size_t>(suffix_length) + 1);
    if (completion_length > 0)
        ft_memcpy(&state->buffer[state->position], completion,
            static_cast<ft_size_t>(completion_length));
    state->position += completion_length;
    return (FT_ERR_SUCCESS);
}

static void rl_update_display(const char *prompt, readline_state_t *state)
{
    int32_t columns_after_cursor;

    if (rl_update_display_metrics(state) != FT_ERR_SUCCESS)
        return ;
    if (rl_clear_line(prompt, state->buffer) != FT_ERR_SUCCESS)
        return ;
    pf_printf("%s%s", prompt, state->buffer);
    columns_after_cursor = state->prev_display_columns - state->display_pos;
    if (columns_after_cursor > 0)
        pf_printf("\033[%dD", columns_after_cursor);
    fflush(stdout);
    return ;
}

int32_t rl_handle_tab_completion(readline_state_t *state, const char *prompt)
{
    ft_bool        lock_acquired;
    int32_t         result;
    const char *completion;
    int32_t         lock_result;

    if (state == ft_nullptr || prompt == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    result = FT_ERR_SUCCESS;
    completion = ft_nullptr;
    lock_result = rl_state_lock(state, &lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (!state->in_completion_mode)
    {
        char prefix[INITIAL_BUFFER_SIZE];
        int32_t prefix_len;
        int32_t prefix_status;

        prefix_status = rl_find_word_start_and_prefix(state, prefix, &prefix_len, INITIAL_BUFFER_SIZE);
        if (prefix_status != FT_ERR_SUCCESS)
        {
            result = prefix_status;
            goto cleanup;
        }
        if (prefix_len == FT_ERR_SUCCESS)
            goto cleanup_success;
        if (rl_completion_prepare_candidates(state->buffer, state->position, prefix, prefix_len) != FT_ERR_SUCCESS)
        {
            result = FT_ERR_INTERNAL;
            goto cleanup;
        }
        if (rl_gather_matching_suggestions(state, prefix, prefix_len) != FT_ERR_SUCCESS)
        {
            result = FT_ERR_INTERNAL;
            goto cleanup;
        }
        if (state->current_match_count == FT_ERR_SUCCESS)
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
        int32_t completion_result = rl_apply_completion(state, completion);
        if (completion_result != FT_ERR_SUCCESS)
        {
            result = completion_result;
            goto cleanup;
        }
        rl_update_display(prompt, state);
        state->current_match_index = (state->current_match_index + 1) % state->current_match_count;
    }
cleanup_success:
    result = FT_ERR_SUCCESS;
cleanup:
    (void)rl_state_unlock(state, lock_acquired);
    return (result);
}
