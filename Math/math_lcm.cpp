#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"

static unsigned long long    math_lcm_magnitude(long long value)
{
    unsigned long long magnitude;

    if (value < 0)
    {
        magnitude = static_cast<unsigned long long>(-(value + 1LL));
        magnitude += 1ULL;
        return (magnitude);
    }
    magnitude = static_cast<unsigned long long>(value);
    return (magnitude);
}

static int  math_lcm_check_overflow(unsigned long long quotient, unsigned long long magnitude_second, unsigned long long limit)
{
    if (quotient != 0 && magnitude_second > limit / quotient)
        return (1);
    return (0);
}

static int  math_lcm_report_result_int(int return_value)
{
    return (return_value);
}

static long math_lcm_report_result_long(long return_value)
{
    return (return_value);
}

static long long  math_lcm_report_result_long_long(long long return_value)
{
    return (return_value);
}

int math_lcm(int first_number, int second_number)
{
    int                     greatest_common_divisor;
    unsigned long long      magnitude_first;
    unsigned long long      magnitude_second;
    unsigned long long      quotient;
    unsigned long long      result_value;
    unsigned long long      limit_value;
    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
    {
        return (math_lcm_report_result_int(0));
    }
    magnitude_first = math_lcm_magnitude(static_cast<long long>(first_number));
    magnitude_second = math_lcm_magnitude(static_cast<long long>(second_number));
    quotient = magnitude_first / math_lcm_magnitude(static_cast<long long>(greatest_common_divisor));
    limit_value = static_cast<unsigned long long>(FT_INT32_MAX);
    if (math_lcm_check_overflow(quotient, magnitude_second, limit_value))
    {
        return (math_lcm_report_result_int(0));
    }
    result_value = quotient * magnitude_second;
    return (math_lcm_report_result_int(static_cast<int>(result_value)));
}

long math_lcm(long first_number, long second_number)
{
    long                    greatest_common_divisor;
    unsigned long long      magnitude_first;
    unsigned long long      magnitude_second;
    unsigned long long      quotient;
    unsigned long long      result_value;
    unsigned long long      limit_value;
    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
    {
        return (math_lcm_report_result_long(0));
    }
    magnitude_first = math_lcm_magnitude(static_cast<long long>(first_number));
    magnitude_second = math_lcm_magnitude(static_cast<long long>(second_number));
    quotient = magnitude_first / math_lcm_magnitude(static_cast<long long>(greatest_common_divisor));
    limit_value = static_cast<unsigned long long>(FT_LLONG_MAX);
    if (math_lcm_check_overflow(quotient, magnitude_second, limit_value))
    {
        return (math_lcm_report_result_long(0));
    }
    result_value = quotient * magnitude_second;
    return (math_lcm_report_result_long(static_cast<long>(result_value)));
}

long long   math_lcm(long long first_number, long long second_number)
{
    long long               greatest_common_divisor;
    unsigned long long      magnitude_first;
    unsigned long long      magnitude_second;
    unsigned long long      quotient;
    unsigned long long      result_value;
    unsigned long long      limit_value;
    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
    {
        return (math_lcm_report_result_long_long(0));
    }
    magnitude_first = math_lcm_magnitude(first_number);
    magnitude_second = math_lcm_magnitude(second_number);
    quotient = magnitude_first / math_lcm_magnitude(greatest_common_divisor);
    limit_value = static_cast<unsigned long long>(FT_LLONG_MAX);
    if (math_lcm_check_overflow(quotient, magnitude_second, limit_value))
    {
        return (math_lcm_report_result_long_long(0));
    }
    result_value = quotient * magnitude_second;
    return (math_lcm_report_result_long_long(static_cast<long long>(result_value)));
}
