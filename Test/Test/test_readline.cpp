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
    return (0);
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
        return (-1);
    return (80);
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

