#include "class_big_number.hpp"

ft_string big_number_to_hex_string(const ft_big_number& number) noexcept
{
    ft_big_number number_copy(number);
    ft_string hex_string = number_copy.to_string_base(16);
    return (hex_string);
}

ft_big_number big_number_from_hex_string(const char* hex_digits) noexcept
{
    ft_big_number result;

    if (!hex_digits)
    {
        result.assign_base("", 16);
        return (result);
    }
    const char* digits = hex_digits;
    bool has_negative_sign = false;

    if (digits[0] == '-')
    {
        has_negative_sign = true;
        digits++;
    }
    else if (digits[0] == '+')
        digits++;
    if (digits[0] == '0')
    {
        if (digits[1] == 'x' || digits[1] == 'X')
            digits += 2;
    }
    result.assign_base(digits, 16);
    if (ft_big_number::last_operation_error() != 0)
        return (result);
    if (!has_negative_sign)
        return (result);
    ft_big_number zero_reference;

    zero_reference.assign("0");
    if (ft_big_number::last_operation_error() != 0)
        return (zero_reference);
    ft_big_number signed_result = zero_reference - result;
    return (signed_result);
}
