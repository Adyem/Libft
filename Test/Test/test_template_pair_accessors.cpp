#include "../test_internal.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_pair_accessors_mutate_values)
{
    Pair<int32_t, int32_t> pair_instance;

    FT_ASSERT_EQ(0, pair_instance.key);
    FT_ASSERT_EQ(0, pair_instance.value);

    pair_instance.set_key(5);
    pair_instance.set_value(10);

    FT_ASSERT_EQ(5, pair_instance.get_key());
    FT_ASSERT_EQ(10, pair_instance.get_value());
    return (1);
}

FT_TEST(test_template_pair_accessors_constructors_preserve_values)
{
    Pair<int32_t, int32_t> initial_pair(3, 9);
    Pair<int32_t, int32_t> copied_pair(initial_pair);
    Pair<int32_t, int32_t> moved_pair(ft_move(initial_pair));

    FT_ASSERT_EQ(3, copied_pair.get_key());
    FT_ASSERT_EQ(9, copied_pair.get_value());
    FT_ASSERT_EQ(3, moved_pair.get_key());
    FT_ASSERT_EQ(9, moved_pair.get_value());
    return (1);
}
