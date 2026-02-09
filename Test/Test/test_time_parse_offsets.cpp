#include "../../Time/time.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"

FT_TEST(test_time_parse_iso8601_positive_offset_aligns_with_utc,
    "time_parse_iso8601 aligns offset timestamps with canonical UTC")
{
    std::tm base_time;
    std::tm offset_time;
    t_time base_timestamp;
    t_time offset_timestamp;
    bool parse_result;

    ft_memset(&base_time, 0, sizeof(base_time));
    ft_memset(&offset_time, 0, sizeof(offset_time));
    base_timestamp = 0;
    offset_timestamp = 0;

    ft_errno = FT_ERR_CONFIGURATION;
    parse_result = time_parse_iso8601("2024-03-01T10:04:56Z", &base_time, &base_timestamp);
    FT_ASSERT(parse_result == true);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    ft_errno = FT_ERR_TERMINATED;
    parse_result = time_parse_iso8601("2024-03-01T12:34:56+02:30", &offset_time, &offset_timestamp);
    FT_ASSERT(parse_result == true);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    FT_ASSERT_EQ(base_timestamp, offset_timestamp);
    FT_ASSERT_EQ(base_time.tm_year, offset_time.tm_year);
    FT_ASSERT_EQ(base_time.tm_mon, offset_time.tm_mon);
    FT_ASSERT_EQ(base_time.tm_mday, offset_time.tm_mday);
    FT_ASSERT_EQ(base_time.tm_hour, offset_time.tm_hour);
    FT_ASSERT_EQ(base_time.tm_min, offset_time.tm_min);
    FT_ASSERT_EQ(base_time.tm_sec, offset_time.tm_sec);
    return (1);
}

FT_TEST(test_time_parse_iso8601_negative_offset_without_colon,
    "time_parse_iso8601 handles compact negative offsets")
{
    std::tm base_time;
    std::tm offset_time;
    t_time base_timestamp;
    t_time offset_timestamp;
    bool parse_result;

    ft_memset(&base_time, 0, sizeof(base_time));
    ft_memset(&offset_time, 0, sizeof(offset_time));
    base_timestamp = 0;
    offset_timestamp = 0;

    ft_errno = FT_ERR_TERMINATED;
    parse_result = time_parse_iso8601("2024-03-01T10:04:56Z", &base_time, &base_timestamp);
    FT_ASSERT(parse_result == true);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    ft_errno = FT_ERR_ALREADY_EXISTS;
    parse_result = time_parse_iso8601("2024-03-01T03:04:56-0700", &offset_time, &offset_timestamp);
    FT_ASSERT(parse_result == true);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    FT_ASSERT_EQ(base_timestamp, offset_timestamp);
    FT_ASSERT_EQ(base_time.tm_year, offset_time.tm_year);
    FT_ASSERT_EQ(base_time.tm_mon, offset_time.tm_mon);
    FT_ASSERT_EQ(base_time.tm_mday, offset_time.tm_mday);
    FT_ASSERT_EQ(base_time.tm_hour, offset_time.tm_hour);
    FT_ASSERT_EQ(base_time.tm_min, offset_time.tm_min);
    FT_ASSERT_EQ(base_time.tm_sec, offset_time.tm_sec);
    return (1);
}

FT_TEST(test_time_parse_iso8601_rejects_out_of_range_offset,
    "time_parse_iso8601 rejects offsets beyond 24 hours")
{
    std::tm parsed_time;
    t_time parsed_timestamp;
    bool parse_result;

    ft_memset(&parsed_time, 0, sizeof(parsed_time));
    parsed_timestamp = 0;

    ft_errno = FT_ERR_SUCCESSS;
    parse_result = time_parse_iso8601("2024-03-01T12:34:56+25:00", &parsed_time, &parsed_timestamp);
    FT_ASSERT_EQ(false, parse_result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);

    ft_errno = FT_ERR_SUCCESSS;
    parse_result = time_parse_iso8601("2024-03-01T12:34:56-07:61", &parsed_time, &parsed_timestamp);
    FT_ASSERT_EQ(false, parse_result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}
