#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../RNG/rng_stream.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rng_stream_independent_seeds, "rng_stream instances keep independent seeds")
{
    rng_stream first_stream(123u);
    rng_stream second_stream(456u);
    int first_values[3];
    int second_values[3];
    int index;
    Pair<int, int> first_result;
    Pair<int, int> second_result;

    if (first_stream.initialize() != FT_ERR_SUCCESS)
        return (0);
    if (second_stream.initialize() != FT_ERR_SUCCESS)
        return (0);
    index = 0;
    while (index < 3)
    {
        first_result = first_stream.random_int();
        second_result = second_stream.random_int();
        if (first_result.key != FT_ERR_SUCCESS || second_result.key != FT_ERR_SUCCESS)
            return (0);
        first_values[index] = first_result.value;
        second_values[index] = second_result.value;
        index = index + 1;
    }
    index = 0;
    while (index < 3)
    {
        if (first_values[index] != second_values[index])
            return (1);
        index = index + 1;
    }
    return (0);
}

FT_TEST(test_rng_stream_reseed_from_string, "rng_stream can reseed from string and validate arguments")
{
    rng_stream stream_instance(0u);
    int initial_value;
    int reseeded_value;
    Pair<int, int> value_result;
    int reseed_error;

    if (stream_instance.initialize() != FT_ERR_SUCCESS)
        return (0);
    value_result = stream_instance.random_int();
    if (value_result.key != FT_ERR_SUCCESS)
        return (0);
    initial_value = value_result.value;
    reseed_error = stream_instance.reseed_from_string("example-seed");
    if (reseed_error != FT_ERR_SUCCESS)
        return (0);
    value_result = stream_instance.random_int();
    if (value_result.key != FT_ERR_SUCCESS)
        return (0);
    reseeded_value = value_result.value;
    if (initial_value == reseeded_value)
        return (0);
    reseed_error = stream_instance.reseed_from_string(ft_nullptr);
    if (reseed_error != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_rng_stream_copy_preserves_state, "rng_stream copy produces identical subsequent samples")
{
    rng_stream original_stream(987u);
    rng_stream copied_stream;
    int first_sample;
    int second_sample_original;
    int second_sample_copy;
    Pair<int, int> sample_result;

    if (original_stream.initialize() != FT_ERR_SUCCESS)
        return (0);
    sample_result = original_stream.random_int();
    if (sample_result.key != FT_ERR_SUCCESS)
        return (0);
    first_sample = sample_result.value;
    if (copied_stream.initialize(original_stream) != FT_ERR_SUCCESS)
        return (0);
    sample_result = original_stream.random_int();
    if (sample_result.key != FT_ERR_SUCCESS)
        return (0);
    second_sample_original = sample_result.value;
    sample_result = copied_stream.random_int();
    if (sample_result.key != FT_ERR_SUCCESS)
        return (0);
    second_sample_copy = sample_result.value;
    if (second_sample_original != second_sample_copy)
        return (0);
    if (copied_stream.reseed(1234u) != FT_ERR_SUCCESS)
        return (0);
    int independent_sample_original;
    int independent_sample_copy;

    sample_result = original_stream.random_int();
    if (sample_result.key != FT_ERR_SUCCESS)
        return (0);
    independent_sample_original = sample_result.value;
    sample_result = copied_stream.random_int();
    if (sample_result.key != FT_ERR_SUCCESS)
        return (0);
    independent_sample_copy = sample_result.value;
    if (independent_sample_original == independent_sample_copy)
        return (0);
    if (first_sample == second_sample_original)
        return (0);
    return (1);
}
