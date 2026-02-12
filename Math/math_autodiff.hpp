#ifndef MATH_AUTODIFF_HPP
#define MATH_AUTODIFF_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/move.hpp"
#include "../Template/vector.hpp"

class ft_dual_number
{
    private:
        double          _value;
        double          _derivative;
        mutable pt_recursive_mutex *_mutex = ft_nullptr;

        int             lock_mutex(void) const noexcept;
        int             unlock_mutex(void) const noexcept;
        int             prepare_thread_safety(void) noexcept;
        void            teardown_thread_safety(void) noexcept;

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
        int enable_thread_safety() noexcept;
        void disable_thread_safety() noexcept;
        bool is_thread_safe_enabled() const noexcept;
};

typedef ft_dual_number (*math_autodiff_univariate_function)(const ft_dual_number &input, void *user_data);
typedef ft_dual_number (*math_autodiff_multivariate_function)(const ft_vector<ft_dual_number> &input, void *user_data);

int     math_autodiff_univariate(math_autodiff_univariate_function function,
            double point, double *value, double *derivative, void *user_data) noexcept;
int     math_autodiff_gradient(math_autodiff_multivariate_function function,
            const ft_vector<double> &point, ft_vector<double> &gradient,
            double *value, void *user_data) noexcept;

#endif
