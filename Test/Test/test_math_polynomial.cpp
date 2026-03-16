#include "../test_internal.hpp"
#include "../../Math/ft_cubic_spline.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <cmath>

#ifndef LIBFT_TEST_BUILD
#endif

static double test_math_polynomial_sine(double value, void *user_data)
{
    (void)user_data;
    return (std::sin(value));
}

static double test_math_polynomial_square(double value, void *user_data)
{
    (void)user_data;
    return (value * value);
}

FT_TEST(test_polynomial_newton_finds_root)
{
    ft_vector<double> coefficients;
    double root;
    int32_t result_code;
    double tolerance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, coefficients.initialize());
    coefficients.push_back(-2.0);
    coefficients.push_back(0.0);
    coefficients.push_back(1.0);
    tolerance = 0.000001;
    result_code = math_polynomial_find_root_newton(coefficients,
            1.0,
            tolerance,
            32,
            root);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result_code);
    FT_ASSERT(std::fabs(root * root - 2.0) < tolerance);
    return (1);
}

FT_TEST(test_polynomial_quadratic_solver_returns_roots)
{
    double root_one;
    double root_two;
    int32_t result_code;

    result_code = math_polynomial_solve_quadratic(1.0, -5.0, 6.0,
            root_one,
            root_two);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result_code);
    FT_ASSERT(std::fabs((root_one * root_one) - 5.0 * root_one + 6.0) < 0.000001);
    FT_ASSERT(std::fabs((root_two * root_two) - 5.0 * root_two + 6.0) < 0.000001);
    return (1);
}

FT_TEST(test_polynomial_lagrange_interpolation_recovers_parabola)
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    double interpolated;
    int32_t result_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, x_values.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, y_values.initialize());
    x_values.push_back(-1.0);
    x_values.push_back(0.0);
    x_values.push_back(1.0);
    y_values.push_back(1.0);
    y_values.push_back(0.0);
    y_values.push_back(1.0);
    result_code = math_polynomial_lagrange_interpolate(x_values,
            y_values,
            2.0,
            interpolated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result_code);
    FT_ASSERT(std::fabs(interpolated - 4.0) < 0.000001);
    return (1);
}

FT_TEST(test_polynomial_bezier_scalar_midpoint)
{
    ft_vector<double> control_points;
    double value;
    int32_t result_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, control_points.initialize());
    control_points.push_back(0.0);
    control_points.push_back(2.0);
    control_points.push_back(4.0);
    result_code = math_bezier_evaluate(control_points, 0.5, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result_code);
    FT_ASSERT(std::fabs(value - 2.0) < 0.000001);
    return (1);
}

FT_TEST(test_polynomial_cubic_spline_tracks_curve)
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline spline;
    double evaluated;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, x_values.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, y_values.initialize());
    x_values.push_back(0.0);
    x_values.push_back(1.0);
    x_values.push_back(2.0);
    x_values.push_back(3.0);
    y_values.push_back(0.0);
    y_values.push_back(1.0);
    y_values.push_back(8.0);
    y_values.push_back(27.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.initialize(ft_cubic_spline_build(x_values, y_values)));
    evaluated = ft_cubic_spline_evaluate(spline, 1.5);
    FT_ASSERT(std::fabs(evaluated - 3.375) < 0.01);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline.destroy());
    return (1);
}

FT_TEST(test_polynomial_trapezoidal_integral)
{
    double result;
    int32_t status;

    status = math_integrate_trapezoidal(test_math_polynomial_square,
            ft_nullptr,
            0.0,
            1.0,
            128,
            result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    FT_ASSERT(std::fabs(result - (1.0 / 3.0)) < 0.001);
    return (1);
}

FT_TEST(test_polynomial_simpson_integral)
{
    double result;
    int32_t status;
    double pi_value;

    pi_value = std::acos(-1.0);
    status = math_integrate_simpson(test_math_polynomial_sine,
            ft_nullptr,
            0.0,
            pi_value,
            0.000001,
            result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    FT_ASSERT(std::fabs(result - 2.0) < 0.0001);
    return (1);
}
