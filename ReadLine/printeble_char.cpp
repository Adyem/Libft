#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../Printf/printf.hpp"
#include "../Libft/libft.hpp"
#include "readline_internal.hpp"

int rl_handle_printable_char(readline_state_t *state, char c, const char *prompt)
{
    if (state->pos >= state->bufsize - 1)
	{
        int new_bufsize = state->bufsize * 2;
        state->buffer = rl_resize_buffer(state->buffer, state->bufsize, new_bufsize);
        if (!state->buffer)
            return (-1);
        state->bufsize = new_bufsize;
    }
    ft_memmove(&state->buffer[state->pos + 1], &state->buffer[state->pos],
               ft_strlen(&state->buffer[state->pos]) + 1);
    state->buffer[state->pos] = c;
    state->pos++;
    state->prev_buffer_length = ft_strlen(state->buffer);
    rl_clear_line(prompt, state->buffer);
    pf_printf("%s%s", prompt, state->buffer);
    int len_after_cursor = state->prev_buffer_length - state->pos;
    if (len_after_cursor > 0)
        pf_printf("\033[%dD", len_after_cursor);
    fflush(stdout);
    return (0);
}
