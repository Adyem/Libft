#ifndef MATH_POLYNOMIAL_HPP
# define MATH_POLYNOMIAL_HPP

# include "../Template/move.hpp"
# include "../Template/vector.hpp"
# include "../Errno/errno.hpp"
# include "../PThread/recursive_mutex.hpp"
# include "linear_algebra.hpp"
# include <cstddef>
# include <cstdint>

typedef double (*math_unary_function)(double value, void *user_data);

class ft_cubic_spline
{
    private:
        mutable pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t                    _initialized_state = 0;
        static const uint8_t       _state_uninitialized = 0;
        static const uint8_t       _state_destroyed = 1;
        static const uint8_t       _state_initialized = 2;

        void                        abort_lifecycle_error(const char *method_name,
                                        const char *reason) const noexcept;
        void                        abort_if_not_initialized(const char *method_name) const noexcept;
        int                         lock_mutex(void) const noexcept;
        int                         unlock_mutex(void) const noexcept;

    public:
        ft_vector<double> x_values;
        ft_vector<double> a_coefficients;
        ft_vector<double> b_coefficients;
        ft_vector<double> c_coefficients;
        ft_vector<double> d_coefficients;

        ft_cubic_spline() noexcept;
        ft_cubic_spline(ft_cubic_spline &&other) noexcept;
        ft_cubic_spline(const ft_cubic_spline &other) = delete;
        ft_cubic_spline &operator=(const ft_cubic_spline &other) = delete;
        ft_cubic_spline &operator=(ft_cubic_spline &&other) noexcept = delete;
        ~ft_cubic_spline() noexcept;
        int initialize() noexcept;
        int initialize(const ft_cubic_spline &other) noexcept;
        int initialize(ft_cubic_spline &&other) noexcept;
        int destroy() noexcept;
        int move(ft_cubic_spline &other) noexcept;
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
        int enable_thread_safety() noexcept;
        void disable_thread_safety() noexcept;
        bool is_thread_safe_enabled() const noexcept;
};

int math_polynomial_evaluate(const ft_vector<double> &coefficients,
        double x,
        double &result) noexcept;
int math_polynomial_find_root_newton(const ft_vector<double> &coefficients,
        double initial_guess,
        double tolerance,
        size_t max_iterations,
        double &root) noexcept;
int math_polynomial_solve_quadratic(double a,
        double b,
        double c,
        double &root_one,
        double &root_two) noexcept;
int math_polynomial_lagrange_interpolate(const ft_vector<double> &x_values,
        const ft_vector<double> &y_values,
        double x,
        double &result) noexcept;
int math_bezier_evaluate(const ft_vector<double> &control_points,
        double parameter,
        double &result) noexcept;
int math_bezier_evaluate_vector2(const ft_vector<vector2> &control_points,
        double parameter,
        vector2 &result) noexcept;
ft_cubic_spline ft_cubic_spline_build(const ft_vector<double> &x_values,
        const ft_vector<double> &y_values) noexcept;
double ft_cubic_spline_evaluate(const ft_cubic_spline &spline,
        double x) noexcept;
int math_integrate_trapezoidal(math_unary_function function,
        void *user_data,
        double lower_bound,
        double upper_bound,
        size_t subdivisions,
        double &result) noexcept;
int math_integrate_simpson(math_unary_function function,
        void *user_data,
        double lower_bound,
        double upper_bound,
        double tolerance,
        double &result) noexcept;

#endif
