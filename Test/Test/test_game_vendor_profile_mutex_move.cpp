#include "../test_internal.hpp"
#include "../../Template/map.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vendor_profile_mutex_survives_moves,
        "vendor profile mutex stays valid through container moves and resizes")
{
    ft_map<int, ft_vendor_profile> container(1);
    ft_map<int, ft_vendor_profile> moved;
    ft_vendor_profile first_vendor(3, 1.10, 0.80, 0.05);
    ft_vendor_profile second_vendor(7, 1.20, 0.70, 0.06);
    Pair<int, ft_vendor_profile> *first_entry;
    Pair<int, ft_vendor_profile> *second_entry;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, container.initialize());
    container.insert(3, ft_move(first_vendor));
    container.insert(7, ft_move(second_vendor));

    first_entry = container.find(3);
    second_entry = container.find(7);
    FT_ASSERT(first_entry != ft_nullptr);
    FT_ASSERT(second_entry != ft_nullptr);
    FT_ASSERT_EQ(3, first_entry->value.get_vendor_id());
    FT_ASSERT_EQ(7, second_entry->value.get_vendor_id());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.move_from(container));
    first_entry = moved.find(3);
    second_entry = moved.find(7);
    FT_ASSERT(first_entry != ft_nullptr);
    FT_ASSERT(second_entry != ft_nullptr);
    FT_ASSERT_EQ(3, first_entry->value.get_vendor_id());
    FT_ASSERT_EQ(7, second_entry->value.get_vendor_id());

    return (1);
}
