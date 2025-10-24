#include <stdlib.h>
#include <unistd.h>
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "readline_internal.hpp"

static void rl_open_log_file(readline_state_t *state)
{
    static int file_reset;

    if (file_reset == 0 && DEBUG == 1)
    {
        state->error_file.open("data/data--log", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        file_reset = 1;
    }
    else if (DEBUG == 1)
        state->error_file.open("data/data--log", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (state->error_file != -1)
        state->error_file.printf("printing to log file\n");
    return ;
}

int rl_initialize_state(readline_state_t *state)
{
    bool lock_acquired;
    bool thread_safety_created;
    bool thread_safety_was_enabled;
    int  index;

    if (state == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    lock_acquired = false;
    thread_safety_created = false;
    thread_safety_was_enabled = false;
    if (state->thread_safe_enabled == true && state->mutex != ft_nullptr)
        thread_safety_was_enabled = true;
    if (rl_state_prepare_thread_safety(state) != 0)
        return (1);
    if (thread_safety_was_enabled == false && state->thread_safe_enabled == true
        && state->mutex != ft_nullptr)
        thread_safety_created = true;
    if (rl_enable_raw_mode() == -1)
    {
        if (thread_safety_created == true)
            rl_state_teardown_thread_safety(state);
        return (1);
    }
    if (rl_state_lock(state, &lock_acquired) != 0)
    {
        rl_disable_raw_mode();
        if (thread_safety_created == true)
            rl_state_teardown_thread_safety(state);
        return (1);
    }
    if (state->buffer != ft_nullptr)
    {
        cma_free(state->buffer);
        state->buffer = ft_nullptr;
    }
    state->bufsize = INITIAL_BUFFER_SIZE;
    state->buffer = static_cast<char *>(cma_calloc(state->bufsize, sizeof(char)));
    if (!state->buffer)
    {
        rl_state_unlock(state, lock_acquired);
        rl_disable_raw_mode();
        if (thread_safety_created == true)
            rl_state_teardown_thread_safety(state);
        ft_errno = FT_ERR_NO_MEMORY;
        return (1);
    }
    state->pos = 0;
    state->prev_buffer_length = 0;
    state->display_pos = 0;
    state->prev_display_columns = 0;
    state->history_index = history_count;
    state->in_completion_mode = 0;
    state->current_match_count = 0;
    state->current_match_index = 0;
    state->word_start = 0;
    index = 0;
    while (index < MAX_SUGGESTIONS)
    {
        state->current_matches[index] = ft_nullptr;
        index++;
    }
    rl_open_log_file(state);
    rl_state_unlock(state, lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}
