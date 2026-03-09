#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Printf/printf.hpp"
#include "../Basic/basic.hpp"
#include "readline_internal.hpp"

int32_t rl_handle_backspace(readline_state_t *state, const char *prompt)
{
    ft_bool lock_acquired;
    int32_t  result;
    int32_t  lock_error;

    if (state == ft_nullptr || prompt == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    result = FT_ERR_SUCCESS;
    lock_error = rl_state_lock(state, &lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (state->position > 0)
    {
        int32_t grapheme_start;
        int32_t grapheme_end;
        int32_t grapheme_columns;
        int32_t len_after_cursor;
        int32_t tail_length;

        if (rl_utf8_find_previous_grapheme(state->buffer, state->position,
                &grapheme_start, &grapheme_end, &grapheme_columns) == FT_ERR_SUCCESS)
        {
            (void)grapheme_columns;
            tail_length = ft_strlen(state->buffer) - grapheme_end + 1;
            ft_memmove(&state->buffer[grapheme_start],
                &state->buffer[grapheme_end], tail_length);
            state->position = grapheme_start;
            if (rl_update_display_metrics(state) != FT_ERR_SUCCESS
                || rl_clear_line(prompt, state->buffer) != FT_ERR_SUCCESS)
            {
                state->error_file.printf("clear line failed");
                result = FT_ERR_INTERNAL;
                goto cleanup;
            }
            pf_printf("%s%s", prompt, state->buffer);
            len_after_cursor = state->prev_display_columns - state->display_pos;
            if (len_after_cursor > 0)
                pf_printf("\033[%dD", len_after_cursor);
            fflush(stdout);
        }
    }
cleanup:
    (void)rl_state_unlock(state, lock_acquired);
    return (result);
}

static void rl_handle_left_arrow(readline_state_t *state, const char *prompt)
{
    if (state->position > 0)
    {
        int32_t grapheme_start;
        int32_t grapheme_end;
        int32_t grapheme_columns;

        if (rl_utf8_find_previous_grapheme(state->buffer, state->position,
                &grapheme_start, &grapheme_end, &grapheme_columns) == FT_ERR_SUCCESS)
        {
            (void)grapheme_end;
            (void)grapheme_columns;
            state->position = grapheme_start;
            if (rl_update_display_metrics(state) != FT_ERR_SUCCESS)
                return ;
            if (rl_clear_line(prompt, state->buffer) != FT_ERR_SUCCESS)
                return ;
            pf_printf("%s%s", prompt, state->buffer);
            int32_t len_after_cursor = state->prev_display_columns - state->display_pos;
            if (len_after_cursor > 0)
                pf_printf("\033[%dD", len_after_cursor);
            fflush(stdout);
        }
    }
    return ;
}

static void rl_handle_right_arrow(readline_state_t *state, const char *prompt)
{
    ft_size_t buffer_length;

    buffer_length = ft_strlen_size_t(state->buffer);
    if (state->position < static_cast<int32_t>(buffer_length))
    {
        int32_t grapheme_start;
        int32_t grapheme_end;
        int32_t grapheme_columns;

        if (rl_utf8_find_next_grapheme(state->buffer, state->position,
                &grapheme_start, &grapheme_end, &grapheme_columns) == FT_ERR_SUCCESS)
        {
            (void)grapheme_start;
            (void)grapheme_columns;
            state->position = grapheme_end;
            if (rl_update_display_metrics(state) != FT_ERR_SUCCESS)
                return ;
            if (rl_clear_line(prompt, state->buffer) != FT_ERR_SUCCESS)
                return ;
            pf_printf("%s%s", prompt, state->buffer);
            int32_t len_after_cursor = state->prev_display_columns - state->display_pos;
            if (len_after_cursor > 0)
                pf_printf("\033[%dD", len_after_cursor);
            fflush(stdout);
        }
    }
    return ;
}

static int32_t rl_copy_history_entry_to_buffer(readline_state_t *state, const char *history_entry)
{
    ft_size_t entry_length = ft_strlen(history_entry);
    int32_t required_size = static_cast<int32_t>(entry_length) + 1;

    if (required_size > state->buffer_size)
    {
        int32_t new_bufsize = state->buffer_size;

        if (new_bufsize == FT_ERR_SUCCESS)
            new_bufsize = 1;
        while (new_bufsize < required_size)
            new_bufsize *= 2;
        int32_t resize_error = rl_resize_buffer(&state->buffer, &state->buffer_size, new_bufsize);

        if (resize_error != FT_ERR_SUCCESS)
            return (resize_error);
    }
    ft_strlcpy(state->buffer, history_entry, state->buffer_size);
    return (FT_ERR_SUCCESS);
}

static void rl_reset_completion_mode_locked(readline_state_t *state)
{
    if (state == ft_nullptr)
        return ;
    state->in_completion_mode = 0;
    state->current_match_count = 0;
    state->current_match_index = 0;
    rl_completion_reset_dynamic_matches();
    return ;
}

void rl_reset_completion_mode(readline_state_t *state)
{
    ft_bool lock_acquired;

    if (state == ft_nullptr)
        return ;
    lock_acquired = FT_FALSE;
    if (rl_state_lock(state, &lock_acquired) != FT_ERR_SUCCESS)
        return ;
    rl_reset_completion_mode_locked(state);
    rl_state_unlock(state, lock_acquired);
    return ;
}

int32_t rl_read_escape_sequence(char escape_sequence[2])
{
    if (read(STDIN_FILENO, &escape_sequence[0], 1) != 1)
        return (FT_ERR_INTERNAL);
    if (read(STDIN_FILENO, &escape_sequence[1], 1) != 1)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

static int32_t rl_handle_up_arrow(readline_state_t *state, const char *prompt)
{
    if (state->history_index > 0)
    {
        state->history_index--;
        if (rl_clear_line(prompt, state->buffer) != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        state->position = 0;
        int32_t copy_error = rl_copy_history_entry_to_buffer(state, history[state->history_index]);
        if (copy_error != FT_ERR_SUCCESS)
            return (copy_error);
        pf_printf("%s%s", prompt, state->buffer);
        state->position = ft_strlen(state->buffer);
        if (rl_update_display_metrics(state) != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        fflush(stdout);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t rl_handle_down_arrow(readline_state_t *state, const char *prompt)
{
    if (state->history_index < history_count - 1)
    {
        state->history_index++;
        if (rl_clear_line(prompt, state->buffer) != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        state->position = 0;
        int32_t copy_error = rl_copy_history_entry_to_buffer(state, history[state->history_index]);
        if (copy_error != FT_ERR_SUCCESS)
            return (copy_error);
        pf_printf("%s%s", prompt, state->buffer);
        state->position = ft_strlen(state->buffer);
        if (rl_update_display_metrics(state) != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        fflush(stdout);
    }
    else if (state->history_index == history_count - 1)
    {
        state->history_index++;
        if (rl_clear_line(prompt, state->buffer) != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
        state->position = 0;
        state->buffer[0] = '\0';
        pf_printf("%s", prompt);
        state->display_pos = 0;
        state->prev_display_columns = 0;
        state->prev_buffer_length = 0;
        fflush(stdout);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t rl_handle_arrow_keys(readline_state_t *state, const char *prompt, char direction)
{
    if (direction == 'A')
        return (rl_handle_up_arrow(state, prompt));
    else if (direction == 'B')
        return (rl_handle_down_arrow(state, prompt));
    else if (direction == 'C')
        rl_handle_right_arrow(state, prompt);
    else if (direction == 'D')
        rl_handle_left_arrow(state, prompt);
    return (FT_ERR_SUCCESS);
}

int32_t rl_handle_escape_sequence(readline_state_t *state, const char *prompt)
{
    ft_bool lock_acquired;
    int32_t  result;
    int32_t  lock_error;
    char escape_sequence[2];

    if (state == ft_nullptr || prompt == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    result = FT_ERR_SUCCESS;
    lock_error = rl_state_lock(state, &lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (state->in_completion_mode)
        rl_reset_completion_mode_locked(state);
    if (rl_read_escape_sequence(escape_sequence) != FT_ERR_SUCCESS)
    {
        result = FT_ERR_INTERNAL;
        goto cleanup;
    }
    if (escape_sequence[0] == '[')
        result = rl_handle_arrow_keys(state, prompt, escape_sequence[1]);
cleanup:
    (void)rl_state_unlock(state, lock_acquired);
    return (result);
}
