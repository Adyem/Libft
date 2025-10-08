#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "rng_test_utils.hpp"

FT_TEST(test_rng_seed_override_from_environment, "ft_rng_test_seed_engine mixes FT_RNG_TEST_SEED with defaults")
{
    const char  *environment_name = "FT_RNG_TEST_SEED";
    char        *original_value;
    ft_string    original_copy;
    bool        restore_original = false;
    uint32_t    baseline_seed;
    uint32_t    override_seed;
    uint32_t    expected_override_seed;
    int         first_sample;
    int         second_sample;

    original_value = ft_getenv(environment_name);
    if (original_value != ft_nullptr && *original_value != '\0')
    {
        original_copy = original_value;
        if (original_copy.get_error() != ER_SUCCESS)
            return (0);
        restore_original = true;
    }
    ft_unsetenv(environment_name);
    baseline_seed = ft_rng_test_seed_value(100u, "baseline");
    FT_ASSERT_EQ(100u, baseline_seed);
    expected_override_seed = ft_random_seed("override");
    expected_override_seed = expected_override_seed ^ ft_random_seed("baseline");
    expected_override_seed = expected_override_seed ^ 100u;
    if (ft_setenv(environment_name, "override", 1) != 0)
    {
        if (restore_original == true)
            ft_setenv(environment_name, original_copy.c_str(), 1);
        else
            ft_unsetenv(environment_name);
        return (0);
    }
    override_seed = ft_rng_test_seed_value(100u, "baseline");
    FT_ASSERT_EQ(expected_override_seed, override_seed);
    ft_rng_test_seed_engine(100u, "baseline");
    first_sample = ft_random_int();
    ft_rng_test_seed_engine(100u, "baseline");
    second_sample = ft_random_int();
    FT_ASSERT_EQ(first_sample, second_sample);
    if (restore_original == true)
    {
        if (ft_setenv(environment_name, original_copy.c_str(), 1) != 0)
            return (0);
    }
    else
        ft_unsetenv(environment_name);
    return (1);
}

FT_TEST(test_rng_random_int_reproducible, "ft_random_int reproducible sequences")
{
    ft_rng_test_seed_engine(246u, "rng_random_int_first");
    int first_sequence[3];
    int index = 0;
    while (index < 3)
    {
        first_sequence[index] = ft_random_int();
        index = index + 1;
    }
    ft_rng_test_seed_engine(246u, "rng_random_int_first");
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
    ft_rng_test_seed_engine(135u, "rng_random_int_second");
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
    ft_rng_test_seed_engine(97531u, "rng_random_float_first");
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
    ft_rng_test_seed_engine(97531u, "rng_random_float_first");
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
    ft_rng_test_seed_engine(123u, "rng_random_float_second");
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
    ft_rng_test_seed_engine(123u, "rng_random_normal");
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
    ft_rng_test_seed_engine(123u, "rng_random_exponential");
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
    ft_rng_test_seed_engine(123u, "rng_random_poisson");
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
    ft_rng_test_seed_engine(123u, "rng_random_binomial");
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
    ft_rng_test_seed_engine(123u, "rng_random_geometric");
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
