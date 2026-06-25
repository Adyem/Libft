#include "../test_internal.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rarity_band_default_values)
{
    game_rarity_band rarity_band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    FT_ASSERT_EQ(0, rarity_band.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    FT_ASSERT_EQ(1.0, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    return (1);
}

FT_TEST(test_rarity_band_initialize_values)
{
    game_rarity_band rarity_band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.initialize(3, 2.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());

    FT_ASSERT_EQ(3, rarity_band.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    FT_ASSERT_EQ(2.5, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    return (1);
}

FT_TEST(test_rarity_band_setters_overwrite_values)
{
    game_rarity_band rarity_band;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    rarity_band.set_rarity(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    FT_ASSERT_EQ(5, rarity_band.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    rarity_band.set_rarity(8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    FT_ASSERT_EQ(8, rarity_band.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    rarity_band.set_value_multiplier(1.5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    FT_ASSERT_EQ(1.5, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    rarity_band.set_value_multiplier(0.75);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    FT_ASSERT_EQ(0.75, rarity_band.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rarity_band.get_error());
    return (1);
}

FT_TEST(test_rarity_band_initialize_copy_matches_source)
{
    game_rarity_band source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    game_rarity_band destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());

    FT_ASSERT_EQ(7, destination.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(4.0, destination.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(7, source.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(4.0, source.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_rarity_band_initialize_overwrites_previous_values)
{
    game_rarity_band source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(4, 1.25));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    game_rarity_band destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(2, 0.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());

    FT_ASSERT_EQ(4, destination.get_rarity());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(1.25, destination.get_value_multiplier());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    return (1);
}
