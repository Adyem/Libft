#include "math_interval.hpp"

#include <cmath>
#include "../CPP_class/class_nullptr.hpp"

static ft_interval ft_interval_error(int error_code)
{
    ft_interval interval;

    interval.lower = 0.0;
    interval.upper = 0.0;
    interval._error_code = error_code;
    ft_errno = error_code;
    return (interval);
}

static ft_interval ft_interval_propagate_input_error(const ft_interval &left_interval, const ft_interval &right_interval)
{
    if (left_interval._error_code != ER_SUCCESS)
    {
        ft_interval interval;

        interval.lower = left_interval.lower;
        interval.upper = left_interval.upper;
        interval._error_code = left_interval._error_code;
        ft_errno = interval._error_code;
        return (interval);
    }
    if (right_interval._error_code != ER_SUCCESS)
    {
        ft_interval interval;

        interval.lower = right_interval.lower;
        interval.upper = right_interval.upper;
        interval._error_code = right_interval._error_code;
        ft_errno = interval._error_code;
        return (interval);
    }
    return (ft_interval_create(0.0, 0.0));
}

ft_interval ft_interval_create(double lower, double upper) noexcept
{
    ft_interval interval;

    interval.lower = lower;
    interval.upper = upper;
    interval._error_code = ER_SUCCESS;
    if (std::isnan(lower) || std::isnan(upper))
        return (ft_interval_error(FT_ERR_INVALID_ARGUMENT));
    if (lower > upper)
        return (ft_interval_error(FT_ERR_INVALID_ARGUMENT));
    ft_errno = ER_SUCCESS;
    return (interval);
}

ft_interval ft_interval_from_measurement(double value, double absolute_error) noexcept
{
    ft_interval interval;

    if (std::isnan(value) || std::isnan(absolute_error))
        return (ft_interval_error(FT_ERR_INVALID_ARGUMENT));
    if (absolute_error < 0.0)
        return (ft_interval_error(FT_ERR_INVALID_ARGUMENT));
    interval = ft_interval_create(value - absolute_error, value + absolute_error);
    ft_errno = interval._error_code;
    return (interval);
}

ft_interval ft_interval_add(const ft_interval &left_interval, const ft_interval &right_interval) noexcept
{
    ft_interval input_error;
    ft_interval result;

    input_error = ft_interval_propagate_input_error(left_interval, right_interval);
    if (input_error._error_code != ER_SUCCESS)
        return (input_error);
    result = ft_interval_create(left_interval.lower + right_interval.lower,
            left_interval.upper + right_interval.upper);
    ft_errno = result._error_code;
    return (result);
}

ft_interval ft_interval_subtract(const ft_interval &left_interval, const ft_interval &right_interval) noexcept
{
    ft_interval input_error;
    ft_interval result;

    input_error = ft_interval_propagate_input_error(left_interval, right_interval);
    if (input_error._error_code != ER_SUCCESS)
        return (input_error);
    result = ft_interval_create(left_interval.lower - right_interval.upper,
            left_interval.upper - right_interval.lower);
    ft_errno = result._error_code;
    return (result);
}

static double ft_interval_min(double first_value, double second_value)
{
    if (first_value < second_value)
        return (first_value);
    return (second_value);
}

static double ft_interval_max(double first_value, double second_value)
{
    if (first_value > second_value)
        return (first_value);
    return (second_value);
}

ft_interval ft_interval_multiply(const ft_interval &left_interval, const ft_interval &right_interval) noexcept
{
    ft_interval input_error;
    ft_interval result;
    double product_ll;
    double product_lu;
    double product_ul;
    double product_uu;
    double minimum_value;
    double maximum_value;

    input_error = ft_interval_propagate_input_error(left_interval, right_interval);
    if (input_error._error_code != ER_SUCCESS)
        return (input_error);
    product_ll = left_interval.lower * right_interval.lower;
    product_lu = left_interval.lower * right_interval.upper;
    product_ul = left_interval.upper * right_interval.lower;
    product_uu = left_interval.upper * right_interval.upper;
    minimum_value = ft_interval_min(product_ll, product_lu);
    minimum_value = ft_interval_min(minimum_value, product_ul);
    minimum_value = ft_interval_min(minimum_value, product_uu);
    maximum_value = ft_interval_max(product_ll, product_lu);
    maximum_value = ft_interval_max(maximum_value, product_ul);
    maximum_value = ft_interval_max(maximum_value, product_uu);
    result = ft_interval_create(minimum_value, maximum_value);
    ft_errno = result._error_code;
    return (result);
}

