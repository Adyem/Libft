#include "math.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

static ft_big_number *math_big_allocate_result(int32_t *error_code)
{
    ft_big_number *result;
    int32_t initialization_error;

    result = new (std::nothrow) ft_big_number();
    if (result == ft_nullptr)
    {
        if (error_code != ft_nullptr)
            *error_code = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    initialization_error = result->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        delete result;
        if (error_code != ft_nullptr)
            *error_code = initialization_error;
        return (ft_nullptr);
    }
    if (error_code != ft_nullptr)
        *error_code = FT_ERR_SUCCESS;
    return (result);
}

static ft_big_number *math_big_error_result(int32_t error_code)
{
    ft_big_number *result;
    int32_t allocation_error;

    result = math_big_allocate_result(&allocation_error);
    if (result == ft_nullptr)
        return (ft_nullptr);
    (void)error_code;
    return (result);
}

static ft_big_number *math_big_absolute_value(const ft_big_number &number)
{
    ft_big_number zero_number;
    ft_big_number *result;
    int32_t initialization_error;

    initialization_error = zero_number.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));
    result = math_big_allocate_result(&initialization_error);
    if (result == ft_nullptr)
        return (math_big_error_result(initialization_error));
    if (!number.is_negative())
    {
        *result = number;
        if (ft_big_number::get_error() != FT_ERR_SUCCESS)
        {
            initialization_error = ft_big_number::get_error();
            (void)result->destroy();
            delete result;
            return (math_big_error_result(initialization_error));
        }
        result->trim_leading_zeros();
        return (result);
    }
    ft_big_number_proxy absolute_proxy = zero_number - number;
    if (absolute_proxy.get_error() != FT_ERR_SUCCESS)
    {
        initialization_error = absolute_proxy.get_error();
        (void)result->destroy();
        delete result;
        return (math_big_error_result(initialization_error));
    }
    *result = absolute_proxy;
    result->trim_leading_zeros();
    return (result);
}

static ft_big_number *math_big_gcd_normalized(const ft_big_number &first_input,
    const ft_big_number &second_input)
{
    ft_big_number first_value;
    ft_big_number second_value;
    ft_big_number zero_number;
    ft_big_number remainder_number;
    ft_big_number *result;
    int32_t initialization_error;

    initialization_error = zero_number.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));
    initialization_error = first_value.initialize(first_input);
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));
    initialization_error = second_value.initialize(second_input);
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));
    initialization_error = remainder_number.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (math_big_error_result(initialization_error));
    while (!(second_value == zero_number))
    {
        ft_big_number_proxy remainder_proxy = first_value % second_value;

        if (remainder_proxy.get_error() != FT_ERR_SUCCESS)
            return (math_big_error_result(remainder_proxy.get_error()));
        remainder_number = remainder_proxy;
        first_value = second_value;
        second_value = remainder_number;
    }
    first_value.trim_leading_zeros();
    result = math_big_allocate_result(&initialization_error);
    if (result == ft_nullptr)
        return (math_big_error_result(initialization_error));
    *result = first_value;
    if (ft_big_number::get_error() != FT_ERR_SUCCESS)
    {
        initialization_error = ft_big_number::get_error();
        (void)result->destroy();
        delete result;
        return (math_big_error_result(initialization_error));
    }
    return (result);
}

ft_big_number *math_big_gcd(const ft_big_number &first_number, const ft_big_number &second_number)
{
    ft_big_number *first_value;
    ft_big_number *second_value;
    ft_big_number *gcd_value;

    first_value = math_big_absolute_value(first_number);
    if (first_value == ft_nullptr)
        return (ft_nullptr);
    second_value = math_big_absolute_value(second_number);
    if (second_value == ft_nullptr)
    {
        (void)first_value->destroy();
        delete first_value;
        return (ft_nullptr);
    }
    gcd_value = math_big_gcd_normalized(*first_value, *second_value);
    (void)first_value->destroy();
    delete first_value;
    (void)second_value->destroy();
    delete second_value;
    return (gcd_value);
}

ft_big_number *math_big_lcm(const ft_big_number &first_number, const ft_big_number &second_number)
{
    ft_big_number *first_value;
    ft_big_number *second_value;
    ft_big_number *gcd_value;
    ft_big_number *result;
    ft_big_number zero_number;
    ft_big_number product_value;
    int32_t initialization_error;

    first_value = math_big_absolute_value(first_number);
    if (first_value == ft_nullptr)
        return (ft_nullptr);
    second_value = math_big_absolute_value(second_number);
    if (second_value == ft_nullptr)
    {
        (void)first_value->destroy();
        delete first_value;
        return (ft_nullptr);
    }
    initialization_error = zero_number.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        (void)first_value->destroy();
        delete first_value;
        (void)second_value->destroy();
        delete second_value;
        return (math_big_error_result(initialization_error));
    }
    if (*first_value == zero_number || *second_value == zero_number)
    {
        (void)first_value->destroy();
        delete first_value;
        (void)second_value->destroy();
        delete second_value;
        result = math_big_allocate_result(&initialization_error);
        if (result == ft_nullptr)
            return (math_big_error_result(initialization_error));
        return (result);
    }
    gcd_value = math_big_gcd_normalized(*first_value, *second_value);
    if (gcd_value == ft_nullptr)
    {
        (void)first_value->destroy();
        delete first_value;
        (void)second_value->destroy();
        delete second_value;
        return (ft_nullptr);
    }
    initialization_error = product_value.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        (void)first_value->destroy();
        delete first_value;
        (void)second_value->destroy();
        delete second_value;
        (void)gcd_value->destroy();
        delete gcd_value;
        return (math_big_error_result(initialization_error));
    }
    ft_big_number_proxy product_proxy = *first_value * *second_value;
    if (product_proxy.get_error() != FT_ERR_SUCCESS)
    {
        initialization_error = product_proxy.get_error();
        (void)first_value->destroy();
        delete first_value;
        (void)second_value->destroy();
        delete second_value;
        (void)gcd_value->destroy();
        delete gcd_value;
        return (math_big_error_result(initialization_error));
    }
    product_value = product_proxy;
    result = math_big_allocate_result(&initialization_error);
    if (result != ft_nullptr)
    {
        ft_big_number_proxy quotient_proxy = product_value / *gcd_value;

        if (quotient_proxy.get_error() == FT_ERR_SUCCESS)
            *result = quotient_proxy;
        else
            initialization_error = quotient_proxy.get_error();
    }
    (void)first_value->destroy();
    delete first_value;
    (void)second_value->destroy();
    delete second_value;
    (void)gcd_value->destroy();
    delete gcd_value;
    if (result == ft_nullptr)
        return (math_big_error_result(initialization_error));
    if (initialization_error != FT_ERR_SUCCESS)
    {
        (void)result->destroy();
        delete result;
        return (math_big_error_result(initialization_error));
    }
    return (result);
}
