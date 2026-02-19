#include "../test_internal.hpp"
#include "../../Game/game_buff.hpp"
#include "../../Game/game_debuff.hpp"
#include "../../Game/game_resistance.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_buff_rejects_negative_id, "Game: buff setter rejects negative identifiers")
{
    ft_buff buff;

    buff.set_id(-4);

    FT_ASSERT_EQ(0, buff.get_id());
    FT_ASSERT_EQ(0, buff.get_id());
    return (1);
}

FT_TEST(test_game_buff_duration_addition_rejects_negative, "Game: buff duration addition enforces non-negative increments")
{
    ft_buff buff;

    buff.add_duration(-3);

    FT_ASSERT_EQ(0, buff.get_duration());
    FT_ASSERT_EQ(0, buff.get_duration());
    return (1);
}

FT_TEST(test_game_debuff_modifier_updates_apply_deltas, "Game: debuff modifier updates accumulate changes")
{
    ft_debuff debuff;

    debuff.set_modifier2(5);
    debuff.sub_modifier2(2);
    debuff.add_modifier2(4);

    FT_ASSERT_EQ(7, debuff.get_modifier2());
    return (1);
}

FT_TEST(test_game_resistance_reset_clears_values, "Game: resistance reset restores zeroed values")
{
    ft_resistance resistance;

    resistance.set_values(25, 10);

    FT_ASSERT_EQ(25, resistance.get_percent());
    FT_ASSERT_EQ(10, resistance.get_flat());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, resistance.reset());
    FT_ASSERT_EQ(0, resistance.get_percent());
    FT_ASSERT_EQ(0, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_move_assignment_transfers_values, "Game: resistance move assignment transfers and clears state")
{
    ft_resistance source;
    ft_resistance destination;

    source.set_values(15, 3);

    destination = ft_move(source);

    FT_ASSERT_EQ(15, destination.get_percent());
    FT_ASSERT_EQ(3, destination.get_flat());
    FT_ASSERT_EQ(0, source.get_percent());
    FT_ASSERT_EQ(0, source.get_flat());
    return (1);
}
