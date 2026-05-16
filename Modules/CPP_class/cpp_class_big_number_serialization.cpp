#include "class_big_number.hpp"
#include "class_string.hpp"
#include <new>

ft_string *big_number_to_hex_string(const ft_big_number& number) noexcept
{
    ft_big_number number_copy;
    int32_t number_copy_initialization_error = number_copy.initialize(number);

    if (number_copy_initialization_error != FT_ERR_SUCCESS)
        return (ft_string::from_error(number_copy_initialization_error));
    ft_string *hex_string = number_copy.to_string_base(16);
    return (hex_string);
}

ft_big_number *big_number_from_hex_string(const char* hex_digits) noexcept
{
    ft_big_number *result;
    int32_t result_initialization_error;

    result = new (std::nothrow) ft_big_number();
    if (result == ft_nullptr)
        return (ft_nullptr);
    result_initialization_error = result->initialize();
    if (result_initialization_error != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }

    if (!hex_digits)
    {
        result->assign_base("", 16);
        return (result);
    }
    const char* digits = hex_digits;
    ft_bool has_negative_sign = FT_FALSE;

    if (digits[0] == '-')
    {
        has_negative_sign = FT_TRUE;
        digits++;
    }
    else if (digits[0] == '+')
        digits++;
    if (digits[0] == '0')
    {
        if (digits[1] == 'x' || digits[1] == 'X')
            digits += 2;
    }
    int32_t assign_error = result->assign_base(digits, 16);
    if (assign_error != FT_ERR_SUCCESS)
        return (result);
    if (!has_negative_sign)
        return (result);
    ft_big_number zero_reference;
    int32_t zero_reference_initialization_error;

    zero_reference_initialization_error = zero_reference.initialize();
    if (zero_reference_initialization_error != FT_ERR_SUCCESS)
        return (result);

    zero_reference.assign("0");
    *result = zero_reference - *result;
    return (result);
}
