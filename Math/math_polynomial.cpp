#include "math_polynomial.hpp"

#include <cmath>
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/move.hpp"

static void math_polynomial_copy_vector(const ft_vector<double> &source,
    ft_vector<double> &destination,
    size_t count,
    int &error_code) noexcept
{
    size_t index;

    destination.resize(count, 0.0);
    if (destination.get_error() != ER_SUCCESS)
    {
        error_code = destination.get_error();
        return ;
    }
    index = 0;
    while (index < count)
    {
        destination[index] = source[index];
        index++;
    }
    error_code = ER_SUCCESS;
    return ;
}

ft_cubic_spline::ft_cubic_spline() noexcept
    : _error_code(ER_SUCCESS)
{
    return ;
}

ft_cubic_spline::ft_cubic_spline(ft_cubic_spline &&other) noexcept
    : _error_code(other._error_code)
{
    this->x_values = ft_move(other.x_values);
    this->a_coefficients = ft_move(other.a_coefficients);
    this->b_coefficients = ft_move(other.b_coefficients);
    this->c_coefficients = ft_move(other.c_coefficients);
    this->d_coefficients = ft_move(other.d_coefficients);
    other._error_code = ER_SUCCESS;
    return ;
}

ft_cubic_spline &ft_cubic_spline::operator=(ft_cubic_spline &&other) noexcept
{
    if (this != &other)
    {
        this->x_values = ft_move(other.x_values);
        this->a_coefficients = ft_move(other.a_coefficients);
        this->b_coefficients = ft_move(other.b_coefficients);
        this->c_coefficients = ft_move(other.c_coefficients);
        this->d_coefficients = ft_move(other.d_coefficients);
        this->_error_code = other._error_code;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

ft_cubic_spline::~ft_cubic_spline() noexcept
{
    this->_error_code = ER_SUCCESS;
    return ;
}

int ft_cubic_spline::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_cubic_spline::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_cubic_spline::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

static int math_polynomial_validate_coefficients(const ft_vector<double> &coefficients) noexcept
{
    if (coefficients.get_error() != ER_SUCCESS)
        return (coefficients.get_error());
    if (coefficients.size() == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    return (ER_SUCCESS);
}

int math_polynomial_evaluate(const ft_vector<double> &coefficients,
    double x,
    double &result) noexcept
{
    size_t count;
    size_t index;
    double value;
    int validation_error;

    validation_error = math_polynomial_validate_coefficients(coefficients);
    if (validation_error != ER_SUCCESS)
    {
        ft_errno = validation_error;
        result = 0.0;
        return (validation_error);
    }
    count = coefficients.size();
    value = coefficients[count - 1];
    if (count == 1)
    {
        result = value;
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    index = count - 1;
    while (index > 0)
    {
        value = value * x + coefficients[index - 1];
        index--;
    }
    result = value;
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

static int math_polynomial_evaluate_with_derivative(const ft_vector<double> &coefficients,
    double x,
    double &value,
    double &derivative) noexcept
{
    size_t count;
    size_t index;
    int validation_error;

    validation_error = math_polynomial_validate_coefficients(coefficients);
    if (validation_error != ER_SUCCESS)
    {
        ft_errno = validation_error;
        value = 0.0;
        derivative = 0.0;
        return (validation_error);
    }
    count = coefficients.size();
    value = coefficients[count - 1];
    derivative = 0.0;
    if (count == 1)
    {
        derivative = 0.0;
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    index = count - 1;
    while (index > 0)
    {
        derivative = derivative * x + value;
        value = value * x + coefficients[index - 1];
        index--;
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int math_polynomial_find_root_newton(const ft_vector<double> &coefficients,
    double initial_guess,
    double tolerance,
    size_t max_iterations,
    double &root) noexcept
{
    double current;
    double value;
    double derivative;
    size_t iteration;
    int evaluation_error;
    double difference;
    double epsilon;

    if (tolerance <= 0.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        root = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (max_iterations == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        root = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    current = initial_guess;
    iteration = 0;
    epsilon = 0.000000000001;
    while (iteration < max_iterations)
    {
        evaluation_error = math_polynomial_evaluate_with_derivative(coefficients,
                current,
                value,
                derivative);
        if (evaluation_error != ER_SUCCESS)
        {
            root = current;
            return (evaluation_error);
        }
        if (std::fabs(derivative) < epsilon)
        {
            ft_errno = FT_ERR_INVALID_OPERATION;
            root = current;
            return (FT_ERR_INVALID_OPERATION);
        }
        current = current - value / derivative;
        difference = std::fabs(value);
        if (difference <= tolerance)
        {
            root = current;
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        iteration++;
    }
    ft_errno = FT_ERR_INVALID_OPERATION;
    root = current;
    return (FT_ERR_INVALID_OPERATION);
}

int math_polynomial_solve_quadratic(double a,
    double b,
    double c,
    double &root_one,
    double &root_two) noexcept
{
    double discriminant;
    double epsilon;
    double q;
    double sqrt_discriminant;

    epsilon = 0.000000000001;
    if (std::fabs(a) <= epsilon)
    {
        if (std::fabs(b) <= epsilon)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            root_one = 0.0;
            root_two = 0.0;
            return (FT_ERR_INVALID_ARGUMENT);
        }
        root_one = -c / b;
        root_two = root_one;
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        root_one = 0.0;
        root_two = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    sqrt_discriminant = std::sqrt(discriminant);
    if (b >= 0.0)
        q = -0.5 * (b + sqrt_discriminant);
    else
        q = -0.5 * (b - sqrt_discriminant);
    root_one = q / a;
    if (std::fabs(q) <= epsilon)
        root_two = -b / a;
    else
        root_two = c / q;
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int math_polynomial_lagrange_interpolate(const ft_vector<double> &x_values,
    const ft_vector<double> &y_values,
    double x,
    double &result) noexcept
{
    size_t count;
    size_t index;
    size_t inner_index;
    double term;
    double denominator;

    if (x_values.get_error() != ER_SUCCESS)
    {
        ft_errno = x_values.get_error();
        result = 0.0;
        return (x_values.get_error());
    }
    if (y_values.get_error() != ER_SUCCESS)
    {
        ft_errno = y_values.get_error();
        result = 0.0;
        return (y_values.get_error());
    }
    if (x_values.size() == 0 || x_values.size() != y_values.size())
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    count = x_values.size();
    result = 0.0;
    index = 0;
    while (index < count)
    {
        term = y_values[index];
        inner_index = 0;
        while (inner_index < count)
        {
            if (inner_index != index)
            {
                denominator = x_values[index] - x_values[inner_index];
                if (denominator == 0.0)
                {
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                    result = 0.0;
                    return (FT_ERR_INVALID_ARGUMENT);
                }
                term = term * (x - x_values[inner_index]) / denominator;
            }
            inner_index++;
        }
        result += term;
        index++;
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int math_bezier_evaluate(const ft_vector<double> &control_points,
    double parameter,
    double &result) noexcept
{
    ft_vector<double> working;
    size_t count;
    size_t level;
    size_t index;
    int copy_error;

    if (control_points.get_error() != ER_SUCCESS)
    {
        ft_errno = control_points.get_error();
        result = 0.0;
        return (control_points.get_error());
    }
    if (control_points.size() == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (parameter < 0.0 || parameter > 1.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    count = control_points.size();
    math_polynomial_copy_vector(control_points, working, count, copy_error);
    if (copy_error != ER_SUCCESS)
    {
        ft_errno = copy_error;
        result = 0.0;
        return (copy_error);
    }
    if (count == 1)
    {
        result = working[0];
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    level = count - 1;
    while (level > 0)
    {
        index = 0;
        while (index < level)
        {
            working[index] = (1.0 - parameter) * working[index]
                + parameter * working[index + 1];
            index++;
        }
        level--;
    }
    result = working[0];
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

static int math_polynomial_extract_coordinates(const ft_vector<vector2> &control_points,
    ft_vector<double> &x_coordinates,
    ft_vector<double> &y_coordinates) noexcept
{
    size_t count;
    size_t index;
    vector2 point;
    double x_value;
    double y_value;

    if (control_points.get_error() != ER_SUCCESS)
    {
        ft_errno = control_points.get_error();
        return (control_points.get_error());
    }
    count = control_points.size();
    x_coordinates.resize(count, 0.0);
    if (x_coordinates.get_error() != ER_SUCCESS)
    {
        ft_errno = x_coordinates.get_error();
        return (x_coordinates.get_error());
    }
    y_coordinates.resize(count, 0.0);
    if (y_coordinates.get_error() != ER_SUCCESS)
    {
        ft_errno = y_coordinates.get_error();
        return (y_coordinates.get_error());
    }
    index = 0;
    while (index < count)
    {
        point = control_points[index];
        x_value = point.get_x();
        if (point.get_error() != ER_SUCCESS)
        {
            ft_errno = point.get_error();
            return (point.get_error());
        }
        y_value = point.get_y();
        if (point.get_error() != ER_SUCCESS)
        {
            ft_errno = point.get_error();
            return (point.get_error());
        }
        x_coordinates[index] = x_value;
        y_coordinates[index] = y_value;
        index++;
    }
    return (ER_SUCCESS);
}

int math_bezier_evaluate_vector2(const ft_vector<vector2> &control_points,
    double parameter,
    vector2 &result) noexcept
{
    ft_vector<double> x_coordinates;
    ft_vector<double> y_coordinates;
    double x_value;
    double y_value;
    int coordinate_error;
    vector2 evaluated_point;

    if (parameter < 0.0 || parameter > 1.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    coordinate_error = math_polynomial_extract_coordinates(control_points,
            x_coordinates,
            y_coordinates);
    if (coordinate_error != ER_SUCCESS)
    {
        ft_errno = coordinate_error;
        return (coordinate_error);
    }
    math_bezier_evaluate(x_coordinates, parameter, x_value);
    if (ft_errno != ER_SUCCESS)
        return (ft_errno);
    math_bezier_evaluate(y_coordinates, parameter, y_value);
    if (ft_errno != ER_SUCCESS)
        return (ft_errno);
    evaluated_point = vector2(x_value, y_value);
    result = evaluated_point;
    if (result.get_error() != ER_SUCCESS)
    {
        ft_errno = result.get_error();
        return (result.get_error());
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

ft_cubic_spline ft_cubic_spline_build(const ft_vector<double> &x_values,
    const ft_vector<double> &y_values) noexcept
{
    ft_cubic_spline spline;
    size_t count;
    size_t segment_count;
    ft_vector<double> h;
    ft_vector<double> alpha;
    ft_vector<double> l;
    ft_vector<double> mu;
    ft_vector<double> z;
    size_t index;
    double denominator;
    int error_code;

    if (x_values.get_error() != ER_SUCCESS)
    {
        spline.set_error(x_values.get_error());
        return (spline);
    }
    if (y_values.get_error() != ER_SUCCESS)
    {
        spline.set_error(y_values.get_error());
        return (spline);
    }
    if (x_values.size() < 2 || x_values.size() != y_values.size())
    {
        spline.set_error(FT_ERR_INVALID_ARGUMENT);
        return (spline);
    }
    count = x_values.size();
    segment_count = count - 1;
    math_polynomial_copy_vector(x_values, spline.x_values, count, error_code);
    if (error_code != ER_SUCCESS)
    {
        spline.set_error(error_code);
        return (spline);
    }
    math_polynomial_copy_vector(y_values, spline.a_coefficients, count, error_code);
    if (error_code != ER_SUCCESS)
    {
        spline.set_error(error_code);
        return (spline);
    }
    spline.b_coefficients.resize(segment_count, 0.0);
    if (spline.b_coefficients.get_error() != ER_SUCCESS)
    {
        spline.set_error(spline.b_coefficients.get_error());
        return (spline);
    }
    spline.c_coefficients.resize(count, 0.0);
    if (spline.c_coefficients.get_error() != ER_SUCCESS)
    {
        spline.set_error(spline.c_coefficients.get_error());
        return (spline);
    }
    spline.d_coefficients.resize(segment_count, 0.0);
    if (spline.d_coefficients.get_error() != ER_SUCCESS)
    {
        spline.set_error(spline.d_coefficients.get_error());
        return (spline);
    }
    h.resize(segment_count, 0.0);
    if (h.get_error() != ER_SUCCESS)
    {
        spline.set_error(h.get_error());
        return (spline);
    }
    alpha.resize(count, 0.0);
    if (alpha.get_error() != ER_SUCCESS)
    {
        spline.set_error(alpha.get_error());
        return (spline);
    }
    index = 0;
    while (index < segment_count)
    {
        h[index] = spline.x_values[index + 1] - spline.x_values[index];
        if (h[index] <= 0.0)
        {
            spline.set_error(FT_ERR_INVALID_ARGUMENT);
            return (spline);
        }
        index++;
    }
    index = 1;
    while (index < segment_count)
    {
        alpha[index] = 3.0 * (spline.a_coefficients[index + 1] - spline.a_coefficients[index]) / h[index]
            - 3.0 * (spline.a_coefficients[index] - spline.a_coefficients[index - 1]) / h[index - 1];
        index++;
    }
    l.resize(count, 0.0);
    if (l.get_error() != ER_SUCCESS)
    {
        spline.set_error(l.get_error());
        return (spline);
    }
    mu.resize(count, 0.0);
    if (mu.get_error() != ER_SUCCESS)
    {
        spline.set_error(mu.get_error());
        return (spline);
    }
    z.resize(count, 0.0);
    if (z.get_error() != ER_SUCCESS)
    {
        spline.set_error(z.get_error());
        return (spline);
    }
    l[0] = 1.0;
    mu[0] = 0.0;
    z[0] = 0.0;
    index = 1;
    while (index < segment_count)
    {
        denominator = 2.0 * (spline.x_values[index + 1] - spline.x_values[index - 1]) - h[index - 1] * mu[index - 1];
        if (denominator == 0.0)
        {
            spline.set_error(FT_ERR_INVALID_ARGUMENT);
            return (spline);
        }
        l[index] = denominator;
        mu[index] = h[index] / denominator;
        z[index] = (alpha[index] - h[index - 1] * z[index - 1]) / denominator;
        index++;
    }
    l[count - 1] = 1.0;
    z[count - 1] = 0.0;
    spline.c_coefficients[count - 1] = 0.0;
    index = segment_count;
    while (index > 0)
    {
        spline.c_coefficients[index - 1] = z[index - 1] - mu[index - 1] * spline.c_coefficients[index];
        spline.b_coefficients[index - 1] = (spline.a_coefficients[index] - spline.a_coefficients[index - 1]) / h[index - 1]
            - h[index - 1] * (spline.c_coefficients[index] + 2.0 * spline.c_coefficients[index - 1]) / 3.0;
        spline.d_coefficients[index - 1] = (spline.c_coefficients[index] - spline.c_coefficients[index - 1]) / (3.0 * h[index - 1]);
        index--;
    }
    spline.set_error(ER_SUCCESS);
    return (spline);
}

double ft_cubic_spline_evaluate(const ft_cubic_spline &spline,
    double x) noexcept
{
    size_t segment_count;
    size_t index;
    double clamped_x;
    double delta;
    double value;

    if (spline.get_error() != ER_SUCCESS)
    {
        ft_errno = spline.get_error();
        return (0.0);
    }
    if (spline.x_values.size() < 2)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0.0);
    }
    clamped_x = x;
    if (clamped_x < spline.x_values[0])
        clamped_x = spline.x_values[0];
    if (clamped_x > spline.x_values[spline.x_values.size() - 1])
        clamped_x = spline.x_values[spline.x_values.size() - 1];
    segment_count = spline.x_values.size() - 1;
    index = segment_count - 1;
    while (index > 0 && clamped_x < spline.x_values[index])
        index--;
    delta = clamped_x - spline.x_values[index];
    value = spline.a_coefficients[index]
        + spline.b_coefficients[index] * delta
        + spline.c_coefficients[index] * delta * delta
        + spline.d_coefficients[index] * delta * delta * delta;
    ft_errno = ER_SUCCESS;
    return (value);
}

static int math_integrate_trapezoidal_step(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    size_t subdivisions,
    double &result) noexcept
{
    double step;
    size_t index;
    double sum;
    double current_x;
    double function_value;

    if (subdivisions == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    step = (upper_bound - lower_bound) / static_cast<double>(subdivisions);
    sum = 0.0;
    index = 1;
    while (index < subdivisions)
    {
        current_x = lower_bound + step * static_cast<double>(index);
        function_value = function(current_x, user_data);
        if (std::isnan(function_value))
            return (FT_ERR_INVALID_OPERATION);
        sum += function_value;
        index++;
    }
    result = (function(lower_bound, user_data) + function(upper_bound, user_data)) * 0.5 + sum;
    result *= step;
    return (ER_SUCCESS);
}

int math_integrate_trapezoidal(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    size_t subdivisions,
    double &result) noexcept
{
    double local_result;
    int step_error;
    double orientation;

    if (function == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (lower_bound == upper_bound)
    {
        result = 0.0;
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    orientation = 1.0;
    if (upper_bound < lower_bound)
    {
        double temporary;

        temporary = lower_bound;
        lower_bound = upper_bound;
        upper_bound = temporary;
        orientation = -1.0;
    }
    step_error = math_integrate_trapezoidal_step(function,
            user_data,
            lower_bound,
            upper_bound,
            subdivisions,
            local_result);
    if (step_error != ER_SUCCESS)
    {
        ft_errno = step_error;
        result = 0.0;
        return (step_error);
    }
    result = orientation * local_result;
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

static int math_integrate_simpson_estimate(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    size_t subdivisions,
    double &result) noexcept
{
    double step;
    size_t index;
    double odd_sum;
    double even_sum;
    double x_value;
    double function_value;

    if ((subdivisions & 1) != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    step = (upper_bound - lower_bound) / static_cast<double>(subdivisions);
    odd_sum = 0.0;
    even_sum = 0.0;
    index = 1;
    while (index < subdivisions)
    {
        x_value = lower_bound + step * static_cast<double>(index);
        function_value = function(x_value, user_data);
        if (std::isnan(function_value))
            return (FT_ERR_INVALID_OPERATION);
        if ((index & 1) != 0)
            odd_sum += function_value;
        else
            even_sum += function_value;
        index++;
    }
    result = function(lower_bound, user_data) + function(upper_bound, user_data)
        + 4.0 * odd_sum + 2.0 * even_sum;
    result *= step / 3.0;
    return (ER_SUCCESS);
}

int math_integrate_simpson(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    double tolerance,
    double &result) noexcept
{
    size_t refinements;
    size_t subdivisions;
    double previous;
    double current;
    int estimate_error;
    double orientation;
    double difference;
    double epsilon;

    if (function == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (tolerance <= 0.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (lower_bound == upper_bound)
    {
        result = 0.0;
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    orientation = 1.0;
    if (upper_bound < lower_bound)
    {
        double temporary;

        temporary = lower_bound;
        lower_bound = upper_bound;
        upper_bound = temporary;
        orientation = -1.0;
    }
    refinements = 0;
    subdivisions = 2;
    previous = 0.0;
    current = 0.0;
    epsilon = 0.000000000001;
    while (refinements < 10)
    {
        estimate_error = math_integrate_simpson_estimate(function,
                user_data,
                lower_bound,
                upper_bound,
                subdivisions,
                current);
        if (estimate_error != ER_SUCCESS)
        {
            ft_errno = estimate_error;
            result = 0.0;
            return (estimate_error);
        }
        if (refinements > 0)
        {
            difference = std::fabs(current - previous);
            if (difference <= tolerance + epsilon)
            {
                result = orientation * current;
                ft_errno = ER_SUCCESS;
                return (ER_SUCCESS);
            }
        }
        previous = current;
        subdivisions *= 2;
        refinements++;
    }
    ft_errno = FT_ERR_INVALID_OPERATION;
    result = orientation * current;
    return (FT_ERR_INVALID_OPERATION);
}
