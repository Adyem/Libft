#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
# include <process.h>
#else
# include <unistd.h>
#endif

static void remove_file_if_exists(const char *path)
{
    if (!path)
        return ;
    std::remove(path);
    return ;
}

FT_TEST(test_su_service_daemonize_creates_pid_file,
        "su_service_daemonize writes pid file when requested")
{
    char    pid_path[128];
    FILE    *pid_file;
    char    buffer[64];
    char    *line_result;
    long    file_pid;

#if defined(_WIN32) || defined(_WIN64)
    std::snprintf(pid_path, sizeof(pid_path), "service_pid_%lu.pid",
        static_cast<unsigned long>(_getpid()));
#else
    std::snprintf(pid_path, sizeof(pid_path), "service_pid_%ld.pid",
        static_cast<long>(getpid()));
#endif
    remove_file_if_exists(pid_path);
    su_service_force_no_fork(true);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, su_service_daemonize(".", pid_path, false));
    su_service_force_no_fork(false);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    pid_file = std::fopen(pid_path, "r");
    FT_ASSERT(pid_file != ft_nullptr);
    std::memset(buffer, 0, sizeof(buffer));
    line_result = std::fgets(buffer, sizeof(buffer), pid_file);
    std::fclose(pid_file);
    FT_ASSERT(line_result != ft_nullptr);
    file_pid = std::strtol(buffer, ft_nullptr, 10);
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(static_cast<long>(_getpid()), file_pid);
#else
    FT_ASSERT_EQ(static_cast<long>(getpid()), file_pid);
#endif
    remove_file_if_exists(pid_path);
    return (1);
}

FT_TEST(test_su_service_install_signal_handlers_rejects_null,
        "su_service_install_signal_handlers validates handler")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, su_service_install_signal_handlers(ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

static volatile sig_atomic_t g_su_service_signal_value = 0;

static void test_su_service_signal_handler(int signal_number, void *context)
{
    (void)context;
    g_su_service_signal_value = signal_number;
    return ;
}

FT_TEST(test_su_service_install_signal_handlers_dispatches,
        "su_service_install_signal_handlers catches SIGTERM")
{
    g_su_service_signal_value = 0;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, su_service_install_signal_handlers(test_su_service_signal_handler,
        ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    std::raise(SIGTERM);
    FT_ASSERT_EQ(SIGTERM, g_su_service_signal_value);
    su_service_clear_signal_handlers();
    return (1);
}
