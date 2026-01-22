#include "../../Template/pair.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_pair_copy_constructor_clones_values,
        "Pair copy constructor duplicates the stored key/value")
{
    Pair<int, int> original_pair(3, 5);
    Pair<int, int> copied_pair(original_pair);

    FT_ASSERT_EQ(3, copied_pair.key);
    FT_ASSERT_EQ(5, copied_pair.value);
    FT_ASSERT_EQ(3, copied_pair.get_key());
    FT_ASSERT_EQ(5, copied_pair.get_value());
    return (1);
}

FT_TEST(test_pair_move_constructor_transfers_values,
        "Pair move constructor steals the stored key/value")
{
    Pair<int, int> source_pair(7, 9);
    Pair<int, int> moved_pair(ft_move(source_pair));

    FT_ASSERT_EQ(7, moved_pair.key);
    FT_ASSERT_EQ(9, moved_pair.value);
    FT_ASSERT_EQ(7, moved_pair.get_key());
    FT_ASSERT_EQ(9, moved_pair.get_value());
    return (1);
}

FT_TEST(test_pair_move_assignment_updates_values,
        "Pair move assignment replaces the stored key/value")
{
    Pair<int, int> destination_pair(2, 4);
    Pair<int, int> source_pair(12, 14);

    destination_pair = ft_move(source_pair);

    FT_ASSERT_EQ(12, destination_pair.key);
    FT_ASSERT_EQ(14, destination_pair.value);
    FT_ASSERT_EQ(12, destination_pair.get_key());
    FT_ASSERT_EQ(14, destination_pair.get_value());
    return (1);
}
