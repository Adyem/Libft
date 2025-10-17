#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
static DWORD g_orig_mode;

static void cmp_set_errno_from_last_error()
{
    DWORD last_error;

    last_error = GetLastError();
    if (last_error == 0)
        ft_errno = FT_ERR_TERMINATED;
    else
        ft_errno = ft_map_system_error(static_cast<int>(last_error));
    return ;
}

int cmp_readline_enable_raw_mode()
{
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        cmp_set_errno_from_last_error();
        return (-1);
    }
    DWORD mode;
    if (!GetConsoleMode(handle, &mode))
    {
        cmp_set_errno_from_last_error();
        return (-1);
    }
    g_orig_mode = mode;
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    if (!SetConsoleMode(handle, mode))
    {
        cmp_set_errno_from_last_error();
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

void cmp_readline_disable_raw_mode()
{
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        cmp_set_errno_from_last_error();
        return ;
    }
    if (!SetConsoleMode(handle, g_orig_mode))
    {
        cmp_set_errno_from_last_error();
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}

int cmp_readline_terminal_width()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        cmp_set_errno_from_last_error();
        return (-1);
    }
    if (!GetConsoleScreenBufferInfo(handle, &info))
    {
        cmp_set_errno_from_last_error();
        return (-1);
    }
    int terminal_width = info.srWindow.Right - info.srWindow.Left + 1;
    ft_errno = ER_SUCCESS;
    return (terminal_width);
}
#else
# include <termios.h>
# include <unistd.h>
# include <sys/ioctl.h>
# include <errno.h>
static termios g_orig_termios;

static void cmp_set_errno_from_errno()
{
    if (errno == 0)
        ft_errno = FT_ERR_TERMINATED;
    else
        ft_errno = ft_map_system_error(errno);
    return ;
}

int cmp_readline_enable_raw_mode()
{
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &raw) == -1)
    {
        cmp_set_errno_from_errno();
        return (-1);
    }
    g_orig_termios = raw;
    raw.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1)
    {
        cmp_set_errno_from_errno();
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

void cmp_readline_disable_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios) == -1)
    {
        cmp_set_errno_from_errno();
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}

int cmp_readline_terminal_width()
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        cmp_set_errno_from_errno();
        return (-1);
    }
    int terminal_width = ws.ws_col;
    ft_errno = ER_SUCCESS;
    return (terminal_width);
}
#endif
