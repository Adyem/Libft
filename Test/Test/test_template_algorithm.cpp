#include "../test_internal.hpp"
#include "../../Template/algorithm.hpp"
#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "rng_test_utils.hpp"
#include <vector>
#include <array>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_sort_orders_values, "ft_sort orders values using operator<")
{
    std::vector<int> values;
    values.push_back(5);
    values.push_back(1);
    values.push_back(4);
    values.push_back(2);
    values.push_back(3);
    ft_sort(values.begin(), values.end());
    std::array<int, 5> expected = {1, 2, 3, 4, 5};
    size_t index;

    index = 0;
    while (index < expected.size())
    {
        FT_ASSERT_EQ(expected[index], values[index]);
        index++;
    }
    return (1);
}

FT_TEST(test_ft_sort_custom_comparator, "ft_sort supports custom comparators")
{
    std::vector<int> values;
    values.push_back(2);
    values.push_back(5);
    values.push_back(1);
    values.push_back(4);
    values.push_back(3);
    ft_sort(values.begin(), values.end(), [](int left, int right){ return (left > right); });
    std::array<int, 5> expected = {5, 4, 3, 2, 1};
    size_t index;

    index = 0;
    while (index < expected.size())
    {
        FT_ASSERT_EQ(expected[index], values[index]);
        index++;
    }
    return (1);
}

FT_TEST(test_ft_binary_search_default, "ft_binary_search finds elements in ascending ranges")
{
    std::vector<int> values;
    values.push_back(1);
    values.push_back(3);
    values.push_back(5);
    values.push_back(7);
    values.push_back(9);

    FT_ASSERT(ft_binary_search(values.begin(), values.end(), 7));
    FT_ASSERT_EQ(false, ft_binary_search(values.begin(), values.end(), 4));
    return (1);
}

FT_TEST(test_ft_binary_search_custom_comparator, "ft_binary_search respects custom ordering")
{
    std::vector<int> values;
    values.push_back(9);
    values.push_back(7);
    values.push_back(5);
    values.push_back(3);
    values.push_back(1);

    FT_ASSERT(ft_binary_search(values.begin(), values.end(), 5,
            [](int left, int right){ return (left > right); }));
    FT_ASSERT_EQ(false, ft_binary_search(values.begin(), values.end(), 6,
            [](int left, int right){ return (left > right); }));
    return (1);
}

FT_TEST(test_ft_binary_search_empty_range, "ft_binary_search returns false for empty ranges")
{
    std::vector<int> values;

    FT_ASSERT_EQ(false, ft_binary_search(values.begin(), values.end(), 1));
    return (1);
}

FT_TEST(test_ft_shuffle_matches_manual_simulation, "ft_shuffle matches manual Fisher-Yates simulation with seeded RNG")
{
    std::vector<int> expected;
    expected.push_back(1);
    expected.push_back(2);
    expected.push_back(3);
    expected.push_back(4);
    expected.push_back(5);
    if (!expected.empty())
    {
        size_t iterator_index;

        ft_rng_test_seed_engine(123u, "template_algorithm_shuffle");
        iterator_index = expected.size() - 1;
        while (iterator_index > 0)
        {
            size_t random_index;

            random_index = static_cast<size_t>(ft_random_int());
            random_index %= iterator_index + 1;
            int temporary_value = expected[iterator_index];
            expected[iterator_index] = expected[random_index];
            expected[random_index] = temporary_value;
            iterator_index--;
        }
    }
    std::vector<int> shuffled;
    shuffled.push_back(1);
    shuffled.push_back(2);
    shuffled.push_back(3);
    shuffled.push_back(4);
    shuffled.push_back(5);
    ft_rng_test_seed_engine(123u, "template_algorithm_shuffle");
    ft_shuffle(shuffled.begin(), shuffled.end());
    size_t index;

    index = 0;
    while (index < shuffled.size())
    {
        FT_ASSERT_EQ(expected[index], shuffled[index]);
        index++;
    }
    return (1);
}

FT_TEST(test_ft_reverse_basic, "ft_reverse reverses the provided range")
{
    std::vector<int> values;
    values.push_back(1);
    values.push_back(2);
    values.push_back(3);
    values.push_back(4);
    values.push_back(5);
    ft_reverse(values.begin(), values.end());
    std::array<int, 5> expected = {5, 4, 3, 2, 1};
    size_t index;

    index = 0;
    while (index < expected.size())
    {
        FT_ASSERT_EQ(expected[index], values[index]);
        index++;
    }
    return (1);
}

FT_TEST(test_ft_reverse_single_element, "ft_reverse leaves single element range unchanged")
{
    std::vector<int> values;
    values.push_back(42);
    ft_reverse(values.begin(), values.end());
    FT_ASSERT_EQ(1UL, values.size());
    FT_ASSERT_EQ(42, values[0]);
    return (1);
}
