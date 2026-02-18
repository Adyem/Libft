#include "../test_internal.hpp"
#include "test_scma_shared.hpp"
#include <csetjmp>
#include <csignal>
#include <cstring>
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static sigjmp_buf g_scma_accessor_abort_jump;

static void scma_accessor_abort_handler(int /*signal*/)
{
    siglongjmp(g_scma_accessor_abort_jump, 1);
}

static int scma_accessor_expect_sigabrt(void (*operation)())
{
    struct sigaction action;
    struct sigaction backup;
    memset(&action, 0, sizeof(action));
    action.sa_handler = scma_accessor_abort_handler;
    sigemptyset(&action.sa_mask);
    int result = 0;

    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_scma_accessor_abort_jump, 1) == 0)
    {
        operation();
        result = 0;
    }
    else
    {
        result = 1;
    }
    sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

static void scma_accessor_initialize_twice_aborts_operation()
{
    scma_handle_accessor<int> accessor;

    (void)scma_test_initialize(64);
    (void)accessor.initialize();
    (void)accessor.initialize();
    return ;
}

static void scma_accessor_destroy_twice_aborts_operation()
{
    scma_handle_accessor<int> accessor;

    (void)scma_test_initialize(64);
    (void)accessor.initialize();
    (void)accessor.destroy();
    (void)accessor.destroy();
    return ;
}

static void scma_accessor_is_bound_on_destroyed_aborts_operation()
{
    scma_handle_accessor<int> accessor;

    (void)scma_test_initialize(64);
    (void)accessor.initialize();
    (void)accessor.destroy();
    (void)accessor.is_bound();
    return ;
}

static void scma_accessor_copy_ctor_from_destroyed_source_aborts_operation()
{
    scma_handle_accessor<int> source_accessor;

    (void)scma_test_initialize(64);
    (void)source_accessor.initialize();
    (void)source_accessor.destroy();
    scma_handle_accessor<int> copy_accessor(source_accessor);
    (void)copy_accessor;
    return ;
}

static void scma_accessor_move_ctor_from_destroyed_source_aborts_operation()
{
    scma_handle_accessor<int> source_accessor;

    (void)scma_test_initialize(64);
    (void)source_accessor.initialize();
    (void)source_accessor.destroy();
    scma_handle_accessor<int> moved_accessor(
        static_cast<scma_handle_accessor<int> &&>(source_accessor));
    (void)moved_accessor;
    return ;
}

static void scma_accessor_copy_ctor_from_uninitialized_source_aborts_operation()
{
    scma_handle_accessor<int> source_accessor;
    scma_handle_accessor<int> copy_accessor(source_accessor);

    (void)copy_accessor;
    return ;
}

static void scma_accessor_move_ctor_from_uninitialized_source_aborts_operation()
{
    scma_handle_accessor<int> source_accessor;
    scma_handle_accessor<int> moved_accessor(
        static_cast<scma_handle_accessor<int> &&>(source_accessor));

    (void)moved_accessor;
    return ;
}

FT_TEST(test_scma_accessor_initialize_destroy_reinitialize_success,
    "scma accessor supports initialize/destroy/reinitialize lifecycle")
{
    scma_handle_accessor<int> accessor;

    FT_ASSERT_EQ(1, scma_test_initialize(64));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(1, accessor.is_initialized());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(1, accessor.is_initialized());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_initialize_with_handle_binds,
    "scma accessor initialize(handle) binds to a valid handle")
{
    scma_handle_accessor<int> accessor;
    scma_handle handle;

    FT_ASSERT_EQ(1, scma_test_initialize(128));
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize(handle));
    FT_ASSERT_EQ(1, accessor.is_initialized());
    FT_ASSERT_EQ(1, accessor.is_bound());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    FT_ASSERT_EQ(1, scma_free(handle));
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_initialize_invalid_handle_sets_error_and_unbinds,
    "scma accessor initialize(handle) with invalid handle fails cleanly")
{
    scma_handle_accessor<int> accessor;
    scma_handle invalid_handle;

    FT_ASSERT_EQ(1, scma_test_initialize(64));
    invalid_handle.index = FT_SYSTEM_SIZE_MAX;
    invalid_handle.generation = FT_SYSTEM_SIZE_MAX;
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, accessor.initialize(invalid_handle));
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, accessor.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(0, accessor.is_bound());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_initialize_twice_aborts,
    "scma accessor initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(scma_accessor_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_destroy_twice_aborts,
    "scma accessor destroy aborts when called after destroy")
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(scma_accessor_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_is_bound_after_destroy_aborts,
    "scma accessor methods abort when called after destroy")
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(scma_accessor_is_bound_on_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_copy_ctor_from_destroyed_source_aborts,
    "scma accessor copy constructor aborts when source is destroyed")
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(
        scma_accessor_copy_ctor_from_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_move_ctor_from_destroyed_source_aborts,
    "scma accessor move constructor aborts when source is destroyed")
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(
        scma_accessor_move_ctor_from_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_copy_ctor_from_uninitialized_source_aborts,
    "scma accessor copy constructor aborts when source is uninitialized")
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(
        scma_accessor_copy_ctor_from_uninitialized_source_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_move_ctor_from_uninitialized_source_aborts,
    "scma accessor move constructor aborts when source is uninitialized")
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(
        scma_accessor_move_ctor_from_uninitialized_source_aborts_operation));
    return (1);
}
