#include "../test_internal.hpp"
#include "../../Modules/Time/time_fps.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_time_fps_set_rejects_low_frame_rate)
{
    time_fps limiter;
    int set_result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, limiter.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, limiter.set_frames_per_second(30));
    set_result = limiter.set_frames_per_second(10);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, set_result);
    FT_ASSERT_EQ(0L, limiter.get_frames_per_second());
    return (1);
}

FT_TEST(test_time_fps_set_accepts_valid_frame_rate)
{
    time_fps limiter;
    long fps_before;
    int set_result;
    long fps_after;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, limiter.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, limiter.set_frames_per_second(30));
    fps_before = limiter.get_frames_per_second();
    FT_ASSERT_EQ(30L, fps_before);
    set_result = limiter.set_frames_per_second(48);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_result);

    fps_after = limiter.get_frames_per_second();
    FT_ASSERT_EQ(48L, fps_after);
    return (1);
}
