#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#endif

#include "readline_internal.hpp"

#ifdef _WIN32
static DWORD orig_mode;
#endif

#ifndef _WIN32
termios	orig_termios;
#endif

static inline void disable_raw_mode_platform()
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin != INVALID_HANDLE_VALUE)
        SetConsoleMode(hStdin, orig_mode);
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
#endif
    return ;
}

void rl_disable_raw_mode()
{
    disable_raw_mode_platform();
    return ;
}

static inline int enable_raw_mode_platform()
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        return (-1);

    DWORD mode;
    if (!GetConsoleMode(hStdin, &mode))
        return (-1);
    orig_mode = mode;
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    if (!SetConsoleMode(hStdin, mode))
        return (-1);
    return (0);
#else
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &raw) == -1)
        return (-1);

    orig_termios = raw;
    raw.c_lflag &= ~(ECHO | ICANON);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1)
        return (-1);

    return (0);
#endif
}

int rl_enable_raw_mode()
{
    return (enable_raw_mode_platform());
}
