#include "../test_internal.hpp"
#include "../../Modules/CPP_class/bitset.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
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

FT_TEST(test_template_bitset_move_constructor_preserves_thread_safety)
{
    ft_bitset source_value(12);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.enable_thread_safety());
    source_value.set(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());
    source_value.set(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());

    ft_bitset moved_value(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.move(source_value));

    FT_ASSERT_EQ(FT_TRUE, moved_value.is_thread_safe());
    FT_ASSERT_EQ(FT_TRUE, moved_value.test(3));
    FT_ASSERT_EQ(FT_TRUE, moved_value.test(9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.destroy());
    return (1);
}

FT_TEST(test_template_bitset_move_method_preserves_thread_safety)
{
    ft_bitset source_value(12);
    ft_bitset destination_value(0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.enable_thread_safety());
    source_value.set(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());
    source_value.set(8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    FT_ASSERT_EQ(FT_TRUE, destination_value.is_thread_safe());
    FT_ASSERT_EQ(FT_TRUE, destination_value.test(1));
    FT_ASSERT_EQ(FT_TRUE, destination_value.test(8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    return (1);
}
