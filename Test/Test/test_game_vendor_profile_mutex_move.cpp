#include "../test_internal.hpp"
#include "../../Modules/Template/map.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vendor_profile_mutex_survives_moves)
{
    ft_map<int, game_vendor_profile> container(1);
    ft_map<int, game_vendor_profile> moved;
    game_vendor_profile first_vendor;
    game_vendor_profile second_vendor;
    Pair<int, game_vendor_profile> *first_entry;
    Pair<int, game_vendor_profile> *second_entry;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, container.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_vendor.initialize(3, 1.10, 0.80, 0.05));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_vendor.initialize(7, 1.20, 0.70, 0.06));
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
