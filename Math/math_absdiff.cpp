#include "math.hpp"

int math_absdiff(int first_number, int second_number)
{
    int difference;

    difference = first_number - second_number;
    return (math_abs(difference));
}

long math_absdiff(long first_number, long second_number)
{
    long difference;

    difference = first_number - second_number;
    return (math_abs(difference));
}

long long math_absdiff(long long first_number, long long second_number)
{
    long long difference;

    difference = first_number - second_number;
    return (math_abs(difference));
}

double math_absdiff(double first_number, double second_number)
{
    double difference;

    difference = first_number - second_number;
    return (math_fabs(difference));
}

