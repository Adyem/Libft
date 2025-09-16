#include "math.hpp"

static ft_big_number math_big_absolute_value(const ft_big_number &number)
{
    if (number.get_error() != 0)
    {
        ft_big_number error_copy(number);

        return (error_copy);
    }
    if (!number.is_negative())
    {
        ft_big_number positive_number(number);

        positive_number.trim_leading_zeros();
        return (positive_number);
    }
    ft_big_number zero_number;
    ft_big_number positive_number = zero_number - number;

    if (positive_number.get_error() != 0)
        return (positive_number);
    positive_number.trim_leading_zeros();
    return (positive_number);
}

static ft_big_number math_big_gcd_normalized(ft_big_number first_value, ft_big_number second_value)
{
    if (first_value.get_error() != 0)
        return (first_value);
    if (second_value.get_error() != 0)
        return (second_value);
    ft_big_number zero_number;

    while (!(second_value == zero_number))
    {
        ft_big_number remainder_number = first_value % second_value;

        if (remainder_number.get_error() != 0)
            return (remainder_number);
        first_value = second_value;
        if (first_value.get_error() != 0)
            return (first_value);
        second_value = remainder_number;
        if (second_value.get_error() != 0)
            return (second_value);
    }
    first_value.trim_leading_zeros();
    return (first_value);
}

ft_big_number math_big_gcd(const ft_big_number &first_number, const ft_big_number &second_number)
{
    ft_big_number first_value = math_big_absolute_value(first_number);

    if (first_value.get_error() != 0)
        return (first_value);
    ft_big_number second_value = math_big_absolute_value(second_number);

    if (second_value.get_error() != 0)
        return (second_value);
    ft_big_number gcd_value = math_big_gcd_normalized(first_value, second_value);

    return (gcd_value);
}

ft_big_number math_big_lcm(const ft_big_number &first_number, const ft_big_number &second_number)
{
    ft_big_number first_value = math_big_absolute_value(first_number);

    if (first_value.get_error() != 0)
        return (first_value);
    ft_big_number second_value = math_big_absolute_value(second_number);

    if (second_value.get_error() != 0)
        return (second_value);
    ft_big_number zero_number;

    if (first_value == zero_number || second_value == zero_number)
    {
        ft_big_number zero_result;

        return (zero_result);
    }
    ft_big_number gcd_value = math_big_gcd_normalized(first_value, second_value);

    if (gcd_value.get_error() != 0)
        return (gcd_value);
    ft_big_number product_value = first_value * second_value;

    if (product_value.get_error() != 0)
        return (product_value);
    ft_big_number lcm_value = product_value / gcd_value;

    return (lcm_value);
}
