#include "../test_internal.hpp"
#include "../../Template/pair.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_pair_getters_and_setters_mutate_values,
        "Pair getters and setters reflect stored key/value data")
{
    Pair<int, int> pair_instance;

    FT_ASSERT_EQ(0, pair_instance.key);
    FT_ASSERT_EQ(0, pair_instance.value);
    FT_ASSERT_EQ(0, pair_instance.get_key());
    FT_ASSERT_EQ(0, pair_instance.get_value());

    pair_instance.set_key(5);
    pair_instance.set_value(10);

    FT_ASSERT_EQ(5, pair_instance.key);
    FT_ASSERT_EQ(10, pair_instance.value);
    FT_ASSERT_EQ(5, pair_instance.get_key());
    FT_ASSERT_EQ(10, pair_instance.get_value());

    pair_instance.key = 7;
    pair_instance.value = 12;

    FT_ASSERT_EQ(7, pair_instance.get_key());
    FT_ASSERT_EQ(12, pair_instance.get_value());
    return (1);
}

FT_TEST(test_pair_direct_initialization_preserves_fields,
        "Pair constructors forward the provided arguments")
{
    Pair<int, int> initial_pair(3, 9);
    Pair<int, int> copied_pair(initial_pair);
    Pair<int, int> moved_pair(ft_move(initial_pair));

    FT_ASSERT_EQ(3, copied_pair.key);
    FT_ASSERT_EQ(9, copied_pair.value);
    FT_ASSERT_EQ(3, copied_pair.get_key());
    FT_ASSERT_EQ(9, copied_pair.get_value());

    FT_ASSERT_EQ(3, moved_pair.key);
    FT_ASSERT_EQ(9, moved_pair.value);
    FT_ASSERT_EQ(3, moved_pair.get_key());
    FT_ASSERT_EQ(9, moved_pair.get_value());
    return (1);
}
