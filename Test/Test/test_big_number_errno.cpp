#include "../../CPP_class/class_big_number.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/move.hpp"
#include <cstring>

FT_TEST(test_big_number_errno_resets_construction_and_assignment, "ft_big_number constructors preserve ft_errno and use ft_sys_errno")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    {
        ft_big_number default_number;

        FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
        FT_ASSERT_EQ(FT_SYS_ERR_INVALID_STATE, ft_sys_errno);
        FT_ASSERT_EQ(FT_ERR_SUCCESSS, default_number.get_error());
    }

    ft_big_number seeded_number;

    seeded_number.assign("123");
    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number copied_number(seeded_number);
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copied_number.get_error());

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number moved_number(ft_move(copied_number));
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved_number.get_error());

    ft_big_number assign_target;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    assign_target = seeded_number;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, assign_target.get_error());

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    assign_target = ft_move(moved_number);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, assign_target.get_error());
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_errno_resets_mutators, "ft_big_number mutators clear stale ft_errno on success")
{
    ft_big_number number;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    number.assign("4567");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    number.assign_base("1111", 2);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    number.append_digit('8');
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    number.append("90");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    number.append_unsigned(12);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    number.trim_leading_zeros();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    number.reduce_to(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    number.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, number.get_error());
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_errno_resets_accessors, "ft_big_number accessors and comparisons preserve ft_errno and reset ft_sys_errno")
{
    ft_big_number left_number;
    ft_big_number right_number;

    left_number.assign("50");
    right_number.assign("25");

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(left_number == left_number);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(left_number != right_number);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(left_number > right_number);
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(left_number >= right_number);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(left_number >= left_number);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(!(left_number < ft_big_number()));
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(right_number <= left_number);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), right_number.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(!right_number.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(!right_number.is_negative());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT(right_number.is_positive());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT_EQ(0, std::strcmp(right_number.c_str(), "25"));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, right_number.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    FT_ASSERT_EQ(0, std::strcmp(right_number.get_error_str(), "Success"));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    ft_errno = 0;
    ft_sys_errno = FT_SYS_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_big_number_errno_resets_arithmetic, "ft_big_number arithmetic clears stale ft_errno on success")
{
    ft_big_number left_number;
    ft_big_number right_number;
    ft_big_number base_number;
    ft_big_number exponent_number;
    ft_big_number modulus_number;

    left_number.assign("20");
    right_number.assign("5");
    base_number.assign("4");
    exponent_number.assign("3");
    modulus_number.assign("7");

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number sum_number = left_number + right_number;
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(0, std::strcmp(sum_number.c_str(), "25"));

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number difference_number = left_number - right_number;
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(0, std::strcmp(difference_number.c_str(), "15"));

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number product_number = left_number * right_number;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(0, std::strcmp(product_number.c_str(), "100"));

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number quotient_number = left_number / right_number;
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(0, std::strcmp(quotient_number.c_str(), "4"));

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number remainder_number = left_number % right_number;
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(0, std::strcmp(remainder_number.c_str(), "0"));

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number power_number = base_number.mod_pow(exponent_number, modulus_number);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(0, std::strcmp(power_number.c_str(), "1"));

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_string base16_string = left_number.to_string_base(16);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_SYS_ERR_SUCCESS, ft_sys_errno);
    FT_ASSERT_EQ(0, base16_string.get_error());
    ft_errno = 0;
    ft_sys_errno = FT_SYS_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_big_number_errno_resets_hex_helpers, "ft_big_number hex helpers clear stale ft_errno")
{
    ft_big_number number_value;

    number_value.assign("255");

    ft_errno = FT_ERR_DIVIDE_BY_ZERO;
    ft_string hex_digits = big_number_to_hex_string(number_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(0, hex_digits.get_error());
    FT_ASSERT(hex_digits == "FF");

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_big_number parsed_value = big_number_from_hex_string(hex_digits.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, parsed_value.get_error());
    FT_ASSERT_EQ(0, std::strcmp(parsed_value.c_str(), "255"));
    ft_errno = 0;
    return (1);
}
