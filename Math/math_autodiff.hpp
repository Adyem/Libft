#ifndef MATH_AUTODIFF_HPP
#define MATH_AUTODIFF_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/vector.hpp"

class ft_dual_number
{
    private:
        double          _value;
        double          _derivative;
        mutable pt_recursive_mutex _mutex;

    public:
        ft_dual_number() noexcept;
        ft_dual_number(double value, double derivative) noexcept;
        ft_dual_number(const ft_dual_number &other) noexcept;
        ft_dual_number(ft_dual_number &&other) noexcept;
        ~ft_dual_number() noexcept;

        ft_dual_number &operator=(const ft_dual_number &other) noexcept;
        ft_dual_number &operator=(ft_dual_number &&other) noexcept;

        static ft_dual_number   constant(double value) noexcept;
        static ft_dual_number   variable(double value) noexcept;

        double  value() const noexcept;
        double  derivative() const noexcept;

        ft_dual_number   operator+(const ft_dual_number &other) const noexcept;
        ft_dual_number   operator-(const ft_dual_number &other) const noexcept;
        ft_dual_number   operator*(const ft_dual_number &other) const noexcept;
        ft_dual_number   operator/(const ft_dual_number &other) const noexcept;

        ft_dual_number   apply_sin() const noexcept;
        ft_dual_number   apply_cos() const noexcept;
        ft_dual_number   apply_exp() const noexcept;
        ft_dual_number   apply_log() const noexcept;

        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
};

typedef ft_dual_number (*math_autodiff_univariate_function)(const ft_dual_number &input, void *user_data);
typedef ft_dual_number (*math_autodiff_multivariate_function)(const ft_vector<ft_dual_number> &input, void *user_data);

int     math_autodiff_univariate(math_autodiff_univariate_function function,
            double point, double *value, double *derivative, void *user_data) noexcept;
int     math_autodiff_gradient(math_autodiff_multivariate_function function,
            const ft_vector<double> &point, ft_vector<double> &gradient,
            double *value, void *user_data) noexcept;

#endif
