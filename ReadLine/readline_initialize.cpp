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
    if (state->error_file.get_fd() != -1)
        state->error_file.printf("printing to log file\n");
    return ;
}

int rl_initialize_state(readline_state_t *state)
{
    bool lock_acquired = false;
    bool thread_safety_created = false;
    bool had_mutex = false;
    int  index;
    int  result;

    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    had_mutex = (state->mutex != ft_nullptr);
    result = rl_state_prepare_thread_safety(state);
    if (result != FT_ERR_SUCCESS)
        return (result);
    if (!had_mutex && state->mutex != ft_nullptr)
        thread_safety_created = true;
    if (rl_enable_raw_mode() != 0)
    {
        if (thread_safety_created == true)
            rl_state_teardown_thread_safety(state);
        return (FT_ERR_TERMINATED);
    }
    result = rl_state_lock(state, &lock_acquired);
    if (result != FT_ERR_SUCCESS)
    {
        rl_disable_raw_mode();
        if (thread_safety_created == true)
            rl_state_teardown_thread_safety(state);
        return (result);
    }
    if (state->buffer != ft_nullptr)
    {
        cma_free(state->buffer);
        state->buffer = ft_nullptr;
    }
    state->bufsize = INITIAL_BUFFER_SIZE;
    state->buffer = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(state->bufsize)));
    if (state->buffer == ft_nullptr)
    {
        rl_state_unlock(state, lock_acquired);
        rl_disable_raw_mode();
        if (thread_safety_created == true)
            rl_state_teardown_thread_safety(state);
        return (FT_ERR_NO_MEMORY);
    }
    ft_bzero(state->buffer, static_cast<ft_size_t>(state->bufsize));
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
    return (FT_ERR_SUCCESS);
}
