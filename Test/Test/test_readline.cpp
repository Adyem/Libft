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

static int g_terminal_width_call_count;
static int g_terminal_width_failure_call;
static int g_terminal_width_should_set_errno;
static int g_terminal_width_errno_value;
static int g_raw_mode_override_result;
static int g_raw_mode_should_set_errno;
static int g_raw_mode_errno_value;
static int g_force_strlen_overflow;
static const char *g_strlen_overflow_target;

static int rl_test_strlen_override(const char *string)
{
    if (g_force_strlen_overflow == 1 && string == g_strlen_overflow_target)
    {
        ft_errno = FT_ERANGE;
        g_force_strlen_overflow = 0;
        return (FT_INT_MAX);
    }
    return (ft_strlen(string));
}

int cmp_readline_enable_raw_mode(void)
{
    if (g_raw_mode_override_result == 0)
        return (0);
    if (g_raw_mode_should_set_errno == 1)
        ft_errno = g_raw_mode_errno_value;
    return (g_raw_mode_override_result);
}

void cmp_readline_disable_raw_mode(void)
{
    return ;
}

int cmp_readline_terminal_width(void)
{
    g_terminal_width_call_count++;
    if (g_terminal_width_failure_call != 0
        && g_terminal_width_call_count == g_terminal_width_failure_call)
    {
        if (g_terminal_width_should_set_errno == 1)
            ft_errno = g_terminal_width_errno_value;
        return (-1);
    }
    return (80);
}

FT_TEST(test_readline_enable_raw_mode_propagates_errno, "rl_enable_raw_mode forwards helper error codes")
{
    int result;

    ft_errno = ER_SUCCESS;
    g_raw_mode_override_result = -1;
    g_raw_mode_should_set_errno = 1;
    g_raw_mode_errno_value = ERRNO_OFFSET + 123;
    result = rl_enable_raw_mode();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(g_raw_mode_errno_value, ft_errno);
    g_raw_mode_override_result = 0;
    g_raw_mode_should_set_errno = 0;
    g_raw_mode_errno_value = 0;
    return (1);
}

FT_TEST(test_readline_enable_raw_mode_sets_default_errno, "rl_enable_raw_mode assigns fallback error when helper is silent")
{
    int result;

    ft_errno = ER_SUCCESS;
    g_raw_mode_override_result = -1;
    g_raw_mode_should_set_errno = 0;
    g_raw_mode_errno_value = 0;
    result = rl_enable_raw_mode();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(FT_ETERM, ft_errno);
    g_raw_mode_override_result = 0;
    g_raw_mode_should_set_errno = 0;
    g_raw_mode_errno_value = 0;
    return (1);
}

FT_TEST(test_readline_terminal_width_propagates_errno, "rl_get_terminal_width forwards helper error codes")
{
    int width;

    ft_errno = ER_SUCCESS;
    g_terminal_width_call_count = 0;
    g_terminal_width_failure_call = 1;
    g_terminal_width_should_set_errno = 1;
    g_terminal_width_errno_value = ERRNO_OFFSET + 456;
    width = rl_get_terminal_width();
    FT_ASSERT_EQ(-1, width);
    FT_ASSERT_EQ(g_terminal_width_errno_value, ft_errno);
    g_terminal_width_failure_call = 0;
    g_terminal_width_call_count = 0;
    g_terminal_width_should_set_errno = 0;
    g_terminal_width_errno_value = 0;
    return (1);
}

FT_TEST(test_readline_terminal_width_sets_default_errno, "rl_get_terminal_width assigns fallback error when helper is silent")
{
    int width;

    ft_errno = ER_SUCCESS;
    g_terminal_width_call_count = 0;
    g_terminal_width_failure_call = 1;
    g_terminal_width_should_set_errno = 0;
    g_terminal_width_errno_value = 0;
    width = rl_get_terminal_width();
    FT_ASSERT_EQ(-1, width);
    FT_ASSERT_EQ(FT_ETERM, ft_errno);
    g_terminal_width_failure_call = 0;
    g_terminal_width_call_count = 0;
    g_terminal_width_should_set_errno = 0;
    g_terminal_width_errno_value = 0;
    return (1);
}

