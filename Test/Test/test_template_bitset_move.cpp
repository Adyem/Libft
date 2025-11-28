#include "../../Template/bitset.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_bitset_move_constructor_rebuilds_mutex,
        "ft_bitset move constructor reinstates thread-safety with fresh state mutex")
{
    ft_bitset source_bits(8);

    FT_ASSERT_EQ(0, source_bits.enable_thread_safety());
    source_bits.set(2);
    source_bits.set(4);
    FT_ASSERT(source_bits.is_thread_safe_enabled());

    ft_bitset moved_bits(ft_move(source_bits));

    FT_ASSERT(moved_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, moved_bits.test(2));
    FT_ASSERT_EQ(true, moved_bits.test(4));
    FT_ASSERT_EQ(ER_SUCCESS, moved_bits.get_error());
    return (1);
}

FT_TEST(test_ft_bitset_move_assignment_rebuilds_mutex,
        "ft_bitset move assignment reinstates thread-safety with fresh state mutex")
{
    ft_bitset destination_bits(4);
    ft_bitset source_bits(6);

    FT_ASSERT_EQ(0, destination_bits.enable_thread_safety());
    destination_bits.set(1);
    FT_ASSERT(destination_bits.is_thread_safe_enabled());

    FT_ASSERT_EQ(0, source_bits.enable_thread_safety());
    source_bits.set(0);
    source_bits.set(5);
    FT_ASSERT(source_bits.is_thread_safe_enabled());

    destination_bits = ft_move(source_bits);

    FT_ASSERT(destination_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, destination_bits.test(0));
    FT_ASSERT_EQ(true, destination_bits.test(5));
    FT_ASSERT_EQ(6UL, destination_bits.size());
    FT_ASSERT_EQ(ER_SUCCESS, destination_bits.get_error());
    return (1);
}

FT_TEST(test_ft_bitset_move_preserves_bit_data_without_thread_safety,
        "ft_bitset move constructor keeps thread-safety disabled while copying bits")
{
    ft_bitset source_bits(5);

    source_bits.set(1);
    source_bits.set(3);
    FT_ASSERT_EQ(false, source_bits.is_thread_safe_enabled());

    ft_bitset moved_bits(ft_move(source_bits));

    FT_ASSERT_EQ(false, moved_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, moved_bits.test(1));
    FT_ASSERT_EQ(true, moved_bits.test(3));
    FT_ASSERT_EQ(5UL, moved_bits.size());
    return (1);
}

FT_TEST(test_ft_bitset_move_reports_out_of_range_when_reused_without_resize,
        "ft_bitset moved-from instance stays valid but empty until resized")
{
    ft_bitset source_bits(3);
    ft_bitset moved_bits(1);

    FT_ASSERT_EQ(0, source_bits.enable_thread_safety());
    source_bits.set(2);
    FT_ASSERT(source_bits.is_thread_safe_enabled());

    moved_bits = ft_move(source_bits);

    FT_ASSERT(moved_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_bits.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, moved_bits.test(2));
    FT_ASSERT_EQ(0UL, source_bits.size());
    FT_ASSERT_EQ(0, source_bits.enable_thread_safety());
    FT_ASSERT(source_bits.is_thread_safe_enabled());
    source_bits.set(0);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, source_bits.get_error());
    FT_ASSERT_EQ(false, source_bits.test(0));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, source_bits.get_error());
    return (1);
}
