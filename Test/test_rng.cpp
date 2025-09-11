#include "../RNG/rng.hpp"
#include "../RNG/rng_internal.hpp"
#include <cstdlib>

int test_rng_random_normal(void)
{
    g_srand_init = true;
    srand(123);
    int sample_count = 10000;
    int index = 0;
    float sum = 0.0f;
    while (index < sample_count)
    {
        sum += ft_random_normal();
        index++;
    }
    float mean = sum / static_cast<float>(sample_count);
    if (mean < -0.1f)
        return (0);
    if (mean > 0.1f)
        return (0);
    return (1);
}

int test_rng_random_exponential(void)
{
    g_srand_init = true;
    srand(123);
    int sample_count = 10000;
    int index = 0;
    float sum = 0.0f;
    while (index < sample_count)
    {
        sum += ft_random_exponential(2.0f);
        index++;
    }
    float mean = sum / static_cast<float>(sample_count);
    if (mean < 0.4f)
        return (0);
    if (mean > 0.6f)
        return (0);
    return (1);
}
