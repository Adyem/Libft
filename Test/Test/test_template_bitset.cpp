#include "../test_internal.hpp"
#include "../../Template/bitset.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <utility>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_bitset_basic_operations, "ft_bitset set, reset, flip, and test manage bit states")
{
    ft_bitset bitset_value(16);

    FT_ASSERT_EQ(16UL, bitset_value.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());

    bitset_value.set(3);
    FT_ASSERT(bitset_value.test(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());

    bitset_value.set(7);
    FT_ASSERT(bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());

    bitset_value.reset(3);
    FT_ASSERT_EQ(false, bitset_value.test(3));
    FT_ASSERT(bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());

    bitset_value.flip(7);
    FT_ASSERT_EQ(false, bitset_value.test(7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());

    bitset_value.set(0);
    bitset_value.clear();
    FT_ASSERT_EQ(false, bitset_value.test(0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());
    return (1);
}

FT_TEST(test_ft_bitset_out_of_range_errors, "ft_bitset reports errors when accessing out of range indices")
{
    ft_bitset bitset_value(8);

    bitset_value.set(12);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, bitset_value.get_error());

    FT_ASSERT_EQ(false, bitset_value.test(9));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, bitset_value.get_error());

    bitset_value.reset(15);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, bitset_value.get_error());

    bitset_value.set(4);
    FT_ASSERT(bitset_value.test(4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bitset_value.get_error());
    return (1);
}

FT_TEST(test_ft_bitset_move_transfers_storage, "ft_bitset move operations transfer storage without data loss")
{
    ft_bitset source_bitset(12);

    source_bitset.set(1);
    source_bitset.set(8);
    FT_ASSERT(source_bitset.test(1));
    FT_ASSERT(source_bitset.test(8));

    ft_bitset moved_bitset(std::move(source_bitset));

    FT_ASSERT_EQ(12UL, moved_bitset.size());
    FT_ASSERT(moved_bitset.test(1));
    FT_ASSERT(moved_bitset.test(8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_bitset.get_error());

    FT_ASSERT_EQ(0UL, source_bitset.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_bitset.get_error());

    ft_bitset assigned_bitset(3);
    assigned_bitset = std::move(moved_bitset);

    FT_ASSERT_EQ(12UL, assigned_bitset.size());
    FT_ASSERT(assigned_bitset.test(1));
    FT_ASSERT(assigned_bitset.test(8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned_bitset.get_error());
    FT_ASSERT_EQ(0UL, moved_bitset.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_bitset.get_error());
    return (1);
}
