#include <unistd.h>
#include <cstring>
#include "../../ReadLine/readline.hpp"
#include "../../ReadLine/readline_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/limits.hpp"
#include "../../Libft/libft.hpp"


FT_TEST(test_readline_clear_line_null_prompt, "rl_clear_line rejects null prompts")
{
    int clear_result;
    const char *buffer;

    buffer = "";
    ft_errno = ER_SUCCESS;
    clear_result = rl_clear_line(ft_nullptr, buffer);
    FT_ASSERT_EQ(-1, clear_result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_readline_clear_line_null_buffer, "rl_clear_line rejects null buffers")
{
    int clear_result;
    const char *prompt;

    prompt = "> ";
    ft_errno = ER_SUCCESS;
    clear_result = rl_clear_line(prompt, ft_nullptr);
    FT_ASSERT_EQ(-1, clear_result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}


FT_TEST(test_readline_initialize_state_null_pointer, "rl_initialize_state rejects null state pointers")
{
    int init_result;

    ft_errno = ER_SUCCESS;
    init_result = rl_initialize_state(ft_nullptr);
    FT_ASSERT_EQ(1, init_result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_readline_initialize_state_allocation_failure, "rl_initialize_state reports allocation failures")
{
    readline_state_t state;
    int init_result;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    init_result = rl_initialize_state(&state);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(1, init_result);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_readline_initialize_state_success, "rl_initialize_state sets up the state on success")
{
    readline_state_t state;
    int init_result;
    char *allocated_buffer;
    int buffer_size;
    int buffer_position;
    int buffer_history_index;
    int initialize_errno;

    ft_errno = ER_SUCCESS;
    init_result = rl_initialize_state(&state);
    allocated_buffer = state.buffer;
    buffer_size = state.bufsize;
    buffer_position = state.pos;
    buffer_history_index = state.history_index;
    initialize_errno = ft_errno;
    rl_disable_raw_mode();
    if (allocated_buffer != ft_nullptr)
        cma_free(allocated_buffer);
    FT_ASSERT_EQ(0, init_result);
    FT_ASSERT_EQ(ER_SUCCESS, initialize_errno);
    FT_ASSERT(allocated_buffer != ft_nullptr);
    FT_ASSERT_EQ(INITIAL_BUFFER_SIZE, buffer_size);
    FT_ASSERT_EQ(0, buffer_position);
    FT_ASSERT_EQ(history_count, buffer_history_index);
    return (1);
}

FT_TEST(test_readline_history_recall_resizes_buffer, "history recall grows the active buffer")
{
    readline_state_t state;
    const char *prompt = "> ";
    char *history_entry;
    ssize_t written;
    int stdin_backup;
    int pipe_descriptors[2];
    int result;
    char sequence[2];

    result = 1;
    pipe_descriptors[0] = -1;
    pipe_descriptors[1] = -1;
    stdin_backup = -1;
    sequence[0] = '[';
    sequence[1] = 'A';
    history_entry = cma_strdup("this is a history entry that exceeds the initial buffer size");
    if (!history_entry)
        return (0);
    history[0] = history_entry;
    history_count = 1;
    state.buffer = static_cast<char *>(cma_calloc(4, sizeof(char)));
    if (!state.buffer)
    {
        result = 0;
        goto cleanup;
    }
    state.bufsize = 4;
    state.pos = 0;
    state.prev_buffer_length = 0;
    state.history_index = history_count;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    std::memset(state.current_matches, 0, sizeof(state.current_matches));
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
    if (state.bufsize < static_cast<int>(std::strlen(history_entry) + 1))
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
    history[0] = ft_nullptr;
    history_count = 0;
    if (result == 0)
        return (0);
    return (1);
}

