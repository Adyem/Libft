#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

static uint64_t    math_lcm_magnitude(int64_t value)
{
    uint64_t magnitude;

    if (value < 0)
    {
        magnitude = static_cast<uint64_t>(-(value + 1LL));
        magnitude += 1U;
        return (magnitude);
    }
    magnitude = static_cast<uint64_t>(value);
    return (magnitude);
}

static int32_t  math_lcm_check_overflow(uint64_t quotient,
    uint64_t magnitude_second, uint64_t limit)
{
    if (quotient != 0 && magnitude_second > limit / quotient)
        return (1);
    return (0);
}

static int32_t  math_lcm_report_result_int32(int32_t return_value)
{
    return (return_value);
}

static int64_t  math_lcm_report_result_int64(int64_t return_value)
{
    return (return_value);
}

int32_t math_lcm(int32_t first_number, int32_t second_number)
{
    int32_t                 greatest_common_divisor;
    uint64_t                magnitude_first;
    uint64_t                magnitude_second;
    uint64_t                quotient;
    uint64_t                result_value;
    uint64_t                limit_value;
    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
    {
        return (math_lcm_report_result_int32(0));
    }
    magnitude_first = math_lcm_magnitude(static_cast<int64_t>(first_number));
    magnitude_second = math_lcm_magnitude(static_cast<int64_t>(second_number));
    quotient = magnitude_first
        / math_lcm_magnitude(static_cast<int64_t>(greatest_common_divisor));
    limit_value = static_cast<uint64_t>(FT_INT32_MAX);
    if (math_lcm_check_overflow(quotient, magnitude_second, limit_value))
    {
        return (math_lcm_report_result_int32(0));
    }
    result_value = quotient * magnitude_second;
    return (math_lcm_report_result_int32(static_cast<int32_t>(result_value)));
}

int64_t math_lcm(int64_t first_number, int64_t second_number)
{
    int64_t                 greatest_common_divisor;
    uint64_t                magnitude_first;
    uint64_t                magnitude_second;
    uint64_t                quotient;
    uint64_t                result_value;
    uint64_t                limit_value;

    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
    {
        return (math_lcm_report_result_int64(0));
    }
    magnitude_first = math_lcm_magnitude(first_number);
    magnitude_second = math_lcm_magnitude(second_number);
    quotient = magnitude_first / math_lcm_magnitude(greatest_common_divisor);
    limit_value = static_cast<uint64_t>(FT_LLONG_MAX);
    if (math_lcm_check_overflow(quotient, magnitude_second, limit_value))
    {
        return (math_lcm_report_result_int64(0));
    }
    result_value = quotient * magnitude_second;
    return (math_lcm_report_result_int64(static_cast<int64_t>(result_value)));
}
