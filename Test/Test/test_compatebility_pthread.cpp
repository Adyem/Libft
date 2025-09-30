#if !defined(_WIN32) && !defined(_WIN64)
# if !defined(_GNU_SOURCE)
#  define _GNU_SOURCE
# endif
#endif

#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#endif

#if !defined(_WIN32) && !defined(_WIN64)
# include <dlfcn.h>
# include <unistd.h>

static bool g_force_sched_yield_failure = false;
static int g_force_sched_yield_errno = EINVAL;

static int call_real_sched_yield(void)
{
    typedef int (*sched_yield_function)(void);
    static sched_yield_function real_sched_yield_pointer = ft_nullptr;
    if (real_sched_yield_pointer == ft_nullptr)
    {
        void *symbol_pointer = dlsym(RTLD_NEXT, "sched_yield");
        real_sched_yield_pointer = reinterpret_cast<sched_yield_function>(symbol_pointer);
    }
    if (real_sched_yield_pointer == ft_nullptr)
        return (0);
    return (real_sched_yield_pointer());
}

extern "C" int sched_yield(void)
{
    if (g_force_sched_yield_failure != false)
    {
        errno = g_force_sched_yield_errno;
        return (-1);
    }
    return (call_real_sched_yield());
}

static void set_force_sched_yield_failure(int errno_value)
{
    g_force_sched_yield_failure = true;
    g_force_sched_yield_errno = errno_value;
    return ;
}

static void clear_force_sched_yield_failure(void)
{
    g_force_sched_yield_failure = false;
    g_force_sched_yield_errno = 0;
    return ;
}

static bool g_force_usleep_failure = false;
static int g_force_usleep_errno = EINTR;

static bool g_force_pthread_cancel_failure = false;
static int g_force_pthread_cancel_errno = ESRCH;

static int call_real_pthread_cancel(pthread_t thread)
{
    typedef int (*pthread_cancel_function)(pthread_t);
    static pthread_cancel_function real_pthread_cancel_pointer = ft_nullptr;
    if (real_pthread_cancel_pointer == ft_nullptr)
    {
        void *symbol_pointer = dlsym(RTLD_NEXT, "pthread_cancel");
        real_pthread_cancel_pointer = reinterpret_cast<pthread_cancel_function>(symbol_pointer);
    }
    if (real_pthread_cancel_pointer == ft_nullptr)
        return (ESRCH);
    return (real_pthread_cancel_pointer(thread));
}

extern "C" int pthread_cancel(pthread_t thread)
{
    if (g_force_pthread_cancel_failure != false)
    {
        return (g_force_pthread_cancel_errno);
    }
    return (call_real_pthread_cancel(thread));
}

static void set_force_pthread_cancel_failure(int error_value)
{
    g_force_pthread_cancel_failure = true;
    g_force_pthread_cancel_errno = error_value;
    return ;
}

static void clear_force_pthread_cancel_failure(void)
{
    g_force_pthread_cancel_failure = false;
    g_force_pthread_cancel_errno = ESRCH;
    return ;
}

static int call_real_usleep(useconds_t microseconds)
{
    typedef int (*usleep_function)(useconds_t);
    static usleep_function real_usleep_pointer = ft_nullptr;
    if (real_usleep_pointer == ft_nullptr)
    {
        void *symbol_pointer = dlsym(RTLD_NEXT, "usleep");
        real_usleep_pointer = reinterpret_cast<usleep_function>(symbol_pointer);
    }
    if (real_usleep_pointer == ft_nullptr)
        return (0);
    return (real_usleep_pointer)(microseconds);
}

extern "C" int usleep(useconds_t microseconds)
{
    if (g_force_usleep_failure != false)
    {
        errno = g_force_usleep_errno;
        return (-1);
    }
    return (call_real_usleep(microseconds));
}

static void set_force_usleep_failure(int errno_value)
{
    g_force_usleep_failure = true;
    g_force_usleep_errno = errno_value;
    return ;
}

static void clear_force_usleep_failure(void)
{
    g_force_usleep_failure = false;
    g_force_usleep_errno = 0;
    return ;
}
#endif

static void *completed_thread_function(void *argument)
{
    (void)argument;
    return (ft_nullptr);
}

static void *cancellable_thread_function(void *argument)
{
    (void)argument;
    while (true)
    {
#if !defined(_WIN32) && !defined(_WIN64)
        pthread_testcancel();
        usleep(1000);
#else
        Sleep(1);
#endif
    }
    return (ft_nullptr);
}

FT_TEST(test_cmp_thread_cancel_failure_sets_ft_errno, "cmp_thread_cancel failure propagates errno")
{
    pthread_t thread_identifier;

    FT_ASSERT_EQ(0, pthread_create(&thread_identifier, ft_nullptr, completed_thread_function, ft_nullptr));
    FT_ASSERT_EQ(0, pthread_join(thread_identifier, ft_nullptr));
    set_force_pthread_cancel_failure(ESRCH);
    ft_errno = ER_SUCCESS;
    int cancel_result = cmp_thread_cancel(thread_identifier);
    clear_force_pthread_cancel_failure();
    FT_ASSERT_EQ(ESRCH, cancel_result);
    FT_ASSERT_EQ(ESRCH + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_cmp_thread_cancel_success_clears_ft_errno, "cmp_thread_cancel success clears ft_errno")
{
    pthread_t thread_identifier;

    FT_ASSERT_EQ(0, pthread_create(&thread_identifier, ft_nullptr, cancellable_thread_function, ft_nullptr));
    ft_errno = FT_EINVAL;
    int cancel_result = cmp_thread_cancel(thread_identifier);
    FT_ASSERT_EQ(0, cancel_result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, pthread_join(thread_identifier, ft_nullptr));
    return (1);
}

#if !defined(_WIN32) && !defined(_WIN64)
FT_TEST(test_cmp_thread_yield_failure_and_success, "cmp_thread_yield reports failure and clears ft_errno on success")
{
    ft_errno = ER_SUCCESS;
    set_force_sched_yield_failure(EAGAIN);
    int yield_result = cmp_thread_yield();
    FT_ASSERT_EQ(-1, yield_result);
    FT_ASSERT_EQ(EAGAIN + ERRNO_OFFSET, ft_errno);
    clear_force_sched_yield_failure();
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, cmp_thread_yield());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cmp_thread_sleep_failure_and_success, "cmp_thread_sleep reports failure and clears ft_errno on success")
{
    ft_errno = ER_SUCCESS;
    set_force_usleep_failure(EINTR);
    int sleep_result = cmp_thread_sleep(1);
    FT_ASSERT_EQ(-1, sleep_result);
    FT_ASSERT_EQ(EINTR + ERRNO_OFFSET, ft_errno);
    clear_force_usleep_failure();
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, cmp_thread_sleep(1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
#else
FT_TEST(test_cmp_thread_yield_success_clears_ft_errno, "cmp_thread_yield clears ft_errno on success")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, cmp_thread_yield());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cmp_thread_sleep_success_clears_ft_errno, "cmp_thread_sleep clears ft_errno on success")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, cmp_thread_sleep(1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
#endif
