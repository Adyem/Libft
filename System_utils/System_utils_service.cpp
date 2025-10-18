#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <process.h>
#else
# include <unistd.h>
# include <sys/stat.h>
#endif

static bool g_su_service_force_no_fork = false;
static t_su_service_signal_handler g_su_service_signal_handler = ft_nullptr;
static void *g_su_service_signal_context = ft_nullptr;

#if defined(_WIN32) || defined(_WIN64)
static bool g_su_service_console_handler_installed = false;
static void (*g_su_service_previous_sigint)(int) = SIG_DFL;
static void (*g_su_service_previous_sigterm)(int) = SIG_DFL;

static BOOL WINAPI su_service_console_handler(DWORD control_type)
{
    int mapped_signal;

    mapped_signal = 0;
    if (!g_su_service_signal_handler)
        return (FALSE);
    if (control_type == CTRL_C_EVENT || control_type == CTRL_BREAK_EVENT)
        mapped_signal = SIGINT;
    else if (control_type == CTRL_CLOSE_EVENT || control_type == CTRL_SHUTDOWN_EVENT || control_type == CTRL_LOGOFF_EVENT)
        mapped_signal = SIGTERM;
    if (mapped_signal == 0)
        return (FALSE);
    g_su_service_signal_handler(mapped_signal, g_su_service_signal_context);
    return (TRUE);
}

static void su_service_c_signal_dispatch(int signal_number)
{
    if (!g_su_service_signal_handler)
        return ;
    g_su_service_signal_handler(signal_number, g_su_service_signal_context);
    return ;
}
#else
static struct sigaction g_su_service_previous_sigint;
static struct sigaction g_su_service_previous_sigterm;
#ifdef SIGHUP
static struct sigaction g_su_service_previous_sighup;
static bool g_su_service_sighup_installed = false;
#endif
static bool g_su_service_sigint_installed = false;
static bool g_su_service_sigterm_installed = false;

static void su_service_signal_dispatch(int signal_number)
{
    if (!g_su_service_signal_handler)
        return ;
    g_su_service_signal_handler(signal_number, g_su_service_signal_context);
    return ;
}
#endif

void    su_service_force_no_fork(bool enable)
{
    g_su_service_force_no_fork = enable;
    return ;
}

