#include "math.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

int32_t math_average(int32_t first_number, int32_t second_number)
{
    int32_t first_half;
    int32_t second_half;
    int32_t remainder_sum;
    int32_t halves_sum;

    first_half = (first_number / 2);
    second_half = (second_number / 2);
    remainder_sum = (first_number % 2) + (second_number % 2);
    halves_sum = (first_half + second_half);
    if (remainder_sum == 2)
        return (halves_sum + 1);
    if (remainder_sum == -2)
        return (halves_sum - 1);
    if (remainder_sum == 1)
    {
        if (halves_sum < 0)
            return (halves_sum + 1);
        return (halves_sum);
    }
    if (remainder_sum == -1)
    {
        if (halves_sum > 0)
            return (halves_sum - 1);
        return (halves_sum);
    }
    return (halves_sum);
}

int64_t math_average(int64_t first_number, int64_t second_number)
{
    int64_t first_half;
    int64_t second_half;
    int64_t remainder_sum;
    int64_t halves_sum;

    first_half = (first_number / 2);
    second_half = (second_number / 2);
    remainder_sum = (first_number % 2) + (second_number % 2);
    halves_sum = (first_half + second_half);
    if (remainder_sum == 2)
        return (halves_sum + 1);
    if (remainder_sum == -2)
        return (halves_sum - 1);
    if (remainder_sum == 1)
    {
        if (halves_sum < 0)
            return (halves_sum + 1);
        return (halves_sum);
    }
    if (remainder_sum == -1)
    {
        if (halves_sum > 0)
            return (halves_sum - 1);
        return (halves_sum);
    }
    return (halves_sum);
}

double math_average(double first_number, double second_number)
{
    return ((first_number + second_number) / 2.0);
}
