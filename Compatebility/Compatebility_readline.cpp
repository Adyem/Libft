#include "compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
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

int cmp_readline_terminal_dimensions(unsigned short *rows, unsigned short *cols,
    unsigned short *x_pixels, unsigned short *y_pixels)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE handle;
    unsigned short computed_rows;
    unsigned short computed_columns;

    handle = GetStdHandle(STD_OUTPUT_HANDLE);
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
    computed_rows = static_cast<unsigned short>(info.srWindow.Bottom - info.srWindow.Top + 1);
    computed_columns = static_cast<unsigned short>(info.srWindow.Right - info.srWindow.Left + 1);
    if (rows != ft_nullptr)
        *rows = computed_rows;
    if (cols != ft_nullptr)
        *cols = computed_columns;
    if (x_pixels != ft_nullptr)
        *x_pixels = 0;
    if (y_pixels != ft_nullptr)
        *y_pixels = 0;
    ft_errno = ER_SUCCESS;
    return (0);
}

int cmp_readline_terminal_width()
{
    unsigned short rows;
    unsigned short cols;

    if (cmp_readline_terminal_dimensions(&rows, &cols, ft_nullptr, ft_nullptr) != 0)
        return (-1);
    return (static_cast<int>(cols));
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

int cmp_readline_terminal_dimensions(unsigned short *rows, unsigned short *cols,
    unsigned short *x_pixels, unsigned short *y_pixels)
{
    struct winsize window_size;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1)
    {
        cmp_set_errno_from_errno();
        return (-1);
    }
    if (rows != ft_nullptr)
        *rows = window_size.ws_row;
    if (cols != ft_nullptr)
        *cols = window_size.ws_col;
    if (x_pixels != ft_nullptr)
        *x_pixels = window_size.ws_xpixel;
    if (y_pixels != ft_nullptr)
        *y_pixels = window_size.ws_ypixel;
    ft_errno = ER_SUCCESS;
    return (0);
}

int cmp_readline_terminal_width()
{
    unsigned short rows;
    unsigned short cols;

    if (cmp_readline_terminal_dimensions(&rows, &cols, ft_nullptr, ft_nullptr) != 0)
        return (-1);
    return (static_cast<int>(cols));
}
#endif
