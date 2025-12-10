#include "../../RNG/rng.hpp"
#include "../../RNG/rng_stream.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_rng_stream_independent_seeds, "rng_stream instances keep independent seeds")
{
    rng_stream first_stream(123u);
    rng_stream second_stream(456u);
    int first_values[3];
    int second_values[3];
    int index;

    index = 0;
    while (index < 3)
    {
        first_values[index] = first_stream.random_int();
        second_values[index] = second_stream.random_int();
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

    initial_value = stream_instance.random_int();
    stream_instance.reseed_from_string("example-seed");
    reseeded_value = stream_instance.random_int();
    if (stream_instance.get_error() != FT_ER_SUCCESSS)
        return (0);
    if (initial_value == reseeded_value)
        return (0);
    stream_instance.reseed_from_string(ft_nullptr);
    if (stream_instance.get_error() != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_rng_stream_copy_preserves_state, "rng_stream copy produces identical subsequent samples")
{
    rng_stream original_stream(987u);
    int first_sample;
    int second_sample_original;
    int second_sample_copy;

    first_sample = original_stream.random_int();
    rng_stream copied_stream(original_stream);
    second_sample_original = original_stream.random_int();
    second_sample_copy = copied_stream.random_int();
    if (second_sample_original != second_sample_copy)
        return (0);
    copied_stream.reseed(1234u);
    int independent_sample_original;
    int independent_sample_copy;

    independent_sample_original = original_stream.random_int();
    independent_sample_copy = copied_stream.random_int();
    if (independent_sample_original == independent_sample_copy)
        return (0);
    if (first_sample == second_sample_original)
        return (0);
    return (1);
}