FT_TEST(test_readline_backspace_failure, "ReadLine handles helper failures")
{
    int result;
    int stdin_backup;
    int stdout_backup;
    int pipe_descriptors[2];
    const char input_sequence[3] = {'a', 127, '\n'};
    int original_history;

    result = 1;
    stdin_backup = dup(STDIN_FILENO);
    stdout_backup = dup(STDOUT_FILENO);
    if (stdin_backup < 0 || stdout_backup < 0)
        result = 0;
    else if (pipe(pipe_descriptors) != 0)
        result = 0;
    else
    {
        ssize_t written;

        written = write(pipe_descriptors[1], input_sequence, sizeof(input_sequence));
        if (written != static_cast<ssize_t>(sizeof(input_sequence)))
            result = 0;
        else if (dup2(pipe_descriptors[0], STDIN_FILENO) == -1)
            result = 0;
        else
        {
            g_terminal_width_call_count = 0;
            g_terminal_width_failure_call = 2;
            original_history = history_count;
            char *line = rl_readline("> ");
            if (line != ft_nullptr)
                result = 0;
            else if (history_count != original_history)
                result = 0;
        }
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
    }
    if (stdin_backup >= 0)
    {
        dup2(stdin_backup, STDIN_FILENO);
        close(stdin_backup);
    }
    if (stdout_backup >= 0)
    {
        dup2(stdout_backup, STDOUT_FILENO);
        close(stdout_backup);
    }
    g_terminal_width_failure_call = 0;
    g_terminal_width_call_count = 0;
    if (result == 0)
        return (0);
    return (1);
}

FT_TEST(test_readline_clear_line_null_prompt, "rl_clear_line rejects null prompts")
{
    int clear_result;
    const char *buffer;

    buffer = "";
    ft_errno = ER_SUCCESS;
    g_force_strlen_overflow = 0;
    g_strlen_overflow_target = ft_nullptr;
    rl_set_strlen_override(ft_nullptr);
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
    g_force_strlen_overflow = 0;
    g_strlen_overflow_target = ft_nullptr;
    rl_set_strlen_override(ft_nullptr);
    clear_result = rl_clear_line(prompt, ft_nullptr);
    FT_ASSERT_EQ(-1, clear_result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_readline_clear_line_strlen_overflow, "rl_clear_line preserves ft_errno on length overflow")
{
    int clear_result;
    const char *prompt;
    const char *buffer;

    prompt = "overflow";
    buffer = "";
    ft_errno = ER_SUCCESS;
    g_force_strlen_overflow = 1;
    g_strlen_overflow_target = prompt;
    rl_set_strlen_override(rl_test_strlen_override);
    clear_result = rl_clear_line(prompt, buffer);
    FT_ASSERT_EQ(-1, clear_result);
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    g_strlen_overflow_target = ft_nullptr;
    rl_set_strlen_override(ft_nullptr);
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

FT_TEST(test_readline_initialize_state_raw_mode_failure, "rl_initialize_state propagates raw mode errno")
{
    readline_state_t state;
    int init_result;
    int expected_errno;

    ft_errno = ER_SUCCESS;
    expected_errno = ERRNO_OFFSET + 789;
    g_raw_mode_override_result = -1;
    g_raw_mode_should_set_errno = 1;
    g_raw_mode_errno_value = expected_errno;
    init_result = rl_initialize_state(&state);
    g_raw_mode_override_result = 0;
    g_raw_mode_should_set_errno = 0;
    g_raw_mode_errno_value = 0;
    FT_ASSERT_EQ(1, init_result);
    FT_ASSERT_EQ(expected_errno, ft_errno);
    return (1);
}

FT_TEST(test_readline_initialize_state_allocation_failure, "rl_initialize_state reports allocation failures")
{
    readline_state_t state;
    int init_result;

    ft_errno = ER_SUCCESS;
    g_raw_mode_override_result = 0;
    g_raw_mode_should_set_errno = 0;
    g_raw_mode_errno_value = 0;
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
    g_raw_mode_override_result = 0;
    g_raw_mode_should_set_errno = 0;
    g_raw_mode_errno_value = 0;
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

