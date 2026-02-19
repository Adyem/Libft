#include "../test_internal.hpp"
#include "../../Template/bitset.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_bitset_basic_operations, "ft_bitset set, reset, flip, and test manage bit states")
{
    ft_bitset bitset_value(16);

    FT_ASSERT_EQ(16UL, bitset_value.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());

    bitset_value.set(3);
    FT_ASSERT(bitset_value.test(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());

    bitset_value.set(7);
    FT_ASSERT(bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());

    bitset_value.reset(3);
    FT_ASSERT_EQ(false, bitset_value.test(3));
    FT_ASSERT(bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());

    bitset_value.flip(7);
    FT_ASSERT_EQ(false, bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());

    bitset_value.set(0);
    bitset_value.clear();
    FT_ASSERT_EQ(false, bitset_value.test(0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());
    return (1);
}

FT_TEST(test_ft_bitset_out_of_range_errors, "ft_bitset reports errors when accessing out of range indices")
{
    ft_bitset bitset_value(8);

    bitset_value.set(12);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_bitset::last_operation_error());

    FT_ASSERT_EQ(false, bitset_value.test(9));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_bitset::last_operation_error());

    bitset_value.reset(15);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_bitset::last_operation_error());

    bitset_value.set(4);
    FT_ASSERT(bitset_value.test(4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());
    return (1);
}

FT_TEST(test_ft_bitset_clear_and_set, "ft_bitset clear leaves future sets valid")
{
    ft_bitset bitset_value(12);

    bitset_value.set(1);
    bitset_value.set(8);
    FT_ASSERT(bitset_value.test(1));
    FT_ASSERT(bitset_value.test(8));

    bitset_value.clear();
    FT_ASSERT_EQ(false, bitset_value.test(1));
    FT_ASSERT_EQ(false, bitset_value.test(8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());

    bitset_value.set(5);
    FT_ASSERT(bitset_value.test(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());
    return (1);
}
