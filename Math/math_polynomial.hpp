#ifndef MATH_POLYNOMIAL_HPP
# define MATH_POLYNOMIAL_HPP

# include "../Template/vector.hpp"
# include "../Errno/errno.hpp"
# include "linear_algebra.hpp"
# include <cstddef>

typedef double (*math_unary_function)(double value, void *user_data);

class ft_cubic_spline
{
    private:
        mutable int _error_code;

    public:
        ft_vector<double> x_values;
        ft_vector<double> a_coefficients;
        ft_vector<double> b_coefficients;
        ft_vector<double> c_coefficients;
        ft_vector<double> d_coefficients;

        ft_cubic_spline() noexcept;
        ft_cubic_spline(ft_cubic_spline &&other) noexcept;
        ft_cubic_spline &operator=(ft_cubic_spline &&other) noexcept;
        ft_cubic_spline(const ft_cubic_spline &other) = delete;
        ft_cubic_spline &operator=(const ft_cubic_spline &other) = delete;
        ~ft_cubic_spline() noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        void set_error(int error_code) const noexcept;
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
