#include "math.hpp"

static int compile_math_usage()
{
    int max_value;
    int min_value;

    max_value = ft_max(2, 5);
    min_value = ft_min(2, 5);
    return (max_value + min_value);
}

static int math_dummy = compile_math_usage();
