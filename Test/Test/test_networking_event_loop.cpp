#include "../test_internal.hpp"
#include "../../Modules/Networking/networking.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_networking_event_loop_add_socket_reports_allocation_failure)
{
    event_loop loop;
    int add_result;

    event_loop_init(&loop);
    cma_set_alloc_limit(1);
    add_result = event_loop_add_socket(&loop, 42, false);
    cma_set_alloc_limit(0);
    if (add_result != -1)
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

FT_TEST(test_networking_event_loop_remove_socket_sets_errno_when_missing)
{
    event_loop loop;
    int add_result;
    int remove_result;

    event_loop_init(&loop);
    add_result = event_loop_add_socket(&loop, 7, false);
    if (add_result != 0)
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

FT_TEST(test_networking_event_loop_init_sets_up_thread_safety)
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

FT_TEST(test_networking_event_loop_lock_and_unlock_reset_errno)
{
    event_loop loop;
    ft_bool lock_acquired;
    int        lock_result;

    event_loop_init(&loop);
    lock_acquired = FT_FALSE;
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
    event_loop_unlock(&loop, lock_acquired);
    event_loop_clear(&loop);
    return (1);
}
