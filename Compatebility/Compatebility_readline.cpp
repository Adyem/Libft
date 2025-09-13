#include "compatebility_internal.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
static DWORD g_orig_mode;

int cmp_readline_enable_raw_mode()
{
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
        return (-1);
    DWORD mode;
    if (!GetConsoleMode(handle, &mode))
        return (-1);
    g_orig_mode = mode;
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    if (!SetConsoleMode(handle, mode))
        return (-1);
    return (0);
}

void cmp_readline_disable_raw_mode()
{
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle != INVALID_HANDLE_VALUE)
        SetConsoleMode(handle, g_orig_mode);
    return ;
}

int cmp_readline_terminal_width()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE || !GetConsoleScreenBufferInfo(handle, &info))
        return (-1);
    return (info.srWindow.Right - info.srWindow.Left + 1);
}
#else
# include <termios.h>
# include <unistd.h>
# include <sys/ioctl.h>
static termios g_orig_termios;

int cmp_readline_enable_raw_mode()
{
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &raw) == -1)
        return (-1);
    g_orig_termios = raw;
    raw.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1)
        return (-1);
    return (0);
}

void cmp_readline_disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
    return ;
}

int cmp_readline_terminal_width()
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        return (-1);
    return (ws.ws_col);
}
#endif
