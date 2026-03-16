#include "../test_internal.hpp"
#include "../../Template/pair.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_pair_copy_constructor_clones_values)
{
    Pair<int32_t, int32_t> original_pair(3, 5);
    Pair<int32_t, int32_t> copied_pair(original_pair);

    FT_ASSERT_EQ(3, copied_pair.key);
    FT_ASSERT_EQ(5, copied_pair.value);
    FT_ASSERT_EQ(3, copied_pair.get_key());
    FT_ASSERT_EQ(5, copied_pair.get_value());
    return (1);
}

FT_TEST(test_template_pair_move_constructor_transfers_values)
{
    Pair<int32_t, int32_t> source_pair(7, 9);
    Pair<int32_t, int32_t> moved_pair(ft_move(source_pair));

    FT_ASSERT_EQ(7, moved_pair.key);
    FT_ASSERT_EQ(9, moved_pair.value);
    FT_ASSERT_EQ(7, moved_pair.get_key());
    FT_ASSERT_EQ(9, moved_pair.get_value());
    return (1);
}

FT_TEST(test_template_pair_move_assignment_updates_values)
{
    Pair<int32_t, int32_t> destination_pair(2, 4);
    Pair<int32_t, int32_t> source_pair(12, 14);

    destination_pair = ft_move(source_pair);

    FT_ASSERT_EQ(12, destination_pair.key);
    FT_ASSERT_EQ(14, destination_pair.value);
    FT_ASSERT_EQ(12, destination_pair.get_key());
    FT_ASSERT_EQ(14, destination_pair.get_value());
    return (1);
}
