#include "../../System_utils/test_runner.hpp"
#include "../../Time/time.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"

static t_time_info create_sample_time_info(void)
{
    t_time_info sample_time_info;

    sample_time_info.seconds = 0;
    sample_time_info.minutes = 0;
    sample_time_info.hours = 0;
    sample_time_info.month_day = 1;
    sample_time_info.month = 0;
    sample_time_info.year = 0;
    sample_time_info.week_day = 0;
    sample_time_info.year_day = 0;
    sample_time_info.is_daylight_saving = 0;
    return (sample_time_info);
}

FT_TEST(test_time_strftime_null_buffer_sets_errno, "time_strftime null buffer sets FT_ERR_INVALID_ARGUMENT")
{
    t_time_info sample_time_info;

    sample_time_info = create_sample_time_info();
    ft_errno = ER_SUCCESS;
    size_t format_result = time_strftime(static_cast<char *>(ft_nullptr), 16, "%Y", &sample_time_info);
    FT_ASSERT_EQ(static_cast<size_t>(0), format_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_time_strftime_zero_size_sets_errno, "time_strftime zero size sets FT_ERR_INVALID_ARGUMENT")
{
    char buffer[8];
    t_time_info sample_time_info;

    buffer[0] = 'X';
    sample_time_info = create_sample_time_info();
    ft_errno = ER_SUCCESS;
    size_t format_result = time_strftime(buffer, 0, "%Y", &sample_time_info);
    FT_ASSERT_EQ(static_cast<size_t>(0), format_result);
    FT_ASSERT_EQ('X', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_time_strftime_null_format_sets_errno, "time_strftime null format sets FT_ERR_INVALID_ARGUMENT")
{
    char buffer[8];
    t_time_info sample_time_info;

    buffer[0] = 'X';
    sample_time_info = create_sample_time_info();
    ft_errno = ER_SUCCESS;
    size_t format_result = time_strftime(buffer, sizeof(buffer), static_cast<const char *>(ft_nullptr), &sample_time_info);
    FT_ASSERT_EQ(static_cast<size_t>(0), format_result);
    FT_ASSERT_EQ('X', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_time_strftime_success_resets_errno, "time_strftime success resets errno")
{
    char buffer[32];
    t_time_info sample_time_info;
    const char  *expected_string;
    size_t  expected_length;

    sample_time_info = create_sample_time_info();
    sample_time_info.year = 124;
    sample_time_info.month = 10;
    sample_time_info.month_day = 5;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    size_t format_result = time_strftime(buffer, sizeof(buffer), "%Y-%m-%d", &sample_time_info);
    expected_string = "2024-11-05";
    expected_length = ft_strlen(expected_string);
    FT_ASSERT_EQ(expected_length, format_result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, ft_strcmp(expected_string, buffer));
    return (1);
}
