#include "../test_internal.hpp"
#include "../../PThread/recursive_mutex.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static int pt_recursive_mutex_expect_sigabrt(void (*operation)())
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

static void pt_recursive_mutex_initialize_twice_aborts_operation()
{
    pt_recursive_mutex mutex_object;

    (void)mutex_object.initialize();
    (void)mutex_object.initialize();
    return ;
}

static void pt_recursive_mutex_destroy_uninitialized_aborts_operation()
{
    pt_recursive_mutex mutex_object;

    (void)mutex_object.destroy();
    return ;
}

static void pt_recursive_mutex_destroy_twice_aborts_operation()
{
    pt_recursive_mutex mutex_object;

    (void)mutex_object.initialize();
    (void)mutex_object.destroy();
    (void)mutex_object.destroy();
    return ;
}

static void pt_recursive_mutex_lock_uninitialized_aborts_operation()
{
    pt_recursive_mutex mutex_object;

    (void)mutex_object.lock();
    return ;
}

static void pt_recursive_mutex_uninitialized_destructor_aborts_operation()
{
    alignas(pt_recursive_mutex) unsigned char storage[sizeof(pt_recursive_mutex)];
    pt_recursive_mutex *mutex_pointer;

    std::memset(storage, 0, sizeof(storage));
    mutex_pointer = reinterpret_cast<pt_recursive_mutex *>(storage);
    mutex_pointer->~pt_recursive_mutex();
    return ;
}

FT_TEST(test_pt_recursive_mutex_initialize_destroy_reinitialize_success,
    "pt_recursive_mutex initialize and destroy can be repeated safely")
{
    pt_recursive_mutex mutex_object;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.destroy());
    return (1);
}

FT_TEST(test_pt_recursive_mutex_recursive_lock_then_destroy_busy,
    "pt_recursive_mutex destroy returns busy while recursive lock depth is active")
{
    pt_recursive_mutex mutex_object;
    pt_thread_id_type current_thread_id;

    current_thread_id = pt_thread_self();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.try_lock(current_thread_id));
    FT_ASSERT_EQ(FT_ERR_THREAD_BUSY, mutex_object.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_object.destroy());
    return (1);
}

FT_TEST(test_pt_recursive_mutex_initialize_twice_aborts,
    "pt_recursive_mutex initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, pt_recursive_mutex_expect_sigabrt(
        pt_recursive_mutex_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_pt_recursive_mutex_destroy_uninitialized_aborts,
    "pt_recursive_mutex destroy aborts when called on uninitialized object")
{
    FT_ASSERT_EQ(1, pt_recursive_mutex_expect_sigabrt(
        pt_recursive_mutex_destroy_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_pt_recursive_mutex_destroy_twice_aborts,
    "pt_recursive_mutex destroy aborts when called on destroyed object")
{
    FT_ASSERT_EQ(1, pt_recursive_mutex_expect_sigabrt(
        pt_recursive_mutex_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_pt_recursive_mutex_lock_uninitialized_aborts,
    "pt_recursive_mutex lock aborts when called on uninitialized object")
{
    FT_ASSERT_EQ(1, pt_recursive_mutex_expect_sigabrt(
        pt_recursive_mutex_lock_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_pt_recursive_mutex_uninitialized_destructor_aborts,
    "pt_recursive_mutex destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, pt_recursive_mutex_expect_sigabrt(
        pt_recursive_mutex_uninitialized_destructor_aborts_operation));
    return (1);
}
