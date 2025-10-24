#include "../../Networking/networking.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_event_loop_add_socket_reports_allocation_failure,
    "event_loop_add_socket reports allocation failure")
{
    event_loop loop;
    int add_result;

    event_loop_init(&loop);
    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    add_result = event_loop_add_socket(&loop, 42, false);
    cma_set_alloc_limit(0);
    if (add_result != -1)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (ft_errno != FT_ERR_NO_MEMORY)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (loop.read_count != 0)
    {
        event_loop_clear(&loop);
        return (0);
    }
    event_loop_clear(&loop);
    return (1);
}

FT_TEST(test_event_loop_remove_socket_sets_errno_when_missing,
    "event_loop_remove_socket sets FT_ERR_INVALID_ARGUMENT when descriptor missing")
{
    event_loop loop;
    int add_result;
    int remove_result;

    event_loop_init(&loop);
    ft_errno = ER_SUCCESS;
    add_result = event_loop_add_socket(&loop, 7, false);
    if (add_result != 0)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (ft_errno != ER_SUCCESS)
    {
        event_loop_clear(&loop);
        return (0);
    }
    remove_result = event_loop_remove_socket(&loop, 42, false);
    if (remove_result != -1)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (loop.read_count != 1)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (loop.read_file_descriptors[0] != 7)
    {
        event_loop_clear(&loop);
        return (0);
    }
    event_loop_clear(&loop);
    return (1);
}

FT_TEST(test_event_loop_init_sets_up_thread_safety,
    "event_loop_init prepares thread safety primitives")
{
    event_loop loop;

    event_loop_init(&loop);
    if (!loop.thread_safe_enabled)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (loop.mutex == ft_nullptr)
    {
        event_loop_clear(&loop);
        return (0);
    }
    event_loop_clear(&loop);
    return (1);
}

FT_TEST(test_event_loop_lock_and_unlock_preserve_errno,
    "event_loop_lock acquires the mutex and unlock preserves errno")
{
    event_loop loop;
    bool       lock_acquired;
    int        lock_result;

    event_loop_init(&loop);
    lock_acquired = false;
    lock_result = event_loop_lock(&loop, &lock_acquired);
    if (lock_result != 0)
    {
        event_loop_clear(&loop);
        return (0);
    }
    if (!lock_acquired)
    {
        event_loop_clear(&loop);
        return (0);
    }
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    event_loop_unlock(&loop, lock_acquired);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
    {
        event_loop_clear(&loop);
        return (0);
    }
    event_loop_clear(&loop);
    return (1);
}
