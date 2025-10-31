#ifndef MATH_INTERVAL_HPP
# define MATH_INTERVAL_HPP

#include <cstddef>
#include "../Errno/errno.hpp"

typedef struct s_ft_interval
{
    double  lower;
    double  upper;
    mutable int _error_code;
}   ft_interval;

ft_interval  ft_interval_create(double lower, double upper) noexcept;
ft_interval  ft_interval_from_measurement(double value, double absolute_error) noexcept;
ft_interval  ft_interval_add(const ft_interval &left_interval, const ft_interval &right_interval) noexcept;
ft_interval  ft_interval_subtract(const ft_interval &left_interval, const ft_interval &right_interval) noexcept;
ft_interval  ft_interval_multiply(const ft_interval &left_interval, const ft_interval &right_interval) noexcept;
ft_interval  ft_interval_divide(const ft_interval &left_interval, const ft_interval &right_interval) noexcept;
ft_interval  ft_interval_widen(const ft_interval &interval, double absolute_error) noexcept;
ft_interval  ft_interval_propagate_linear(const ft_interval *components,
                const double *sensitivities,
                size_t component_count) noexcept;
int          ft_interval_get_error(const ft_interval *interval) noexcept;
const char  *ft_interval_get_error_str(const ft_interval *interval) noexcept;

double      ft_interval_midpoint(const ft_interval &interval) noexcept;
double      ft_interval_radius(const ft_interval &interval) noexcept;
int         ft_interval_contains(const ft_interval &interval, double value) noexcept;

#endif
