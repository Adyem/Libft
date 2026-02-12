#include "../test_internal.hpp"
#include "test_scma_shared.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

static int scma_expect_sigabrt_uninitialized(void (*operation)(scma_handle_accessor<int>&))
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
        operation(*accessor_pointer);
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (!WIFSIGNALED(child_status))
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
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

static void scma_call_destroy(scma_handle_accessor<int> &accessor)
{
    (void)accessor.destroy();
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

FT_TEST(test_scma_accessor_uninitialized_destroy_aborts,
    "scma accessor destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, scma_expect_sigabrt_uninitialized(scma_call_destroy));
    return (1);
}
