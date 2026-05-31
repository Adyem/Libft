#include "../test_internal.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include "../../Modules/ReadLine/readline.hpp"
#include "../../Modules/ReadLine/readline_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Advanced/advanced.hpp"

static void test_readline_cleanup_state(readline_state_t *state)
{
    ft_bool lock_acquired;
    int32_t lock_result;

    if (state == ft_nullptr)
        return ;
    lock_acquired = FT_FALSE;
    lock_result = rl_state_lock(state, &lock_acquired);
    if (lock_result == 0 && state->buffer != ft_nullptr)
    {
        cma_free(state->buffer);
        state->buffer = ft_nullptr;
    }
    if (lock_result == 0)
    {
        if (lock_acquired == FT_TRUE)
            rl_state_unlock(state, lock_acquired);
    }
    rl_state_teardown_thread_safety(state);
    return ;
}

FT_TEST(test_readline_history_recall_resizes_buffer)
{
    readline_state_t state;
    const char *prompt = "> ";
    char *history_entry;
    char *history_backup[MAX_HISTORY];
    ssize_t written;
    int stdin_backup;
    int pipe_descriptors[2];
    int result;
    char sequence[2];
    int32_t history_count_backup;
    int32_t backup_index;

    result = 1;
    pipe_descriptors[0] = -1;
    pipe_descriptors[1] = -1;
    stdin_backup = -1;
    sequence[0] = '[';
    sequence[1] = 'A';
    backup_index = 0;
    history_count_backup = history_count;
    while (backup_index < MAX_HISTORY)
    {
        history_backup[backup_index] = history[backup_index];
        backup_index += 1;
    }
    history_entry = adv_strdup("this is a history entry that exceeds the initial buffer size");
    if (!history_entry)
        return (0);
    history[0] = history_entry;
    history_count = 1;
    state.buffer = static_cast<char *>(cma_malloc(4));
    if (!state.buffer)
    {
        result = 0;
        goto cleanup;
    }
    state.buffer_size = 4;
    state.position = 0;
    state.prev_buffer_length = 0;
    state.display_pos = 0;
    state.prev_display_columns = 0;
    state.history_index = history_count;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    std::memset(state.current_matches, 0, sizeof(state.current_matches));
    std::memset(state.current_match_scores, 0, sizeof(state.current_match_scores));
    stdin_backup = dup(STDIN_FILENO);
    if (stdin_backup < 0)
    {
        result = 0;
        goto cleanup;
    }
    if (pipe(pipe_descriptors) != 0)
    {
        result = 0;
        goto cleanup;
    }
    if (dup2(pipe_descriptors[0], STDIN_FILENO) == -1)
    {
        result = 0;
        goto cleanup;
    }
    written = write(pipe_descriptors[1], sequence, sizeof(sequence));
    if (written != static_cast<ssize_t>(sizeof(sequence)))
    {
        result = 0;
        goto cleanup;
    }
    if (rl_handle_escape_sequence(&state, prompt) == -1)
    {
        result = 0;
        goto cleanup;
    }
    if (std::strcmp(state.buffer, history_entry) != 0)
    {
        result = 0;
        goto cleanup;
    }
    if (state.buffer_size < static_cast<int>(std::strlen(history_entry) + 1))
    {
        result = 0;
        goto cleanup;
    }
cleanup:
    if (pipe_descriptors[0] != -1)
        close(pipe_descriptors[0]);
    if (pipe_descriptors[1] != -1)
        close(pipe_descriptors[1]);
    if (stdin_backup != -1)
    {
        dup2(stdin_backup, STDIN_FILENO);
        close(stdin_backup);
    }
    if (state.buffer)
        cma_free(state.buffer);
    if (history_entry)
        cma_free(history_entry);
    backup_index = 0;
    while (backup_index < MAX_HISTORY)
    {
        history[backup_index] = history_backup[backup_index];
        backup_index += 1;
    }
    history_count = history_count_backup;
    if (result == 0)
        return (0);
    return (1);
}

FT_TEST(test_readline_reverse_history_search_dispatches_ctrl_r_binding)
{
    readline_state_t state;
    char *history_backup[MAX_HISTORY];
    int32_t history_count_backup;
    int initialize_result;
    int dispatch_result;
    int stdin_backup;
    int pipe_descriptors[2];
    ssize_t written;
    ft_bool key_handled;
    int32_t backup_index;

    backup_index = 0;
    history_count_backup = history_count;
    while (backup_index < MAX_HISTORY)
    {
        history_backup[backup_index] = history[backup_index];
        backup_index += 1;
    }
    history[0] = adv_strdup("alpha");
    history[1] = adv_strdup("beta");
    if (history[0] == ft_nullptr || history[1] == ft_nullptr)
        return (0);
    history_count = 2;
    ft_bzero(&state, sizeof(state));
    initialize_result = rl_initialize_state(&state);
    if (initialize_result != FT_ERR_SUCCESS)
        return (0);
    stdin_backup = dup(STDIN_FILENO);
    if (stdin_backup < 0)
    {
        test_readline_cleanup_state(&state);
        cma_free(history[0]);
        cma_free(history[1]);
        backup_index = 0;
        while (backup_index < MAX_HISTORY)
        {
            history[backup_index] = history_backup[backup_index];
            backup_index += 1;
        }
        history_count = history_count_backup;
        return (0);
    }
    pipe_descriptors[0] = -1;
    pipe_descriptors[1] = -1;
    if (pipe(pipe_descriptors) != 0)
    {
        close(stdin_backup);
        test_readline_cleanup_state(&state);
        cma_free(history[0]);
        cma_free(history[1]);
        backup_index = 0;
        while (backup_index < MAX_HISTORY)
        {
            history[backup_index] = history_backup[backup_index];
            backup_index += 1;
        }
        history_count = history_count_backup;
        return (0);
    }
    if (dup2(pipe_descriptors[0], STDIN_FILENO) == -1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        close(stdin_backup);
        test_readline_cleanup_state(&state);
        cma_free(history[0]);
        cma_free(history[1]);
        backup_index = 0;
        while (backup_index < MAX_HISTORY)
        {
            history[backup_index] = history_backup[backup_index];
            backup_index += 1;
        }
        history_count = history_count_backup;
        return (0);
    }
    written = write(pipe_descriptors[1], "be\n", 3);
    close(pipe_descriptors[1]);
    if (written != 3)
    {
        dup2(stdin_backup, STDIN_FILENO);
        close(stdin_backup);
        close(pipe_descriptors[0]);
        test_readline_cleanup_state(&state);
        cma_free(history[0]);
        cma_free(history[1]);
        backup_index = 0;
        while (backup_index < MAX_HISTORY)
        {
            history[backup_index] = history_backup[backup_index];
            backup_index += 1;
        }
        history_count = history_count_backup;
        return (0);
    }
    key_handled = FT_FALSE;
    dispatch_result = rl_dispatch_custom_key(&state, "> ", RL_KEY_CTRL_R, &key_handled);
    dup2(stdin_backup, STDIN_FILENO);
    close(stdin_backup);
    close(pipe_descriptors[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dispatch_result);
    FT_ASSERT_EQ(FT_TRUE, key_handled);
    FT_ASSERT_EQ(0, std::strcmp(state.buffer, "beta"));
    FT_ASSERT_EQ(1, state.history_index);
    test_readline_cleanup_state(&state);
    cma_free(history[0]);
    cma_free(history[1]);
    backup_index = 0;
    while (backup_index < MAX_HISTORY)
    {
        history[backup_index] = history_backup[backup_index];
        backup_index += 1;
    }
    history_count = history_count_backup;
    return (1);
}
