#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Math/math.hpp"

float ft_random_normal(void)
{
    float uniform_one;
    float uniform_two;
    const float pi_value = 3.14159265358979323846f;
    float radius;
    float angle;
    float result;

    ft_init_random_engine();
    uniform_one = ft_random_float();
    int error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    if (uniform_one < 0.0000000001f)
        uniform_one = 0.0000000001f;
    uniform_two = ft_random_float();
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    double log_value;
    double sqrt_value;
    double cosine_value;

    log_value = math_log(uniform_one);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    sqrt_value = math_sqrt(-2.0 * log_value);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    radius = static_cast<float>(sqrt_value);
    angle = 2.0f * pi_value * uniform_two;
    cosine_value = math_cos(angle);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    result = radius * static_cast<float>(cosine_value);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}
