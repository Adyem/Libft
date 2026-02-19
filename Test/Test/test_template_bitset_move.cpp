#include "../test_internal.hpp"
#include "../../Template/bitset.hpp"
 #include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_bitset_thread_safe_operations_report_success,
        "ft_bitset enables thread safety and reports success after guarded mutations")
{
    ft_bitset bitset_value(10);

    FT_ASSERT_EQ(0, bitset_value.enable_thread_safety());
    bitset_value.set(2);
    bitset_value.set(7);
    FT_ASSERT(bitset_value.is_thread_safe());
    FT_ASSERT(bitset_value.test(2));
    FT_ASSERT(bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());
    bitset_value.disable_thread_safety();
    FT_ASSERT_EQ(false, bitset_value.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_bitset_clear_then_resets_last_error,
        "ft_bitset clear resets future operations and preserves last operation success")
{
    ft_bitset bitset_value(12);

    bitset_value.set(5);
    bitset_value.set(9);
    FT_ASSERT(bitset_value.test(5));
    FT_ASSERT(bitset_value.test(9));
    bitset_value.clear();
    FT_ASSERT_EQ(false, bitset_value.test(5));
    FT_ASSERT_EQ(false, bitset_value.test(9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_bitset::last_operation_error());
    return (1);
}

FT_TEST(test_ft_bitset_out_of_range_reports_error,
        "ft_bitset reports FT_ERR_OUT_OF_RANGE when accessing invalid index")
{
    ft_bitset bitset_value(6);

    bitset_value.set(10);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_bitset::last_operation_error());
    FT_ASSERT_EQ(false, bitset_value.test(10));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_bitset::last_operation_error());
    return (1);
}
