#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Basic/basic.hpp"
#include "rng_test_utils.hpp"
#include <limits>
#include <random>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_dice_roll_deterministic_sum, "ft_dice_roll produces reproducible sums with seeded engine")
{
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    int expected_result;
    int actual_result;
    int index;

    ft_rng_test_seed_engine(1337u, "rng_dice_roll_sequence");
    actual_result = ft_dice_roll(4, 6);

    ft_rng_test_seed_engine(1337u, "rng_dice_roll_sequence");
    expected_result = 0;
    index = 0;
    while (index < 4)
    {
        int roll_value;

        roll_value = distribution(g_random_engine);
        expected_result += (roll_value % 6) + 1;
        index++;
    }
    FT_ASSERT_EQ(expected_result, actual_result);
    return (1);
}

FT_TEST(test_ft_dice_roll_zero_arguments_success, "ft_dice_roll returns zero when both parameters are zero")
{
    int result;

    result = ft_dice_roll(0, 0);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_ft_dice_roll_invalid_arguments, "ft_dice_roll rejects counts or faces below one")
{
    int result;

    result = ft_dice_roll(0, 6);
    FT_ASSERT_EQ(-1, result);

    result = ft_dice_roll(2, 0);
    FT_ASSERT_EQ(-1, result);
    return (1);
}

FT_TEST(test_ft_dice_roll_single_face_returns_number, "ft_dice_roll returns the number of dice when faces is one")
{
    int result;

    result = ft_dice_roll(5, 1);
    FT_ASSERT_EQ(5, result);
    return (1);
}
