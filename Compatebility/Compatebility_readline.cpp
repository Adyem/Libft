#include "compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
static DWORD g_orig_mode;
static bool g_raw_mode_active;

static int32_t cmp_map_last_error_to_ft_error()
{
    DWORD last_error;
    int32_t error_code;

    last_error = GetLastError();
    if (last_error == 0)
        error_code = FT_ERR_TERMINATED;
    else
        error_code = cmp_map_system_error_to_ft(static_cast<int32_t>(last_error));
    return (error_code);
}

int32_t cmp_readline_enable_raw_mode()
{
    HANDLE handle;
    DWORD mode;

    handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        int32_t error_code = cmp_map_last_error_to_ft_error();

        if (error_code == FT_ERR_INVALID_HANDLE)
        {
            g_raw_mode_active = false;
            return (FT_ERR_SUCCESS);
        }
        return (error_code);
    }
    if (!GetConsoleMode(handle, &mode))
    {
        int32_t error_code = cmp_map_last_error_to_ft_error();

        if (error_code == FT_ERR_INVALID_HANDLE)
        {
            g_raw_mode_active = false;
            return (FT_ERR_SUCCESS);
        }
        return (error_code);
    }
    g_orig_mode = mode;
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    if (!SetConsoleMode(handle, mode))
    {
        int32_t error_code = cmp_map_last_error_to_ft_error();

        if (error_code == FT_ERR_INVALID_HANDLE)
        {
            g_raw_mode_active = false;
            return (FT_ERR_SUCCESS);
        }
        return (error_code);
    }
    g_raw_mode_active = true;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_readline_disable_raw_mode()
{
    HANDLE handle;

    if (g_raw_mode_active == false)
    {
        return (FT_ERR_SUCCESS);
    }
    handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return (cmp_map_last_error_to_ft_error());
    }
    if (!SetConsoleMode(handle, g_orig_mode))
    {
        return (cmp_map_last_error_to_ft_error());
    }
    g_raw_mode_active = false;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_readline_terminal_dimensions(unsigned short *rows, unsigned short *cols,
    unsigned short *x_pixels, unsigned short *y_pixels)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE handle;
    unsigned short computed_rows;
    unsigned short computed_columns;

    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return (cmp_map_last_error_to_ft_error());
    }
    if (!GetConsoleScreenBufferInfo(handle, &info))
    {
        return (cmp_map_last_error_to_ft_error());
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
    return (FT_ERR_SUCCESS);
}

int32_t cmp_readline_terminal_width(int32_t *width_out)
{
    unsigned short rows;
    unsigned short cols;
    int32_t error_code;

    if (width_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    *width_out = 0;
    error_code = cmp_readline_terminal_dimensions(&rows, &cols, ft_nullptr,
            ft_nullptr);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    *width_out = static_cast<int32_t>(cols);
    return (FT_ERR_SUCCESS);
}
#else
# include <termios.h>
# include <unistd.h>
# include <sys/ioctl.h>
# include <errno.h>
static termios g_orig_termios;
static bool g_raw_mode_active;

static int32_t cmp_map_errno_to_ft_error()
{
    if (errno == 0)
        return (FT_ERR_TERMINATED);
    return (cmp_map_system_error_to_ft(errno));
}

int32_t cmp_readline_enable_raw_mode()
{
    struct termios raw;

    if (isatty(STDIN_FILENO) == 0)
    {
        g_raw_mode_active = false;
        return (FT_ERR_SUCCESS);
    }
    if (tcgetattr(STDIN_FILENO, &raw) == -1)
    {
        return (cmp_map_errno_to_ft_error());
    }
    g_orig_termios = raw;
    raw.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1)
    {
        return (cmp_map_errno_to_ft_error());
    }
    g_raw_mode_active = true;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_readline_disable_raw_mode()
{
    if (g_raw_mode_active == false)
        return (FT_ERR_SUCCESS);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios) == -1)
    {
        return (cmp_map_errno_to_ft_error());
    }
    g_raw_mode_active = false;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_readline_terminal_dimensions(unsigned short *rows, unsigned short *cols,
    unsigned short *x_pixels, unsigned short *y_pixels)
{
    struct winsize window_size;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1)
        return (cmp_map_errno_to_ft_error());
    if (rows != ft_nullptr)
        *rows = window_size.ws_row;
    if (cols != ft_nullptr)
        *cols = window_size.ws_col;
    if (x_pixels != ft_nullptr)
        *x_pixels = window_size.ws_xpixel;
    if (y_pixels != ft_nullptr)
        *y_pixels = window_size.ws_ypixel;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_readline_terminal_width(int32_t *width_out)
{
    unsigned short rows;
    unsigned short cols;
    int32_t error_code;

    if (width_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    *width_out = 0;
    error_code = cmp_readline_terminal_dimensions(&rows, &cols, ft_nullptr,
            ft_nullptr);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    *width_out = static_cast<int32_t>(cols);
    return (FT_ERR_SUCCESS);
}
#endif
