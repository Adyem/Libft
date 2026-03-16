#include "../test_internal.hpp"
#include "../../CPP_class/bitset.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_bitset_thread_safety_contract_enable_disable)
{
    ft_bitset bitset_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.initialize(10));
    FT_ASSERT_EQ(FT_FALSE, bitset_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, bitset_value.is_thread_safe());
    bitset_value.set(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());
    bitset_value.set(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());
    FT_ASSERT_EQ(FT_TRUE, bitset_value.test(2));
    FT_ASSERT_EQ(FT_TRUE, bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, bitset_value.is_thread_safe());
    return (1);
}

FT_TEST(test_template_bitset_thread_safety_contract_out_of_range_sets_error)
{
    ft_bitset bitset_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.initialize(6));
    bitset_value.set(10);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, bitset_value.get_error());
    return (1);
}
