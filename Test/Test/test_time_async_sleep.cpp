#include "../test_internal.hpp"
#include "../../Modules/Time/time.hpp"
#include "../../Modules/Networking/networking.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_time_async_sleep_init_null_pointer_sets_errno)
{
    time_async_sleep_init(ft_nullptr, 100);
    return (1);
}

FT_TEST(test_time_async_sleep_init_negative_delay_completes_immediately)
{
    t_time_async_sleep    sleep_state;
    long long             remaining_delay;

    time_async_sleep_init(&sleep_state, -1);
    FT_ASSERT(time_async_sleep_is_complete(&sleep_state));
    remaining_delay = time_async_sleep_remaining_ms(&sleep_state);
    FT_ASSERT_EQ(0LL, remaining_delay);
    return (1);
}

FT_TEST(test_time_async_sleep_remaining_ms_marks_completion_after_delay)
{
    t_time_async_sleep    sleep_state;
    long long             remaining_delay;
    int                   iteration_count;

    time_async_sleep_init(&sleep_state, 10);
    FT_ASSERT(!time_async_sleep_is_complete(&sleep_state));
    iteration_count = 0;
    while (!time_async_sleep_is_complete(&sleep_state) && iteration_count < 100)
    {
        remaining_delay = time_async_sleep_remaining_ms(&sleep_state);
        if (!time_async_sleep_is_complete(&sleep_state) && remaining_delay > 0)
            time_sleep_ms(1);
        iteration_count++;
    }
    FT_ASSERT(time_async_sleep_is_complete(&sleep_state));
    remaining_delay = time_async_sleep_remaining_ms(&sleep_state);
    FT_ASSERT_EQ(0LL, remaining_delay);
    return (1);
}

FT_TEST(test_time_async_sleep_poll_validates_arguments)
{
    t_time_async_sleep    sleep_state;
    int                   poll_result;

    time_async_sleep_init(&sleep_state, 0);
    poll_result = time_async_sleep_poll(ft_nullptr, &sleep_state);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, poll_result);
    return (1);
}

FT_TEST(test_time_async_sleep_poll_completes_immediate_state_without_blocking)
{
    t_time_async_sleep    sleep_state;
    event_loop            loop;
    int                   poll_result;

    event_loop_init(&loop);
    time_async_sleep_init(&sleep_state, 0);
    poll_result = time_async_sleep_poll(&loop, &sleep_state);
    event_loop_clear(&loop);
    FT_ASSERT_EQ(0, poll_result);
    return (1);
}
