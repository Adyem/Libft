#include "../test_internal.hpp"
#include "../../PThread/mutex.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static int pt_mutex_expect_sigabrt(void (*operation)())
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        operation();
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status) == 0)
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void pt_mutex_initialize_twice_aborts_operation()
{
    pt_mutex mutex_object;

    (void)mutex_object.initialize();
    (void)mutex_object.initialize();
    return ;
}

static void pt_mutex_destroy_uninitialised_aborts_operation()
{
    pt_mutex mutex_object;

    (void)mutex_object.destroy();
    return ;
}

static void pt_mutex_destroy_twice_aborts_operation()
{
    pt_mutex mutex_object;

    (void)mutex_object.initialize();
    (void)mutex_object.destroy();
    (void)mutex_object.destroy();
    return ;
}

static void pt_mutex_lock_uninitialised_aborts_operation()
{
    pt_mutex mutex_object;

    (void)mutex_object.lock();
    return ;
}

static void pt_mutex_uninitialised_destructor_aborts_operation()
{
    alignas(pt_mutex) unsigned char storage[sizeof(pt_mutex)];
    pt_mutex *mutex_pointer;

    std::memset(storage, 0, sizeof(storage));
    mutex_pointer = reinterpret_cast<pt_mutex *>(storage);
    mutex_pointer->~pt_mutex();
    return ;
}

FT_TEST(test_pt_mutex_initialize_destroy_reinitialize_success)
{
    pt_mutex mutex_object;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.destroy());
    return (1);
}

FT_TEST(test_pt_mutex_destroy_busy_then_unlock)
{
    pt_mutex mutex_object;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.lock());
    FT_ASSERT_EQ(FT_ERR_THREAD_BUSY, mutex_object.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.destroy());
    return (1);
}

FT_TEST(test_pt_mutex_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, pt_mutex_expect_sigabrt(pt_mutex_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_pt_mutex_destroy_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, pt_mutex_expect_sigabrt(pt_mutex_destroy_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_pt_mutex_destroy_twice_aborts)
{
    FT_ASSERT_EQ(1, pt_mutex_expect_sigabrt(pt_mutex_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_pt_mutex_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, pt_mutex_expect_sigabrt(pt_mutex_lock_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_pt_mutex_uninitialised_destructor_aborts)
{
    FT_ASSERT_EQ(1, pt_mutex_expect_sigabrt(
        pt_mutex_uninitialised_destructor_aborts_operation));
    return (1);
}
