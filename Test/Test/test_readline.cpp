#include <unistd.h>
#include "../../ReadLine/readline.hpp"
#include "../../ReadLine/readline_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

static int g_terminal_width_call_count;
static int g_terminal_width_failure_call;

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

