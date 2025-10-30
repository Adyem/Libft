#include "../../RNG/rng.hpp"
#include "../../System_utils/test_runner.hpp"
#include "rng_test_utils.hpp"

FT_TEST(test_rng_gamma_pdf_cdf_reference,
        "rng_gamma_pdf and rng_gamma_cdf track reference values")
{
    double pdf_value;
    double cdf_value;
    double expected_pdf;
    double expected_cdf;
    double difference;

    expected_pdf = 0.19196788093577974;
    expected_cdf = 0.45058404864721977;
    pdf_value = rng_gamma_pdf(2.5, 1.5, 3.0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    difference = pdf_value - expected_pdf;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.000001)
        return (0);
    cdf_value = rng_gamma_cdf(2.5, 1.5, 3.0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    difference = cdf_value - expected_cdf;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.01)
        return (0);
    return (1);
}

FT_TEST(test_rng_beta_pdf_cdf_reference,
        "rng_beta_pdf and rng_beta_cdf match precomputed reference values")
{
    double pdf_value;
    double cdf_value;
    double expected_pdf;
    double expected_cdf;
    double difference;

    expected_pdf = 1.5552;
    expected_cdf = 0.76672;
    pdf_value = rng_beta_pdf(2.0, 5.0, 0.4);
    if (ft_errno != ER_SUCCESS)
        return (0);
    difference = pdf_value - expected_pdf;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.0001)
        return (0);
    cdf_value = rng_beta_cdf(2.0, 5.0, 0.4);
    if (ft_errno != ER_SUCCESS)
        return (0);
    difference = cdf_value - expected_cdf;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.01)
        return (0);
    return (1);
}

FT_TEST(test_rng_chi_squared_pdf_cdf_reference,
        "rng_chi_squared_pdf and rng_chi_squared_cdf align with gamma expectations")
{
    double pdf_value;
    double cdf_value;
    double expected_pdf;
    double expected_cdf;
    double difference;

    expected_pdf = 0.08790080915922291;
    expected_cdf = 0.7602705204748466;
    pdf_value = rng_chi_squared_pdf(4.0, 5.5);
    if (ft_errno != ER_SUCCESS)
        return (0);
    difference = pdf_value - expected_pdf;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.0001)
        return (0);
    cdf_value = rng_chi_squared_cdf(4.0, 5.5);
    if (ft_errno != ER_SUCCESS)
        return (0);
    difference = cdf_value - expected_cdf;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.01)
        return (0);
    return (1);
}

FT_TEST(test_rng_gamma_sampling_mean,
        "ft_random_gamma produces samples whose mean approximates shape * scale")
{
    const int sample_total = 4096;
    double sum;
    int index;
    double mean_value;
    double difference;

    ft_rng_test_seed_engine(424242u, "rng_gamma_sampling_mean");
    sum = 0.0;
    index = 0;
    while (index < sample_total)
    {
        sum = sum + static_cast<double>(ft_random_gamma(2.0f, 3.0f));
        if (ft_errno != ER_SUCCESS)
            return (0);
        index = index + 1;
    }
    mean_value = sum / static_cast<double>(sample_total);
    difference = mean_value - 6.0;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.2)
        return (0);
    return (1);
}

FT_TEST(test_rng_beta_sampling_mean,
        "ft_random_beta produces samples whose mean approximates alpha/(alpha+beta)")
{
    const int sample_total = 4096;
    double sum;
    int index;
    double mean_value;
    double expected;
    double difference;

    ft_rng_test_seed_engine(987654u, "rng_beta_sampling_mean");
    sum = 0.0;
    index = 0;
    while (index < sample_total)
    {
        sum = sum + static_cast<double>(ft_random_beta(2.0f, 5.0f));
        if (ft_errno != ER_SUCCESS)
            return (0);
        index = index + 1;
    }
    mean_value = sum / static_cast<double>(sample_total);
    expected = 2.0 / 7.0;
    difference = mean_value - expected;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.02)
        return (0);
    return (1);
}

FT_TEST(test_rng_chi_squared_sampling_mean,
        "ft_random_chi_squared produces samples whose mean approximates the degrees of freedom")
{
    const int sample_total = 4096;
    double sum;
    int index;
    double mean_value;
    double difference;

    ft_rng_test_seed_engine(246813u, "rng_chi_squared_sampling_mean");
    sum = 0.0;
    index = 0;
    while (index < sample_total)
    {
        sum = sum + static_cast<double>(ft_random_chi_squared(5.0f));
        if (ft_errno != ER_SUCCESS)
            return (0);
        index = index + 1;
    }
    mean_value = sum / static_cast<double>(sample_total);
    difference = mean_value - 5.0;
    if (difference < 0.0)
        difference = -difference;
    if (difference > 0.3)
        return (0);
    return (1);
}
