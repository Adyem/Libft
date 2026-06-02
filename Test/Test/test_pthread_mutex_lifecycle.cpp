#include "../test_internal.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <cstring>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static pt_mutex g_pt_mutex_abort_object;

static int pt_mutex_expect_sigabrt(void (*operation)())
{
    return (test_expect_sigabrt_signal(operation));
}

static void pt_mutex_initialize_twice_aborts_operation()
{
    (void)g_pt_mutex_abort_object.initialize();
    (void)g_pt_mutex_abort_object.initialize();
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_pt_mutex_abort_object.destroy());
    return (1);
}

FT_TEST(test_pt_mutex_destroy_uninitialised_succeeds)
{
    FT_ASSERT_EQ(0, pt_mutex_expect_sigabrt(pt_mutex_destroy_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_pt_mutex_destroy_twice_succeeds)
{
    FT_ASSERT_EQ(0, pt_mutex_expect_sigabrt(pt_mutex_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_pt_mutex_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, pt_mutex_expect_sigabrt(pt_mutex_lock_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_pt_mutex_uninitialised_destructor_succeeds)
{
    FT_ASSERT_EQ(0, pt_mutex_expect_sigabrt(
        pt_mutex_uninitialised_destructor_aborts_operation));
    return (1);
}
