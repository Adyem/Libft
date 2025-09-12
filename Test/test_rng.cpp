#include "../RNG/rng.hpp"
#include "../RNG/rng_internal.hpp"
#include "../System_utils/test_runner.hpp"
#include <cstdlib>

FT_TEST(test_rng_random_normal, "ft_random_normal mean")
{
    g_srand_init = true;
    srand(123);
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
    g_srand_init = true;
    srand(123);
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
    g_srand_init = true;
    srand(123);
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
    g_srand_init = true;
    srand(123);
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
    g_srand_init = true;
    srand(123);
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
