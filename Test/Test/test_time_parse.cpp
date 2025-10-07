#include "../../Time/time.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"

FT_TEST(test_time_parse_iso8601_pre_epoch, "time_parse_iso8601 handles pre-epoch timestamps")
{
    std::tm parsed_time;
    t_time parsed_timestamp;
    bool parse_result;

    ft_memset(&parsed_time, 0, sizeof(parsed_time));
    parsed_timestamp = 0;
    ft_errno = FT_ETERM;
    parse_result = time_parse_iso8601("1969-12-31T23:59:59Z", &parsed_time, &parsed_timestamp);
    FT_ASSERT(parse_result == true);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<t_time>(-1), parsed_timestamp);
    FT_ASSERT_EQ(1969 - 1900, parsed_time.tm_year);
    FT_ASSERT_EQ(11, parsed_time.tm_mon);
    FT_ASSERT_EQ(31, parsed_time.tm_mday);
    FT_ASSERT_EQ(23, parsed_time.tm_hour);
    FT_ASSERT_EQ(59, parsed_time.tm_min);
    FT_ASSERT_EQ(59, parsed_time.tm_sec);
    return (1);
}
