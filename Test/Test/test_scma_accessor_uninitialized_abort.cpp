#include "../test_internal.hpp"
#include "test_scma_shared.hpp"
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <new>
#include <sys/wait.h>
#include <unistd.h>
#include "../../CPP_class/class_nullptr.hpp"
#include "../../PThread/recursive_mutex.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static sigjmp_buf g_scma_accessor_uninitialized_jump;

static void scma_accessor_uninitialized_handler(int /*signal*/)
{
    siglongjmp(g_scma_accessor_uninitialized_jump, 1);
}

static int scma_expect_sigabrt_uninitialized(void (*operation)(scma_handle_accessor<int>&))
{
    struct sigaction action;
    struct sigaction backup;
    memset(&action, 0, sizeof(action));
    action.sa_handler = scma_accessor_uninitialized_handler;
    sigemptyset(&action.sa_mask);
    int result = 0;

    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_scma_accessor_uninitialized_jump, 1) == 0)
    {
        alignas(scma_handle_accessor<int>) unsigned char storage[
            sizeof(scma_handle_accessor<int>)];
        scma_handle_accessor<int> *accessor_pointer;

        accessor_pointer = new (storage) scma_handle_accessor<int>();
        operation(*accessor_pointer);
        accessor_pointer->~scma_handle_accessor<int>();
        result = 0;
    }
    else
    {
        result = 1;
    }
    sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

static void scma_call_is_initialized(scma_handle_accessor<int> &accessor)
{
    (void)accessor.is_initialized();
    return ;
}

static void scma_call_bind(scma_handle_accessor<int> &accessor)
{
    scma_handle handle;

    handle.index = 0;
    handle.generation = 0;
    (void)accessor.bind(handle);
    return ;
}

static void scma_call_is_bound(scma_handle_accessor<int> &accessor)
{
    (void)accessor.is_bound();
    return ;
}

static void scma_call_get_handle(scma_handle_accessor<int> &accessor)
{
    (void)accessor.get_handle();
    return ;
}

static void scma_call_get_error(scma_handle_accessor<int> &accessor)
{
    (void)accessor.get_error();
    return ;
}

static void scma_call_noop(scma_handle_accessor<int> &accessor)
{
    (void)accessor;
    return ;
}
FT_TEST(test_scma_accessor_uninitialized_is_initialized_aborts,
    "scma accessor is_initialized aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, scma_expect_sigabrt_uninitialized(scma_call_is_initialized));
    return (1);
}

FT_TEST(test_scma_accessor_uninitialized_bind_aborts,
    "scma accessor bind aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, scma_expect_sigabrt_uninitialized(scma_call_bind));
    return (1);
}

FT_TEST(test_scma_accessor_uninitialized_is_bound_aborts,
    "scma accessor is_bound aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, scma_expect_sigabrt_uninitialized(scma_call_is_bound));
    return (1);
}

FT_TEST(test_scma_accessor_uninitialized_get_handle_aborts,
    "scma accessor get_handle aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, scma_expect_sigabrt_uninitialized(scma_call_get_handle));
    return (1);
}

FT_TEST(test_scma_accessor_uninitialized_get_error_aborts,
    "scma accessor get_error aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, scma_expect_sigabrt_uninitialized(scma_call_get_error));
    return (1);
}

FT_TEST(test_scma_accessor_uninitialized_destroy_returns_invalid_state,
    "scma accessor destroy returns invalid state on uninitialized instance")
{
    scma_handle_accessor<int> accessor;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, accessor.destroy());
    return (1);
}

FT_TEST(test_scma_accessor_uninitialized_destructor_tolerates_object,
    "scma accessor destructor tolerates uninitialized instance")
{
    FT_ASSERT_EQ(0, scma_expect_sigabrt_uninitialized(scma_call_noop));
    return (1);
}

static int scma_expect_sigabrt_destructor_with_lock_failure(void)
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(scma_handle_accessor<int>) unsigned char storage[
            sizeof(scma_handle_accessor<int>)];
        scma_handle_accessor<int> *accessor_pointer;

        accessor_pointer = new (storage) scma_handle_accessor<int>();
        (void)accessor_pointer->initialize();
        pt_recursive_mutex_lock_override_error_code.store(
            FT_ERR_SYS_MUTEX_LOCK_FAILED, std::memory_order_release);
        accessor_pointer->~scma_handle_accessor<int>();
        pt_recursive_mutex_lock_override_error_code.store(
            FT_ERR_SUCCESS, std::memory_order_release);
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

FT_TEST(test_scma_accessor_destructor_survives_mutex_lock_failure,
    "scma accessor destructor skips mutex locking so lock failures do not abort")
{
    FT_ASSERT_EQ(0, scma_expect_sigabrt_destructor_with_lock_failure());
    return (1);
}
