#include "rng.hpp"
#include "rng_internal.hpp"
#include <random>
#include <immintrin.h>
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"

static void ft_random_store_int_block(int *output_values, const int *generated_values) noexcept
{
#if defined(__SSE2__)
    __m128i packed_values;

    packed_values = _mm_set_epi32(generated_values[3], generated_values[2], generated_values[1], generated_values[0]);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output_values), packed_values);
#else
    size_t store_index;

    store_index = 0;
    while (store_index < 4)
    {
        output_values[store_index] = generated_values[store_index];
        store_index += 1;
    }
#endif
    return ;
}

static void ft_random_store_float_block(float *output_values, const float *generated_values) noexcept
{
#if defined(__SSE__)
    __m128 packed_values;

    packed_values = _mm_set_ps(generated_values[3], generated_values[2], generated_values[1], generated_values[0]);
    _mm_storeu_ps(output_values, packed_values);
#else
    size_t store_index;

    store_index = 0;
    while (store_index < 4)
    {
        output_values[store_index] = generated_values[store_index];
        store_index += 1;
    }
#endif
    return ;
}

int ft_random_int_vector(int minimum_value, int maximum_value, int *output_values, size_t output_count)
{
    size_t output_index;
    int generated_values[4];

    if (output_count > 0 && output_values == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (minimum_value > maximum_value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (output_count == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_init_random_engine();
    std::uniform_int_distribution<int> distribution(minimum_value, maximum_value);
    ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);
    if (guard.get_error() != ER_SUCCESS)
        return (-1);
    output_index = 0;
    while (output_index + 4 <= output_count)
    {
        size_t fill_index;

        fill_index = 0;
        while (fill_index < 4)
        {
            generated_values[fill_index] = distribution(g_random_engine);
            fill_index += 1;
        }
        ft_random_store_int_block(output_values + output_index, generated_values);
        output_index += 4;
    }
    while (output_index < output_count)
    {
        output_values[output_index] = distribution(g_random_engine);
        output_index += 1;
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int ft_random_float_vector(float *output_values, size_t output_count)
{
    size_t output_index;
    float generated_values[4];

    if (output_count > 0 && output_values == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (output_count == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_init_random_engine();
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);
    if (guard.get_error() != ER_SUCCESS)
        return (-1);
    output_index = 0;
    while (output_index + 4 <= output_count)
    {
        size_t fill_index;

        fill_index = 0;
        while (fill_index < 4)
        {
            generated_values[fill_index] = distribution(g_random_engine);
            fill_index += 1;
        }
        ft_random_store_float_block(output_values + output_index, generated_values);
        output_index += 4;
    }
    while (output_index < output_count)
    {
        output_values[output_index] = distribution(g_random_engine);
        output_index += 1;
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
