#include "../../Game/game_resistance.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_game_resistance_default_initialization, "Game: resistance defaults to zeroed values")
{
    ft_resistance resistance;

    FT_ASSERT_EQ(0, resistance.get_percent());
    FT_ASSERT_EQ(0, resistance.get_flat());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, resistance.get_error());
    return (1);
}

FT_TEST(test_game_resistance_set_percent_resets_errno, "Game: set_percent sets errno to success")
{
    ft_resistance resistance;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_ER_SUCCESSS, resistance.set_percent(15));
    FT_ASSERT_EQ(15, resistance.get_percent());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_resistance_set_flat_resets_errno, "Game: set_flat sets errno to success")
{
    ft_resistance resistance;

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    FT_ASSERT_EQ(FT_ER_SUCCESSS, resistance.set_flat(6));
    FT_ASSERT_EQ(6, resistance.get_flat());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_resistance_set_values_updates_both, "Game: set_values replaces both percent and flat values")
{
    ft_resistance resistance;

    FT_ASSERT_EQ(FT_ER_SUCCESSS, resistance.set_values(20, 8));
    FT_ASSERT_EQ(20, resistance.get_percent());
    FT_ASSERT_EQ(8, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_reset_clears_after_updates, "Game: reset clears previously assigned values")
{
    ft_resistance resistance;

    resistance.set_values(12, 4);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, resistance.reset());
    FT_ASSERT_EQ(0, resistance.get_percent());
    FT_ASSERT_EQ(0, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_get_percent_sets_errno_success, "Game: get_percent resets errno to success after read")
{
    ft_resistance resistance;

    resistance.set_percent(9);
    ft_errno = FT_ERR_INVALID_POINTER;
    FT_ASSERT_EQ(9, resistance.get_percent());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_resistance_get_flat_sets_errno_success, "Game: get_flat resets errno to success after read")
{
    ft_resistance resistance;

    resistance.set_flat(13);
    ft_errno = FT_ERR_NOT_FOUND;
    FT_ASSERT_EQ(13, resistance.get_flat());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_resistance_get_error_sets_errno_success, "Game: get_error clears errno to success")
{
    ft_resistance resistance;

    resistance.set_values(5, 2);
    ft_errno = FT_ERR_INVALID_POINTER;
    FT_ASSERT_EQ(FT_ER_SUCCESSS, resistance.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_resistance_get_error_str_reports_success, "Game: get_error_str returns success string and resets errno")
{
    ft_resistance resistance;

    resistance.set_values(3, 1);
    ft_errno = FT_ERR_GAME_INVALID_MOVE;
    FT_ASSERT_STR_EQ(ft_strerror(FT_ER_SUCCESSS), resistance.get_error_str());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_resistance_copy_constructor_clones_values, "Game: copy constructor duplicates values and error state")
{
    ft_resistance original;

    original.set_values(18, 7);
    ft_resistance copy(original);
    FT_ASSERT_EQ(18, copy.get_percent());
    FT_ASSERT_EQ(7, copy.get_flat());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copy.get_error());
    FT_ASSERT_EQ(18, original.get_percent());
    FT_ASSERT_EQ(7, original.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_copy_assignment_overwrites_destination, "Game: copy assignment replaces destination values")
{
    ft_resistance source;
    ft_resistance destination;

    source.set_values(22, 5);
    destination.set_values(4, 1);
    destination = source;
    FT_ASSERT_EQ(22, destination.get_percent());
    FT_ASSERT_EQ(5, destination.get_flat());
    FT_ASSERT_EQ(22, source.get_percent());
    FT_ASSERT_EQ(5, source.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_move_constructor_transfers_values, "Game: move constructor transfers values and clears origin")
{
    ft_resistance original;

    original.set_values(14, 9);
    ft_resistance moved(ft_move(original));
    FT_ASSERT_EQ(14, moved.get_percent());
    FT_ASSERT_EQ(9, moved.get_flat());
    FT_ASSERT_EQ(0, original.get_percent());
    FT_ASSERT_EQ(0, original.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_move_assignment_transfers_and_resets, "Game: move assignment transfers values and resets origin")
{
    ft_resistance source;
    ft_resistance destination;

    source.set_values(11, 6);
    destination.set_values(2, 1);
    destination = ft_move(source);
    FT_ASSERT_EQ(11, destination.get_percent());
    FT_ASSERT_EQ(6, destination.get_flat());
    FT_ASSERT_EQ(0, source.get_percent());
    FT_ASSERT_EQ(0, source.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_self_copy_assignment_noop, "Game: self copy assignment leaves values unchanged")
{
    ft_resistance resistance;

    resistance.set_values(7, 3);
    resistance = resistance;
    FT_ASSERT_EQ(7, resistance.get_percent());
    FT_ASSERT_EQ(3, resistance.get_flat());
    return (1);
}

FT_TEST(test_game_resistance_self_move_assignment_noop, "Game: self move assignment leaves values unchanged")
{
    ft_resistance resistance;

    resistance.set_values(16, 12);
    resistance = ft_move(resistance);
    FT_ASSERT_EQ(16, resistance.get_percent());
    FT_ASSERT_EQ(12, resistance.get_flat());
    return (1);
}
