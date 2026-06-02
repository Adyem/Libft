#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

static int32_t math_gcd_normalized_int(int32_t first_number,
    int32_t second_number)
{
    int32_t remainder_value;

    remainder_value = 0;
    while (second_number != 0)
    {
        remainder_value = first_number % second_number;
        first_number = second_number;
        second_number = remainder_value;
    }
    return (first_number);
}

static int64_t math_gcd_normalized_int64(int64_t first_number,
    int64_t second_number)
{
    int64_t remainder_value;

    remainder_value = 0;
    while (second_number != 0)
    {
        remainder_value = first_number % second_number;
        first_number = second_number;
        second_number = remainder_value;
    }
    return (first_number);
}

int32_t math_gcd(int32_t first_number, int32_t second_number)
{
    int32_t result_value;

    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    result_value = math_gcd_normalized_int(first_number, second_number);
    return (result_value);
}

int64_t math_gcd(int64_t first_number, int64_t second_number)
{
    int64_t result_value;

    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    result_value = math_gcd_normalized_int64(first_number, second_number);
    return (result_value);
}
