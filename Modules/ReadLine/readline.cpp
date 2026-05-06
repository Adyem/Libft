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
int32_t        history_count = 0;
char    *suggestions[MAX_SUGGESTIONS];
int32_t        suggestion_count = 0;

static void rl_cleanup_state(readline_state_t *state)
{
    ft_bool lock_acquired;

    if (state == ft_nullptr)
        return ;
    lock_acquired = FT_FALSE;
    if (rl_state_lock(state, &lock_acquired) == FT_ERR_SUCCESS)
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
    int32_t                 init_error;

    init_error = rl_initialize_state(&state);
    if (init_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    pf_printf("%s", prompt);
    fflush(stdout);
    while (1)
    {
        char character;
        int32_t read_error = rl_read_key(&character);
        ft_bool custom_key_handled;

        if (read_error != FT_ERR_SUCCESS)
        {
            rl_error(&state);
            return (ft_nullptr);
        }
        int32_t key_code = static_cast<uint8_t>(character);
        int32_t custom_result;

        custom_key_handled = FT_FALSE;
        custom_result = rl_dispatch_custom_key(&state, prompt, key_code, &custom_key_handled);

        if (custom_result != FT_ERR_SUCCESS)
        {
            rl_error(&state);
            return (ft_nullptr);
        }
        if (custom_key_handled == FT_TRUE)
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
            int32_t backspace_error = rl_handle_backspace(&state, prompt);
            if (backspace_error != FT_ERR_SUCCESS)
            {
                rl_error(&state);
                return (ft_nullptr);
            }
        }
        else if (character == 27)
        {
            int32_t escape_error = rl_handle_escape_sequence(&state, prompt);
            if (escape_error != FT_ERR_SUCCESS)
            {
                rl_error(&state);
                return (ft_nullptr);
            }
        }
        else if (character == '\t')
        {
            int32_t completion_error = rl_handle_tab_completion(&state, prompt);
            if (completion_error != FT_ERR_SUCCESS)
            {
                rl_error(&state);
                return (ft_nullptr);
            }
        }
        else if (character >= 32 && character <= 126)
        {
            int32_t printable_error = rl_handle_printable_char(&state, character, prompt);
            if (printable_error != FT_ERR_SUCCESS)
            {
                rl_error(&state);
                return (ft_nullptr);
            }
        }
    }
    int32_t line_length = ft_strlen(state.buffer);
    state.buffer[line_length] = '\0';
    rl_update_history(state.buffer);
    rl_disable_raw_mode();
    rl_state_teardown_thread_safety(&state);
    if (DEBUG == 1)
        pf_printf("returning %s\n", state.buffer);
    return (state.buffer);
}