ft_interval ft_interval_divide(const ft_interval &left_interval, const ft_interval &right_interval) noexcept
{
    ft_interval input_error;
    ft_interval reciprocal;

    input_error = ft_interval_propagate_input_error(left_interval, right_interval);
    if (input_error._error_code != ER_SUCCESS)
        return (input_error);
    if (right_interval.lower <= 0.0 && right_interval.upper >= 0.0)
        return (ft_interval_error(FT_ERR_INVALID_ARGUMENT));
    reciprocal = ft_interval_create(0.0, 0.0);
    if (right_interval.lower > 0.0)
        reciprocal = ft_interval_create(1.0 / right_interval.upper, 1.0 / right_interval.lower);
    else
        reciprocal = ft_interval_create(1.0 / right_interval.upper, 1.0 / right_interval.lower);
    if (reciprocal._error_code != ER_SUCCESS)
        return (reciprocal);
    return (ft_interval_multiply(left_interval, reciprocal));
}

ft_interval ft_interval_widen(const ft_interval &interval, double absolute_error) noexcept
{
    ft_interval result;

    if (interval._error_code != ER_SUCCESS)
    {
        result = interval;
        ft_errno = result._error_code;
        return (result);
    }
    if (std::isnan(absolute_error) || absolute_error < 0.0)
        return (ft_interval_error(FT_ERR_INVALID_ARGUMENT));
    result = ft_interval_create(interval.lower - absolute_error,
            interval.upper + absolute_error);
    ft_errno = result._error_code;
    return (result);
}

ft_interval ft_interval_propagate_linear(const ft_interval *components,
        const double *sensitivities,
        size_t component_count) noexcept
{
    ft_interval result;
    size_t index;
    double minimum_sum;
    double maximum_sum;

    if (components == ft_nullptr || sensitivities == ft_nullptr)
        return (ft_interval_error(FT_ERR_INVALID_ARGUMENT));
    minimum_sum = 0.0;
    maximum_sum = 0.0;
    index = 0;
    while (index < component_count)
    {
        const ft_interval *current_component;
        double sensitivity;
        double contribution_lower;
        double contribution_upper;

        current_component = &components[index];
        sensitivity = sensitivities[index];
        if (current_component->_error_code != ER_SUCCESS)
            return (ft_interval_error(current_component->_error_code));
        if (sensitivity >= 0.0)
        {
            contribution_lower = sensitivity * current_component->lower;
            contribution_upper = sensitivity * current_component->upper;
        }
        else
        {
            contribution_lower = sensitivity * current_component->upper;
            contribution_upper = sensitivity * current_component->lower;
        }
        minimum_sum += contribution_lower;
        maximum_sum += contribution_upper;
        index++;
    }
    result = ft_interval_create(minimum_sum, maximum_sum);
    ft_errno = result._error_code;
    return (result);
}

int ft_interval_get_error(const ft_interval *interval) noexcept
{
    if (interval == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    ft_errno = interval->_error_code;
    return (interval->_error_code);
}

const char *ft_interval_get_error_str(const ft_interval *interval) noexcept
{
    int error_code;

    error_code = ft_interval_get_error(interval);
    return (ft_strerror(error_code));
}

double ft_interval_midpoint(const ft_interval &interval) noexcept
{
    if (interval._error_code != ER_SUCCESS)
    {
        ft_errno = interval._error_code;
        return (0.0);
    }
    ft_errno = ER_SUCCESS;
    return ((interval.lower + interval.upper) / 2.0);
}

double ft_interval_radius(const ft_interval &interval) noexcept
{
    if (interval._error_code != ER_SUCCESS)
    {
        ft_errno = interval._error_code;
        return (0.0);
    }
    ft_errno = ER_SUCCESS;
    return ((interval.upper - interval.lower) / 2.0);
}

int ft_interval_contains(const ft_interval &interval, double value) noexcept
{
    if (interval._error_code != ER_SUCCESS)
    {
        ft_errno = interval._error_code;
        return (0);
    }
    if (value < interval.lower)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (value > interval.upper)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (1);
}
