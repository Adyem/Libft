#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../Basic/basic.hpp"
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
    int                 error_code;
    int                 init_error;

    init_error = rl_initialize_state(&state);
    if (init_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(init_error);
        return (ft_nullptr);
    }
    pf_printf("%s", prompt);
    fflush(stdout);
    while (1)
    {
        char character;
        int read_error = rl_read_key(&character);

        if (read_error != FT_ERR_SUCCESSS)
        {
            error_code = read_error;
            rl_error(&state);
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        int key_code = static_cast<unsigned char>(character);
        int custom_result = rl_dispatch_custom_key(&state, prompt, key_code);

        if (custom_result == -1)
        {
            error_code = FT_ERR_INTERNAL;
            rl_error(&state);
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
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
            int backspace_error = rl_handle_backspace(&state, prompt);
            if (backspace_error != FT_ERR_SUCCESSS)
            {
                error_code = backspace_error;
                rl_error(&state);
                ft_global_error_stack_push(error_code);
                return (ft_nullptr);
            }
        }
        else if (character == 27)
        {
            int escape_error = rl_handle_escape_sequence(&state, prompt);
            if (escape_error != FT_ERR_SUCCESSS)
            {
                error_code = escape_error;
                rl_error(&state);
                ft_global_error_stack_push(error_code);
                return (ft_nullptr);
            }
        }
        else if (character == '\t')
        {
            int completion_error = rl_handle_tab_completion(&state, prompt);
            if (completion_error != FT_ERR_SUCCESSS)
            {
                error_code = completion_error;
                rl_error(&state);
                ft_global_error_stack_push(error_code);
                return (ft_nullptr);
            }
        }
        else if (character >= 32 && character <= 126)
        {
            int printable_error = rl_handle_printable_char(&state, character, prompt);
            if (printable_error != FT_ERR_SUCCESSS)
            {
                error_code = printable_error;
                rl_error(&state);
                ft_global_error_stack_push(error_code);
                return (ft_nullptr);
            }
        }
    }
    int line_length = ft_strlen(state.buffer);
    state.buffer[line_length] = '\0';
    rl_update_history(state.buffer);
    rl_disable_raw_mode();
    rl_state_teardown_thread_safety(&state);
    if (DEBUG == 1)
        pf_printf("returning %s\n", state.buffer);
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (state.buffer);
}
