#include "math.hpp"
#include <cstdint>

static int32_t compile_math_usage()
{
    int32_t max_value;
    int32_t min_value;

    max_value = ft_max(2, 5);
    min_value = ft_min(2, 5);
    return (max_value + min_value);
}

static int32_t math_dummy = compile_math_usage();
