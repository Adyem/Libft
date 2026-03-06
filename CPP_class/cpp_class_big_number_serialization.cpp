#include "class_big_number.hpp"
#include "class_string.hpp"

ft_string big_number_to_hex_string(const ft_big_number& number) noexcept
{
    ft_big_number number_copy;
    int number_copy_initialization_error = number_copy.initialize(number);

    if (number_copy_initialization_error != FT_ERR_SUCCESS)
        return (ft_string(number_copy_initialization_error));
    ft_string hex_string = number_copy.to_string_base(16);
    return (hex_string);
}

ft_big_number big_number_from_hex_string(const char* hex_digits) noexcept
{
    ft_big_number result;

    if (!hex_digits)
    {
        int assign_error = result.assign_base("", 16);
        (void)assign_error;
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
    int assign_error = result.assign_base(digits, 16);
    if (assign_error != FT_ERR_SUCCESS)
        return (result);
    if (!has_negative_sign)
        return (result);
    ft_big_number zero_reference;

    zero_reference.assign("0");
    ft_big_number signed_result = zero_reference - result;
    return (signed_result);
}
