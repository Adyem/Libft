#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "readline_internal.hpp"
#include "readline.hpp"

char    *history[MAX_HISTORY];
int        history_count = 0;
char    *suggestions[MAX_SUGGESTIONS];
int        suggestion_count = 0;

static void rl_cleanup_state(readline_state_t *state)
{
    bool lock_acquired;

    if (state == ft_nullptr)
        return ;
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) == 0)
    {
        if (state->buffer)
        {
            cma_free(state->buffer);
            state->buffer = ft_nullptr;
        }
        rl_state_unlock(state, lock_acquired);
    }
    rl_state_teardown_thread_safety(state);
    return ;
}

static char *rl_error(readline_state_t *state)
{
    rl_cleanup_state(state);
    rl_disable_raw_mode();
    return (ft_nullptr);
}

char *rl_readline(const char *prompt)
{
    readline_state_t     state;

    if (rl_initialize_state(&state))
        return (ft_nullptr);
    pf_printf("%s", prompt);
    fflush(stdout);
    while (1)
    {
        int key_result = rl_read_key();

        if (key_result == -1)
            return (rl_error(&state));
        char character = static_cast<char>(key_result);
        int custom_result = rl_dispatch_custom_key(&state, prompt, key_result);

        if (custom_result == -1)
            return (rl_error(&state));
        if (custom_result == 1)
            continue ;

        if (character != '\t' && state.in_completion_mode)
        {
            state.in_completion_mode = 0;
            state.current_match_count = 0;
            state.current_match_index = 0;
            rl_completion_reset_dynamic_matches();
        }
        if (character == '\r' || character == '\n')
        {
            pf_printf("\n");
            break ;
        }
        else if (character == 127 || character == '\b')
        {
            if (rl_handle_backspace(&state, prompt) == -1)
                return (rl_error(&state));
        }
        else if (character == 27)
        {
            if (rl_handle_escape_sequence(&state, prompt) == -1)
                return (rl_error(&state));
        }
        else if (character == '\t')
        {
            if (rl_handle_tab_completion(&state, prompt) == -1)
                return (rl_error(&state));
        }
        else if (character >= 32 && character <= 126)
        {
            if (rl_handle_printable_char(&state, character, prompt) == -1)
                return (rl_error(&state));
        }
    }
    int line_length = ft_strlen(state.buffer);
    state.buffer[line_length] = '\0';
    rl_update_history(state.buffer);
    rl_disable_raw_mode();
    rl_state_teardown_thread_safety(&state);
    if (DEBUG == 1)
        pf_printf("returning %s\n", state.buffer);
    ft_errno = ER_SUCCESS;
    return (state.buffer);
}
