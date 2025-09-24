#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_rng_random_int_reproducible, "ft_random_int reproducible sequences")
{
    ft_seed_random_engine(246u);
    int first_sequence[3];
    int index = 0;
    while (index < 3)
    {
        first_sequence[index] = ft_random_int();
        index = index + 1;
    }
    ft_seed_random_engine(246u);
    int second_sequence[3];
    index = 0;
    while (index < 3)
    {
        second_sequence[index] = ft_random_int();
        index = index + 1;
    }
    index = 0;
    while (index < 3)
    {
        FT_ASSERT_EQ(first_sequence[index], second_sequence[index]);
        index = index + 1;
    }
    ft_seed_random_engine(135u);
    int third_sequence[3];
    index = 0;
    while (index < 3)
    {
        third_sequence[index] = ft_random_int();
        index = index + 1;
    }
    bool difference_found = false;
    index = 0;
    while (index < 3)
    {
        if (third_sequence[index] != first_sequence[index])
            difference_found = true;
        index = index + 1;
    }
    if (difference_found == false)
        return (0);
    return (1);
}

FT_TEST(test_rng_random_float_reproducible, "ft_random_float reproducible sequences")
{
    ft_seed_random_engine(97531u);
    float first_sequence[3];
    int index = 0;
    while (index < 3)
    {
        first_sequence[index] = ft_random_float();
        if (first_sequence[index] < 0.0f)
            return (0);
        if (first_sequence[index] >= 1.0f)
            return (0);
        index = index + 1;
    }
    ft_seed_random_engine(97531u);
    float second_sequence[3];
    index = 0;
    while (index < 3)
    {
        second_sequence[index] = ft_random_float();
        float difference_value = second_sequence[index] - first_sequence[index];
        if (difference_value < 0.0f)
            difference_value = -difference_value;
        if (difference_value > 0.0000001f)
            return (0);
        if (second_sequence[index] < 0.0f)
            return (0);
        if (second_sequence[index] >= 1.0f)
            return (0);
        index = index + 1;
    }
    ft_seed_random_engine(123u);
    float third_sequence[3];
    index = 0;
    while (index < 3)
    {
        third_sequence[index] = ft_random_float();
        if (third_sequence[index] < 0.0f)
            return (0);
        if (third_sequence[index] >= 1.0f)
            return (0);
        index = index + 1;
    }
    bool difference_found = false;
    index = 0;
    while (index < 3)
    {
        float difference_value = third_sequence[index] - first_sequence[index];
        if (difference_value < 0.0f)
            difference_value = -difference_value;
        if (difference_value > 0.0000001f)
            difference_found = true;
        index = index + 1;
    }
    if (difference_found == false)
        return (0);
    return (1);
}

FT_TEST(test_rng_random_normal, "ft_random_normal mean")
{
    ft_seed_random_engine(123u);
    int sample_count = 10000;
    int index = 0;
    float sum_values = 0.0f;
    while (index < sample_count)
    {
        sum_values = sum_values + ft_random_normal();
        index = index + 1;
    }
    float mean_value = sum_values / static_cast<float>(sample_count);
    if (mean_value < -0.1f)
        return (0);
    if (mean_value > 0.1f)
        return (0);
    return (1);
}

FT_TEST(test_rng_random_exponential, "ft_random_exponential mean")
{
    ft_seed_random_engine(123u);
    int sample_count = 10000;
    int index = 0;
    float sum_values = 0.0f;
    while (index < sample_count)
    {
        sum_values = sum_values + ft_random_exponential(2.0f);
        index = index + 1;
    }
    float mean_value = sum_values / static_cast<float>(sample_count);
    if (mean_value < 0.4f)
        return (0);
    if (mean_value > 0.6f)
        return (0);
    return (1);
}

FT_TEST(test_rng_random_poisson, "ft_random_poisson mean")
{
    ft_seed_random_engine(123u);
    int sample_count = 10000;
    int index = 0;
    int sum_values = 0;
    double lambda_value = 4.0;
    while (index < sample_count)
    {
        sum_values = sum_values + ft_random_poisson(lambda_value);
        index = index + 1;
    }
    double mean_value = static_cast<double>(sum_values) /
                         static_cast<double>(sample_count);
    if (mean_value < lambda_value - 0.1)
        return (0);
    if (mean_value > lambda_value + 0.1)
        return (0);
    return (1);
}

FT_TEST(test_rng_random_binomial, "ft_random_binomial typical and edge cases")
{
    ft_seed_random_engine(123u);
    int sample_count = 10000;
    int index = 0;
    int sum_values = 0;
    int trial_count = 10;
    double success_probability = 0.5;
    while (index < sample_count)
    {
        sum_values = sum_values +
            ft_random_binomial(trial_count, success_probability);
        index = index + 1;
    }
    double mean_value = static_cast<double>(sum_values) /
                         static_cast<double>(sample_count);
    if (mean_value < 4.9)
        return (0);
    if (mean_value > 5.1)
        return (0);
    FT_ASSERT_EQ(0, ft_random_binomial(0, success_probability));
    FT_ASSERT_EQ(0, ft_random_binomial(trial_count, 0.0));
    FT_ASSERT_EQ(trial_count, ft_random_binomial(trial_count, 1.0));
    FT_ASSERT_EQ(0, ft_random_binomial(-3, success_probability));
    FT_ASSERT_EQ(0, ft_random_binomial(trial_count, -0.1));
    FT_ASSERT_EQ(0, ft_random_binomial(trial_count, 1.5));
    return (1);
}

FT_TEST(test_rng_random_geometric, "ft_random_geometric typical and edge cases")
{
    ft_seed_random_engine(123u);
    int sample_count = 10000;
    int index = 0;
    int sum_values = 0;
    double success_probability = 0.25;
    while (index < sample_count)
    {
        sum_values = sum_values + ft_random_geometric(success_probability);
        index = index + 1;
    }
    double mean_value = static_cast<double>(sum_values) /
                         static_cast<double>(sample_count);
    if (mean_value < 3.9)
        return (0);
    if (mean_value > 4.1)
        return (0);
    FT_ASSERT_EQ(0, ft_random_geometric(0.0));
    FT_ASSERT_EQ(1, ft_random_geometric(1.0));
    FT_ASSERT_EQ(0, ft_random_geometric(-0.1));
    FT_ASSERT_EQ(0, ft_random_geometric(1.5));
    return (1);
}