static int  su_service_redirect_standard_streams(void)
{
#if defined(_WIN32) || defined(_WIN64)
    const char  *null_path;
#else
    const char  *null_path;
#endif
    FILE        *stream;

#if defined(_WIN32) || defined(_WIN64)
    null_path = "NUL";
#else
    null_path = "/dev/null";
#endif
    stream = std::freopen(null_path, "r", stdin);
    if (!stream)
    {
        ft_set_errno_from_system_error(errno);
        return (-1);
    }
    stream = std::freopen(null_path, "w", stdout);
    if (!stream)
    {
        ft_set_errno_from_system_error(errno);
        return (-1);
    }
    stream = std::freopen(null_path, "w", stderr);
    if (!stream)
    {
        ft_set_errno_from_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static int  su_service_write_pid_file(const char *pid_file_path)
{
    char    buffer[64];
    int     file_descriptor;
    size_t  length;
    ssize_t bytes_written;
    int     close_error;

    if (!pid_file_path)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
#if defined(_WIN32) || defined(_WIN64)
    unsigned long pid_value;

    pid_value = static_cast<unsigned long>(_getpid());
#else
    pid_t pid_value;

    pid_value = getpid();
#endif
    std::memset(buffer, 0, sizeof(buffer));
#if defined(_WIN32) || defined(_WIN64)
    std::snprintf(buffer, sizeof(buffer), "%lu\n", pid_value);
#else
    std::snprintf(buffer, sizeof(buffer), "%ld\n", static_cast<long>(pid_value));
#endif
    length = std::strlen(buffer);
    file_descriptor = su_open(pid_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (-1);
    bytes_written = su_write(file_descriptor, buffer, length);
    if (bytes_written < 0 || static_cast<size_t>(bytes_written) != length)
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = FT_ERR_IO;
        su_close(file_descriptor);
        return (-1);
    }
    close_error = su_close(file_descriptor);
    if (close_error != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

int su_service_daemonize(const char *working_directory, const char *pid_file_path,
                         bool redirect_standard_streams)
{
#if defined(_WIN32) || defined(_WIN64)
    if (working_directory && working_directory[0] != '\0')
    {
        if (!SetCurrentDirectoryA(working_directory))
        {
            ft_set_errno_from_system_error(GetLastError());
            return (-1);
        }
    }
    if (redirect_standard_streams)
    {
        if (su_service_redirect_standard_streams() != 0)
            return (-1);
    }
    if (su_service_write_pid_file(pid_file_path) != 0)
        return (-1);
    if (!FreeConsole())
    {
        DWORD   console_error;

        console_error = GetLastError();
        if (console_error != ERROR_INVALID_HANDLE)
        {
            ft_set_errno_from_system_error(console_error);
            return (-1);
        }
    }
    ft_errno = ER_SUCCESS;
    return (0);
#else
    if (!g_su_service_force_no_fork)
    {
        pid_t process_id;

        process_id = fork();
        if (process_id < 0)
        {
            ft_set_errno_from_system_error(errno);
            return (-1);
        }
        if (process_id > 0)
        {
            ft_errno = ER_SUCCESS;
            return (1);
        }
        if (setsid() < 0)
        {
            ft_set_errno_from_system_error(errno);
            return (-1);
        }
        process_id = fork();
        if (process_id < 0)
        {
            ft_set_errno_from_system_error(errno);
            return (-1);
        }
        if (process_id > 0)
            _exit(0);
    }
    umask(0);
    if (working_directory && working_directory[0] != '\0')
    {
        if (chdir(working_directory) != 0)
        {
            ft_set_errno_from_system_error(errno);
            return (-1);
        }
    }
    if (redirect_standard_streams)
    {
        if (su_service_redirect_standard_streams() != 0)
            return (-1);
    }
    if (su_service_write_pid_file(pid_file_path) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
#endif
}

int su_service_install_signal_handlers(t_su_service_signal_handler handler,
                                       void *user_context)
{
    if (!handler)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    su_service_clear_signal_handlers();
    g_su_service_signal_handler = handler;
    g_su_service_signal_context = user_context;
#if defined(_WIN32) || defined(_WIN64)
    errno = 0;
    g_su_service_previous_sigint = std::signal(SIGINT, su_service_c_signal_dispatch);
    if (g_su_service_previous_sigint == SIG_ERR)
    {
        ft_set_errno_from_system_error(errno);
        g_su_service_signal_handler = ft_nullptr;
        g_su_service_signal_context = ft_nullptr;
        return (-1);
    }
    errno = 0;
    g_su_service_previous_sigterm = std::signal(SIGTERM, su_service_c_signal_dispatch);
    if (g_su_service_previous_sigterm == SIG_ERR)
    {
        std::signal(SIGINT, g_su_service_previous_sigint);
        g_su_service_signal_handler = ft_nullptr;
        g_su_service_signal_context = ft_nullptr;
        ft_set_errno_from_system_error(errno);
        return (-1);
    }
    if (!SetConsoleCtrlHandler(su_service_console_handler, TRUE))
    {
        DWORD   console_error;

        console_error = GetLastError();
        std::signal(SIGINT, g_su_service_previous_sigint);
        std::signal(SIGTERM, g_su_service_previous_sigterm);
        g_su_service_signal_handler = ft_nullptr;
        g_su_service_signal_context = ft_nullptr;
        ft_set_errno_from_system_error(console_error);
        return (-1);
    }
    g_su_service_console_handler_installed = true;
#else
    struct sigaction action;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = su_service_signal_dispatch;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, &g_su_service_previous_sigint) != 0)
    {
        g_su_service_signal_handler = ft_nullptr;
        g_su_service_signal_context = ft_nullptr;
        ft_set_errno_from_system_error(errno);
        return (-1);
    }
    g_su_service_sigint_installed = true;
    if (sigaction(SIGTERM, &action, &g_su_service_previous_sigterm) != 0)
    {
        sigaction(SIGINT, &g_su_service_previous_sigint, ft_nullptr);
        g_su_service_sigint_installed = false;
        g_su_service_signal_handler = ft_nullptr;
        g_su_service_signal_context = ft_nullptr;
        ft_set_errno_from_system_error(errno);
        return (-1);
    }
    g_su_service_sigterm_installed = true;
#ifdef SIGHUP
    if (sigaction(SIGHUP, &action, &g_su_service_previous_sighup) != 0)
    {
        sigaction(SIGTERM, &g_su_service_previous_sigterm, ft_nullptr);
        sigaction(SIGINT, &g_su_service_previous_sigint, ft_nullptr);
        g_su_service_sigterm_installed = false;
        g_su_service_sigint_installed = false;
        g_su_service_signal_handler = ft_nullptr;
        g_su_service_signal_context = ft_nullptr;
        ft_set_errno_from_system_error(errno);
        return (-1);
    }
    g_su_service_sighup_installed = true;
#endif
#endif
    ft_errno = ER_SUCCESS;
    return (0);
}

void    su_service_clear_signal_handlers(void)
{
#if defined(_WIN32) || defined(_WIN64)
    if (g_su_service_console_handler_installed)
    {
        SetConsoleCtrlHandler(su_service_console_handler, FALSE);
        g_su_service_console_handler_installed = false;
    }
    if (g_su_service_previous_sigint != SIG_ERR)
        std::signal(SIGINT, g_su_service_previous_sigint);
    if (g_su_service_previous_sigterm != SIG_ERR)
        std::signal(SIGTERM, g_su_service_previous_sigterm);
    g_su_service_previous_sigint = SIG_DFL;
    g_su_service_previous_sigterm = SIG_DFL;
#else
    if (g_su_service_sigint_installed)
    {
        sigaction(SIGINT, &g_su_service_previous_sigint, ft_nullptr);
        g_su_service_sigint_installed = false;
    }
    if (g_su_service_sigterm_installed)
    {
        sigaction(SIGTERM, &g_su_service_previous_sigterm, ft_nullptr);
        g_su_service_sigterm_installed = false;
    }
#ifdef SIGHUP
    if (g_su_service_sighup_installed)
    {
        sigaction(SIGHUP, &g_su_service_previous_sighup, ft_nullptr);
        g_su_service_sighup_installed = false;
    }
#endif
#endif
    g_su_service_signal_handler = ft_nullptr;
    g_su_service_signal_context = ft_nullptr;
    return ;
}
