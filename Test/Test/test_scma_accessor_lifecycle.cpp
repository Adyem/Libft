#include "../test_internal.hpp"
#include "test_scma_shared.hpp"
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <type_traits>
#include "../../Modules/Basic/class_nullptr.hpp"

#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/SCMA/SCMA.hpp"
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

    if (scma_test_initialize(64) != 0)
        return ;
    if (accessor.initialize() != FT_ERR_SUCCESS)
        return ;
    accessor.initialize();
    return ;
}

static void scma_accessor_destroy_twice_aborts_operation()
{
    scma_handle_accessor<int> accessor;

    if (scma_test_initialize(64) != 0)
        return ;
    if (accessor.initialize() != FT_ERR_SUCCESS)
        return ;
    if (accessor.destroy() != FT_ERR_SUCCESS)
        return ;
    accessor.destroy();
    return ;
}

static void scma_accessor_is_bound_on_destroyed_aborts_operation()
{
    scma_handle_accessor<int> accessor;

    if (scma_test_initialize(64) != 0)
        return ;
    if (accessor.initialize() != FT_ERR_SUCCESS)
        return ;
    if (accessor.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)accessor.is_bound();
    return ;
}

FT_TEST(test_scma_accessor_initialize_destroy_reinitialize_success)
{
    scma_handle_accessor<int> accessor;

    FT_ASSERT_EQ(0, scma_test_initialize(64));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(1, accessor.is_initialised());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), accessor._handle.index);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), accessor._handle.generation);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(1, accessor.is_initialised());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_initialize_with_handle_binds)
{
    scma_handle_accessor<int> accessor;
    scma_handle handle;

    FT_ASSERT_EQ(0, scma_test_initialize(128));
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize(handle));
    FT_ASSERT_EQ(1, accessor.is_initialised());
    FT_ASSERT_EQ(1, accessor.is_bound());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    FT_ASSERT_EQ(0, scma_free(handle));
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_initialize_invalid_handle_sets_error_and_unbinds)
{
    scma_handle_accessor<int> accessor;
    scma_handle invalid_handle;

    FT_ASSERT_EQ(0, scma_test_initialize(64));
    invalid_handle.index = FT_SYSTEM_SIZE_MAX;
    invalid_handle.generation = FT_SYSTEM_SIZE_MAX;
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, accessor.initialize(invalid_handle));
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, accessor.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(0, accessor.is_bound());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.destroy());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(scma_accessor_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_destroy_twice_succeeds)
{
    FT_ASSERT_EQ(0, scma_accessor_expect_sigabrt(scma_accessor_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_is_bound_after_destroy_aborts)
{
    FT_ASSERT_EQ(1, scma_accessor_expect_sigabrt(scma_accessor_is_bound_on_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_scma_accessor_copy_constructor_deleted)
{
    FT_ASSERT_EQ(false,
        static_cast<bool>(std::is_copy_constructible<scma_handle_accessor<int> >::value));
    return (1);
}

FT_TEST(test_scma_accessor_move_constructor_deleted)
{
    FT_ASSERT_EQ(false,
        static_cast<bool>(std::is_move_constructible<scma_handle_accessor<int> >::value));
    return (1);
}
