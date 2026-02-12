#include "../test_internal.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#ifndef LIBFT_TEST_BUILD
#endif

#include "../../RNG/rng.hpp"
#include "../../System_utils/test_runner.hpp"
#include "rng_test_utils.hpp"

FT_TEST(test_rng_uniform_chi_squared,
        "ft_random_int uniform distribution stays within chi-squared tolerance")
{
    const int bin_total = 10;
    const int sample_total = 20000;
    std::vector<int> histogram;
    int index;
    double max_int_value;
    double expected_count;
    double chi_squared;

    histogram.resize(bin_total);
    index = 0;
    while (index < bin_total)
    {
        histogram[index] = 0;
        index = index + 1;
    }
    ft_rng_test_seed_engine(123456u, "rng_uniform_chi_squared");
    max_int_value = static_cast<double>(std::numeric_limits<int>::max());
    index = 0;
    while (index < sample_total)
    {
        int     value;
        double  ratio;
        int     bucket_index;

        value = ft_random_int();
        ratio = static_cast<double>(value) / (max_int_value + 1.0);
        bucket_index = static_cast<int>(ratio * static_cast<double>(bin_total));
        if (bucket_index < 0)
            bucket_index = 0;
        if (bucket_index >= bin_total)
            bucket_index = bin_total - 1;
        histogram[bucket_index] = histogram[bucket_index] + 1;
        index = index + 1;
    }
    expected_count = static_cast<double>(sample_total) / static_cast<double>(bin_total);
    chi_squared = 0.0;
    index = 0;
    while (index < bin_total)
    {
        double difference;

        difference = static_cast<double>(histogram[index]) - expected_count;
        chi_squared = chi_squared + ((difference * difference) / expected_count);
        index = index + 1;
    }
    if (chi_squared > 21.666)
        return (0);
    return (1);
}

FT_TEST(test_rng_normal_kolmogorov_smirnov,
        "ft_random_normal samples keep KS distance below 0.05")
{
    const int sample_total = 4096;
    std::vector<float> samples;
    int index;
    double max_difference;

    samples.resize(sample_total);
    ft_rng_test_seed_engine(654321u, "rng_normal_kolmogorov_smirnov");
    index = 0;
    while (index < sample_total)
    {
        samples[index] = ft_random_normal();
        index = index + 1;
    }
    std::sort(samples.begin(), samples.end());
    max_difference = 0.0;
    index = 0;
    while (index < sample_total)
    {
        double empirical_cdf;
        double theoretical_cdf;
        double difference;

        empirical_cdf = static_cast<double>(index + 1) / static_cast<double>(sample_total);
        theoretical_cdf = 0.5 * (1.0 + std::erf(static_cast<double>(samples[index]) / std::sqrt(2.0)));
        difference = empirical_cdf - theoretical_cdf;
        if (difference < 0.0)
            difference = -difference;
        if (difference > max_difference)
            max_difference = difference;
        index = index + 1;
    }
    if (max_difference > 0.05)
        return (0);
    return (1);
}
