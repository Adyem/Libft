#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Printf/printf.hpp"
#include "../Libft/libft.hpp"
#include "readline_internal.hpp"

int rl_handle_backspace(readline_state_t *state, const char *prompt)
{
    if (state->pos > 0)
    {
        state->pos--;
        ft_memmove(&state->buffer[state->pos], &state->buffer[state->pos + 1],
           ft_strlen(state->buffer) - state->pos + 1);
        state->prev_buffer_length = ft_strlen(state->buffer);
        if (rl_clear_line(prompt, state->buffer) == -1)
		{
			state->error_file.printf("clear line failed");
			return (-1);
		}
        pf_printf("%s%s", prompt, state->buffer);
        int len_after_cursor = state->prev_buffer_length - state->pos;
        if (len_after_cursor > 0)
            pf_printf("\033[%dD", len_after_cursor);
        fflush(stdout);
    }
	return (0);
}

static void rl_handle_left_arrow(readline_state_t *state, const char *prompt)
{
    if (state->pos > 0)
    {
        state->pos--;
        if (rl_clear_line(prompt, state->buffer) == -1)
            return ;
        pf_printf("%s%s", prompt, state->buffer);
        int len_after_cursor = ft_strlen(state->buffer) - state->pos;
        if (len_after_cursor > 0)
            pf_printf("\033[%dD", len_after_cursor);
        fflush(stdout);
    }
	return ;
}

static void rl_handle_right_arrow(readline_state_t *state, const char *prompt)
{
    if (state->pos < ft_strlen(state->buffer))
    {
        state->pos++;
        if (rl_clear_line(prompt, state->buffer) == -1)
            return ;
        pf_printf("%s%s", prompt, state->buffer);
        int len_after_cursor = ft_strlen(state->buffer) - state->pos;
        if (len_after_cursor > 0)
            pf_printf("\033[%dD", len_after_cursor);
        fflush(stdout);
    }
	return ;
}

void rl_reset_completion_mode(readline_state_t *state)
{
    state->in_completion_mode = 0;
    state->current_match_count = 0;
    state->current_match_index = 0;
	return ;
}

int rl_read_escape_sequence(char seq[2])
{
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
        return (0);
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
        return (0);
    return (1);
}

static int rl_handle_up_arrow(readline_state_t *state, const char *prompt)
{
    if (state->history_index > 0)
	{
        state->history_index--;
        if (rl_clear_line(prompt, state->buffer) == -1)
			return (-1);
        state->pos = 0;
        strncpy(state->buffer, history[state->history_index], state->bufsize - 1);
        state->buffer[state->bufsize - 1] = '\0';
        pf_printf("%s%s", prompt, state->buffer);
        state->pos = ft_strlen(state->buffer);
        state->prev_buffer_length = state->pos;
        fflush(stdout);
    }
	return (0);
}

static int rl_handle_down_arrow(readline_state_t *state, const char *prompt)
{
	if (state->history_index < history_count - 1)
	{
        state->history_index++;
        if (rl_clear_line(prompt, state->buffer) == -1)
			return (-1);
        state->pos = 0;
        strncpy(state->buffer, history[state->history_index], state->bufsize - 1);
        state->buffer[state->bufsize - 1] = '\0';
        pf_printf("%s%s", prompt, state->buffer);
        state->pos = ft_strlen(state->buffer);
        state->prev_buffer_length = state->pos;
        fflush(stdout);
    }
	else if (state->history_index == history_count - 1)
	{
        state->history_index++;
        if (rl_clear_line(prompt, state->buffer) == -1)
			return (-1);
        state->pos = 0;
        state->buffer[0] = '\0';
        pf_printf("%s", prompt);
        state->prev_buffer_length = 0;
        fflush(stdout);
    }
	return (0);
}

static int rl_handle_arrow_keys(readline_state_t *state, const char *prompt, char direction)
{
	if (direction == 'A')
        return (rl_handle_up_arrow(state, prompt));
	else if (direction == 'B')
        return (rl_handle_down_arrow(state, prompt));
	else if (direction == 'C')
        rl_handle_right_arrow(state, prompt);
	else if (direction == 'D')
        rl_handle_left_arrow(state, prompt);
	return (0);
}

int rl_handle_escape_sequence(readline_state_t *state, const char *prompt)
{
    if (state->in_completion_mode)
        rl_reset_completion_mode(state);
    char seq[2];
	if (!rl_read_escape_sequence(seq))
        return (0);
	if (seq[0] == '[')
        return (rl_handle_arrow_keys(state, prompt, seq[1]));
	return (0);
}
