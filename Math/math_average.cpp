#include "math.hpp"

int math_average(int first_number, int second_number)
{
    int first_half;
    int second_half;
    int remainder_sum;
    int halves_sum;

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

long math_average(long first_number, long second_number)
{
    long first_half;
    long second_half;
    long remainder_sum;
    long halves_sum;

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

long long math_average(long long first_number, long long second_number)
{
    long long first_half;
    long long second_half;
    long long remainder_sum;
    long long halves_sum;

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

