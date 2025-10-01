#include "../../System_utils/test_runner.hpp"
#include "../../Time/time.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cerrno>
#include <chrono>
#include <ctime>
#include <dlfcn.h>

static bool g_time_now_force_failure = false;
static int g_time_now_forced_errno = EINVAL;

static void time_now_set_force_failure(bool enable_failure, int error_code)
{
    g_time_now_force_failure = enable_failure;
    g_time_now_forced_errno = error_code;
    return ;
}

extern "C" std::time_t time(std::time_t *time_value)
{
    typedef std::time_t (*time_function_pointer)(std::time_t *);
    static time_function_pointer real_time_function = ft_nullptr;

    if (g_time_now_force_failure)
    {
        errno = g_time_now_forced_errno;
        if (time_value != ft_nullptr)
            *time_value = static_cast<std::time_t>(-1);
        return (static_cast<std::time_t>(-1));
    }
    if (real_time_function == ft_nullptr)
    {
        void *symbol_pointer;

        symbol_pointer = dlsym(RTLD_NEXT, "time");
        if (symbol_pointer != ft_nullptr)
            real_time_function = reinterpret_cast<time_function_pointer>(symbol_pointer);
    }
    if (real_time_function != ft_nullptr)
        return (real_time_function(time_value));
    errno = 0;
    std::chrono::system_clock::time_point current_time_point = std::chrono::system_clock::now();
    std::time_t fallback_time = std::chrono::system_clock::to_time_t(current_time_point);
    if (time_value != ft_nullptr)
        *time_value = fallback_time;
    return (fallback_time);
}

FT_TEST(test_time_now_success_resets_errno, "time_now success resets ft_errno")
{
    t_time time_value;

    time_now_set_force_failure(false, 0);
    errno = 0;
    ft_errno = FT_EINVAL;
    time_value = time_now();
    FT_ASSERT(time_value != static_cast<t_time>(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_now_ms_success_resets_errno, "time_now_ms success resets ft_errno")
{
    long milliseconds_value;

    time_now_set_force_failure(false, 0);
    errno = 0;
    ft_errno = FT_EINVAL;
    milliseconds_value = time_now_ms();
    FT_ASSERT(milliseconds_value != static_cast<long>(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_now_failure_sets_errno, "time_now failure propagates errno")
{
    t_time time_value;
    int error_code;

    error_code = EACCES;
    time_now_set_force_failure(true, error_code);
    errno = 0;
    ft_errno = ER_SUCCESS;
    time_value = time_now();
    time_now_set_force_failure(false, 0);
    FT_ASSERT_EQ(static_cast<t_time>(-1), time_value);
    FT_ASSERT_EQ(error_code, ft_errno);
    return (1);
}
