#include "../../Math/math_interval.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_interval_create_valid_range, "ft_interval_create accepts ordered bounds")
{
    ft_interval interval;

    interval = ft_interval_create(-2.0, 5.0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, interval._error_code);
    FT_ASSERT_DOUBLE_EQ(-2.0, interval.lower);
    FT_ASSERT_DOUBLE_EQ(5.0, interval.upper);
    return (1);
}

FT_TEST(test_interval_create_invalid_range, "ft_interval_create rejects inverted bounds")
{
    ft_interval interval;

    interval = ft_interval_create(3.0, -1.0);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, interval._error_code);
    return (1);
}

FT_TEST(test_interval_add_propagates_bounds, "ft_interval_add combines ranges")
{
    ft_interval left_interval;
    ft_interval right_interval;
    ft_interval sum_interval;

    left_interval = ft_interval_create(-1.0, 2.0);
    right_interval = ft_interval_create(4.0, 6.0);
    sum_interval = ft_interval_add(left_interval, right_interval);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, sum_interval._error_code);
    FT_ASSERT_DOUBLE_EQ(3.0, sum_interval.lower);
    FT_ASSERT_DOUBLE_EQ(8.0, sum_interval.upper);
    return (1);
}

FT_TEST(test_interval_multiply_allows_sign_variations, "ft_interval_multiply evaluates all endpoint products")
{
    ft_interval left_interval;
    ft_interval right_interval;
    ft_interval product_interval;

    left_interval = ft_interval_create(-2.0, 3.0);
    right_interval = ft_interval_create(-4.0, 5.0);
    product_interval = ft_interval_multiply(left_interval, right_interval);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, product_interval._error_code);
    FT_ASSERT_DOUBLE_EQ(-20.0, product_interval.lower);
    FT_ASSERT_DOUBLE_EQ(15.0, product_interval.upper);
    return (1);
}

FT_TEST(test_interval_divide_detects_zero_crossing, "ft_interval_divide rejects denominators containing zero")
{
    ft_interval numerator_interval;
    ft_interval denominator_interval;
    ft_interval quotient_interval;

    numerator_interval = ft_interval_create(1.0, 2.0);
    denominator_interval = ft_interval_create(-1.0, 1.0);
    quotient_interval = ft_interval_divide(numerator_interval, denominator_interval);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quotient_interval._error_code);
    return (1);
}

FT_TEST(test_interval_linear_propagation_combines_uncertainties, "ft_interval_propagate_linear accumulates weighted ranges")
{
    ft_interval components[2];
    double sensitivities[2];
    ft_interval combined_interval;

    components[0] = ft_interval_from_measurement(10.0, 0.5);
    components[1] = ft_interval_from_measurement(-2.0, 0.25);
    sensitivities[0] = 1.5;
    sensitivities[1] = -3.0;
    combined_interval = ft_interval_propagate_linear(components, sensitivities, 2);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, combined_interval._error_code);
    FT_ASSERT_DOUBLE_EQ((10.0 - 0.5) * 1.5 + (-2.0 + 0.25) * -3.0, combined_interval.lower);
    FT_ASSERT_DOUBLE_EQ((10.0 + 0.5) * 1.5 + (-2.0 - 0.25) * -3.0, combined_interval.upper);
    return (1);
}

FT_TEST(test_interval_midpoint_and_radius_report_geometry, "ft_interval_midpoint and ft_interval_radius expose center and radius")
{
    ft_interval interval;

    interval = ft_interval_create(4.0, 10.0);
    FT_ASSERT_DOUBLE_EQ(7.0, ft_interval_midpoint(interval));
    FT_ASSERT_DOUBLE_EQ(3.0, ft_interval_radius(interval));
    FT_ASSERT_EQ(1, ft_interval_contains(interval, 6.0));
    FT_ASSERT_EQ(0, ft_interval_contains(interval, 11.0));
    return (1);
}

FT_TEST(test_interval_widen_expands_bounds_symmetrically, "ft_interval_widen enlarges an interval by the given error")
{
    ft_interval interval;
    ft_interval widened_interval;

    interval = ft_interval_create(2.0, 4.0);
    widened_interval = ft_interval_widen(interval, 1.5);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, widened_interval._error_code);
    FT_ASSERT_DOUBLE_EQ(0.5, widened_interval.lower);
    FT_ASSERT_DOUBLE_EQ(5.5, widened_interval.upper);
    return (1);
}
