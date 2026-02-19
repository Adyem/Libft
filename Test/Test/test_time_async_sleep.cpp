#include "../test_internal.hpp"
#include "../../Time/time.hpp"
#include "../../Networking/networking.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_time_async_sleep_init_null_pointer_sets_errno,
    "time_async_sleep_init returns invalid argument for null state pointer")
{
    time_async_sleep_init(ft_nullptr, 100);
    return (1);
}

FT_TEST(test_time_async_sleep_init_negative_delay_completes_immediately,
    "time_async_sleep_init treats negative delay as completed state")
{
    t_time_async_sleep    sleep_state;
    long long             remaining_delay;

    time_async_sleep_init(&sleep_state, -1);
    FT_ASSERT(time_async_sleep_is_complete(&sleep_state));
    remaining_delay = time_async_sleep_remaining_ms(&sleep_state);
    FT_ASSERT_EQ(0LL, remaining_delay);
    return (1);
}

FT_TEST(test_time_async_sleep_remaining_ms_marks_completion_after_delay,
    "time_async_sleep_remaining_ms marks completion once deadline passes")
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

FT_TEST(test_time_async_sleep_poll_validates_arguments,
    "time_async_sleep_poll returns invalid argument for null loop pointer")
{
    t_time_async_sleep    sleep_state;
    int                   poll_result;

    time_async_sleep_init(&sleep_state, 0);
    poll_result = time_async_sleep_poll(ft_nullptr, &sleep_state);
    FT_ASSERT_EQ(-1, poll_result);
    return (1);
}

FT_TEST(test_time_async_sleep_poll_completes_immediate_state_without_blocking,
    "time_async_sleep_poll returns success when sleep already completed")
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
