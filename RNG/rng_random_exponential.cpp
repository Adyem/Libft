#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Math/math.hpp"

float ft_random_exponential(float lambda_value)
{
    float uniform_value;
    float result;

    ft_init_random_engine();
    if (lambda_value <= 0.0f)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    uniform_value = ft_random_float();
    int error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    if (uniform_value < 0.0000000001f)
        uniform_value = 0.0000000001f;
    double log_value;

    log_value = math_log(uniform_value);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    result = static_cast<float>(-log_value) / lambda_value;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}
