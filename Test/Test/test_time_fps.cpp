#include "../../Time/fps.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_time_fps_set_rejects_low_frame_rate, "time_fps::set_frames_per_second enforces minimum frame rate")
{
    time_fps limiter(30);
    int set_result;

    ft_errno = ER_SUCCESS;
    set_result = limiter.set_frames_per_second(10);
    FT_ASSERT_EQ(-1, set_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, limiter.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0L, limiter.get_frames_per_second());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, limiter.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_time_fps_set_accepts_valid_frame_rate, "time_fps::set_frames_per_second updates frame duration on success")
{
    time_fps limiter(30);
    long fps_before;
    int set_result;
    long fps_after;

    fps_before = limiter.get_frames_per_second();
    FT_ASSERT_EQ(30L, fps_before);
    FT_ASSERT_EQ(ER_SUCCESS, limiter.get_error());

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    set_result = limiter.set_frames_per_second(48);
    FT_ASSERT_EQ(0, set_result);
    FT_ASSERT_EQ(ER_SUCCESS, limiter.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    fps_after = limiter.get_frames_per_second();
    FT_ASSERT_EQ(48L, fps_after);
    FT_ASSERT_EQ(ER_SUCCESS, limiter.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

