#include "math.hpp"

static ft_big_number math_big_error_result(int error_code)
{
    return (ft_big_number_proxy(error_code));
}

static ft_big_number math_big_absolute_value(const ft_big_number &number)
{
    ft_big_number zero_number;
    int zero_initialization_error;

    zero_initialization_error = zero_number.initialize();
    if (zero_initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(zero_initialization_error));
    if (!number.is_negative())
    {
        ft_big_number positive_number;
        int positive_initialization_error = positive_number.initialize(number);

        if (positive_initialization_error != FT_ERR_SUCCESS)
            return (math_big_error_result(positive_initialization_error));
        positive_number.trim_leading_zeros();
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
            return (math_big_error_result(ft_big_number::last_operation_error()));
        return (positive_number + zero_number);
    }
    ft_big_number positive_number = zero_number - number;

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    positive_number.trim_leading_zeros();
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    return (positive_number + zero_number);
}

static ft_big_number math_big_gcd_normalized(const ft_big_number &first_input,
    const ft_big_number &second_input)
{
    ft_big_number first_value;
    ft_big_number second_value;
    ft_big_number zero_number;
    int initialization_error;

    initialization_error = zero_number.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));
    initialization_error = first_value.initialize(first_input);
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));
    initialization_error = second_value.initialize(second_input);
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));

    while (!(second_value == zero_number))
    {
        ft_big_number remainder_number = first_value % second_value;

        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
            return (math_big_error_result(ft_big_number::last_operation_error()));
        first_value = second_value;
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
            return (math_big_error_result(ft_big_number::last_operation_error()));
        second_value = remainder_number;
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
            return (math_big_error_result(ft_big_number::last_operation_error()));
    }
    first_value.trim_leading_zeros();
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    return (first_value + zero_number);
}

ft_big_number math_big_gcd(const ft_big_number &first_number, const ft_big_number &second_number)
{
    ft_big_number first_value = math_big_absolute_value(first_number);
    ft_big_number zero_number;
    int zero_initialization_error;

    zero_initialization_error = zero_number.initialize();
    if (zero_initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(zero_initialization_error));
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    ft_big_number second_value = math_big_absolute_value(second_number);

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    ft_big_number gcd_value = math_big_gcd_normalized(first_value, second_value);

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    return (gcd_value + zero_number);
}

ft_big_number math_big_lcm(const ft_big_number &first_number, const ft_big_number &second_number)
{
    ft_big_number first_value = math_big_absolute_value(first_number);

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    ft_big_number second_value = math_big_absolute_value(second_number);

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    ft_big_number zero_number;
    int zero_initialization_error;

    zero_initialization_error = zero_number.initialize();
    if (zero_initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(zero_initialization_error));

    if (first_value == zero_number || second_value == zero_number)
    {
        ft_big_number zero_result;
        int zero_result_initialization_error = zero_result.initialize();

        if (zero_result_initialization_error != FT_ERR_SUCCESS)
            return (math_big_error_result(zero_result_initialization_error));
        return (zero_result + zero_number);
    }
    ft_big_number gcd_value = math_big_gcd_normalized(first_value, second_value);

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    ft_big_number product_value = first_value * second_value;

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    ft_big_number lcm_value = product_value / gcd_value;

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESS)
        return (math_big_error_result(ft_big_number::last_operation_error()));
    return (lcm_value + zero_number);
}
