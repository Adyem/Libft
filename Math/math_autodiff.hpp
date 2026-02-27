#ifndef MATH_AUTODIFF_HPP
#define MATH_AUTODIFF_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/move.hpp"
#include "../Template/vector.hpp"
#include <cstdint>

class ft_dual_number_proxy;

class ft_dual_number
{
    private:
        double          _value;
        double          _derivative;
        mutable pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t         _initialized_state = 0;
        int             _operation_error = FT_ERR_SUCCESS;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        static thread_local int _last_error;

        static int      lock_pair(const ft_dual_number &first,
                            const ft_dual_number &second,
                            const ft_dual_number *&lower,
                            const ft_dual_number *&upper);
        static void     unlock_pair(const ft_dual_number *lower,
                            const ft_dual_number *upper);
        void            abort_lifecycle_error(const char *method_name,
                            const char *reason) const noexcept;
        void            abort_if_not_initialized(const char *method_name) const noexcept;
        static int      set_last_operation_error(int error_code) noexcept;
        friend class ft_dual_number_proxy;

    public:
        ft_dual_number() noexcept;
        ft_dual_number(double value, double derivative) noexcept;
        ft_dual_number(const ft_dual_number &other) noexcept;
        ft_dual_number(ft_dual_number &&other) noexcept;
        ~ft_dual_number() noexcept;

        int             initialize() noexcept;
        int             initialize(double value, double derivative) noexcept;
        int             initialize(const ft_dual_number &other) noexcept;
        int             initialize(ft_dual_number &&other) noexcept;
        int             destroy() noexcept;
        int             move(ft_dual_number &other) noexcept;

        ft_dual_number &operator=(const ft_dual_number &other) noexcept;
        ft_dual_number &operator=(ft_dual_number &&other) noexcept;

        static ft_dual_number   constant(double value) noexcept;
        static ft_dual_number   variable(double value) noexcept;

        double  value() const noexcept;
        double  derivative() const noexcept;

        ft_dual_number_proxy operator+(const ft_dual_number &other) const noexcept;
        ft_dual_number_proxy operator-(const ft_dual_number &other) const noexcept;
        ft_dual_number_proxy operator*(const ft_dual_number &other) const noexcept;
        ft_dual_number_proxy operator/(const ft_dual_number &other) const noexcept;

        ft_dual_number   apply_sin() const noexcept;
        ft_dual_number   apply_cos() const noexcept;
        ft_dual_number   apply_exp() const noexcept;
        ft_dual_number   apply_log() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        static int last_operation_error() noexcept;
        static const char *last_operation_error_str() noexcept;
};

class ft_dual_number_proxy
{
    private:
        ft_dual_number  _value;
        int             _last_error;

    public:
        ft_dual_number_proxy() noexcept;
        explicit ft_dual_number_proxy(int error_code) noexcept;
        ft_dual_number_proxy(const ft_dual_number &value, int error_code) noexcept;
        ft_dual_number_proxy(const ft_dual_number_proxy &other) noexcept;
        ft_dual_number_proxy(ft_dual_number_proxy &&other) noexcept;
        ~ft_dual_number_proxy();

        ft_dual_number_proxy &operator=(const ft_dual_number_proxy &other) noexcept;
        ft_dual_number_proxy &operator=(ft_dual_number_proxy &&other) noexcept;

        ft_dual_number_proxy operator+(const ft_dual_number &other) const noexcept;
        ft_dual_number_proxy operator-(const ft_dual_number &other) const noexcept;
        ft_dual_number_proxy operator*(const ft_dual_number &other) const noexcept;
        ft_dual_number_proxy operator/(const ft_dual_number &other) const noexcept;

        operator ft_dual_number() const noexcept;
        int get_error() const noexcept;
};

typedef ft_dual_number (*math_autodiff_univariate_function)(const ft_dual_number &input, void *user_data);
typedef ft_dual_number (*math_autodiff_multivariate_function)(const ft_vector<ft_dual_number> &input, void *user_data);

int     math_autodiff_univariate(math_autodiff_univariate_function function,
            double point, double *value, double *derivative, void *user_data) noexcept;
int     math_autodiff_gradient(math_autodiff_multivariate_function function,
            const ft_vector<double> &point, ft_vector<double> &gradient,
            double *value, void *user_data) noexcept;

#endif
