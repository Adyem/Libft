#include <climits>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../Printf/printf.hpp"
#include "../Libft/libft.hpp"
#include "readline_internal.hpp"

int rl_handle_printable_char(readline_state_t *state, char c, const char *prompt)
{
    bool lock_acquired;
    int  result;
    int new_bufsize;
    char *resized_buffer;
    int length_after_cursor;

    if (state == ft_nullptr || prompt == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    result = -1;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    if (state->buffer == ft_nullptr || state->bufsize <= 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (state->pos < 0 || state->pos > state->bufsize)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (state->pos >= state->bufsize - 1)
    {
        if (state->bufsize > INT_MAX / 2)
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            goto cleanup;
        }
        new_bufsize = state->bufsize * 2;
        resized_buffer = rl_resize_buffer(state->buffer, state->bufsize, new_bufsize);
        if (resized_buffer == ft_nullptr)
            goto cleanup;
        state->buffer = resized_buffer;
        state->bufsize = new_bufsize;
    }
    ft_memmove(&state->buffer[state->pos + 1], &state->buffer[state->pos],
               ft_strlen(&state->buffer[state->pos]) + 1);
    state->buffer[state->pos] = c;
    state->pos++;
    if (rl_update_display_metrics(state) != 0)
        goto cleanup;
    rl_clear_line(prompt, state->buffer);
    pf_printf("%s%s", prompt, state->buffer);
    length_after_cursor = state->prev_display_columns - state->display_pos;
    if (length_after_cursor > 0)
        pf_printf("\033[%dD", length_after_cursor);
    fflush(stdout);
    ft_errno = ER_SUCCESS;
    result = 0;
cleanup:
    rl_state_unlock(state, lock_acquired);
    return (result);
}
