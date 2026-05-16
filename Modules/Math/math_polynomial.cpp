#include "ft_cubic_spline.hpp"

#include <cmath>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <new>
#include "../Template/move.hpp"
static void math_polynomial_copy_vector(const ft_vector<double> &source,
    ft_vector<double> &destination,
    ft_size_t count,
    int32_t &error_code) noexcept
{
    ft_size_t index;

    destination.resize(count, 0.0);
    int32_t resize_error = FT_ERR_SUCCESS;
    if (resize_error != FT_ERR_SUCCESS)
    {
        error_code = resize_error;
        return ;
    }
    index = 0;
    while (index < count)
    {
        destination[index] = source[index];
        index++;
    }
    error_code = FT_ERR_SUCCESS;
    return ;
}

void ft_cubic_spline::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void ft_cubic_spline::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t ft_cubic_spline::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    this->abort_if_not_initialised("ft_cubic_spline::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t ft_cubic_spline::disable_thread_safety() noexcept
{
    int32_t mutex_error;

    this->abort_if_not_initialised("ft_cubic_spline::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
        if (mutex_error != FT_ERR_SUCCESS)
            return (mutex_error);
    }
    return (FT_ERR_SUCCESS);
}

ft_bool ft_cubic_spline::is_thread_safe() const noexcept
{
    this->abort_if_not_initialised("ft_cubic_spline::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

ft_cubic_spline::ft_cubic_spline() noexcept
    : _mutex(ft_nullptr)
{
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

int32_t ft_cubic_spline::initialize() noexcept
{
    int32_t initialize_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("ft_cubic_spline::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    initialize_error = this->_x_values.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (initialize_error);
    }
    initialize_error = this->_a_coefficients.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_x_values.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (initialize_error);
    }
    initialize_error = this->_b_coefficients.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_a_coefficients.destroy();
        (void)this->_x_values.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (initialize_error);
    }
    initialize_error = this->_c_coefficients.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_b_coefficients.destroy();
        (void)this->_a_coefficients.destroy();
        (void)this->_x_values.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (initialize_error);
    }
    initialize_error = this->_d_coefficients.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_c_coefficients.destroy();
        (void)this->_b_coefficients.destroy();
        (void)this->_a_coefficients.destroy();
        (void)this->_x_values.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (initialize_error);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_cubic_spline::initialize(const ft_cubic_spline &other) noexcept
{
    int32_t copy_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("ft_cubic_spline::initialize(const ft_cubic_spline &) source",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    copy_error = this->initialize();
    if (copy_error != FT_ERR_SUCCESS)
        return (copy_error);
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    copy_error = this->_x_values.copy_from(other._x_values);
    if (copy_error == FT_ERR_SUCCESS)
        copy_error = this->_a_coefficients.copy_from(other._a_coefficients);
    if (copy_error == FT_ERR_SUCCESS)
        copy_error = this->_b_coefficients.copy_from(other._b_coefficients);
    if (copy_error == FT_ERR_SUCCESS)
        copy_error = this->_c_coefficients.copy_from(other._c_coefficients);
    if (copy_error == FT_ERR_SUCCESS)
        copy_error = this->_d_coefficients.copy_from(other._d_coefficients);
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (copy_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (copy_error);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_cubic_spline::move(ft_cubic_spline &other) noexcept
{
    const ft_cubic_spline *lower;
    const ft_cubic_spline *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lower_error;
    int32_t upper_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("ft_cubic_spline::move source",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lower = this;
    upper = &other;
    if (lower > upper)
    {
        const ft_cubic_spline *temporary = lower;

        lower = upper;
        upper = temporary;
    }
    while (FT_TRUE)
    {
        lower_error = pt_recursive_mutex_lock_if_not_null(lower->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        upper_error = pt_recursive_mutex_lock_if_not_null(upper->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            break ;
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
            return (upper_error);
        }
        (void)pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    }
    move_error = this->_x_values.copy_from(other._x_values);
    if (move_error == FT_ERR_SUCCESS)
        move_error = this->_a_coefficients.copy_from(other._a_coefficients);
    if (move_error == FT_ERR_SUCCESS)
        move_error = this->_b_coefficients.copy_from(other._b_coefficients);
    if (move_error == FT_ERR_SUCCESS)
        move_error = this->_c_coefficients.copy_from(other._c_coefficients);
    if (move_error == FT_ERR_SUCCESS)
        move_error = this->_d_coefficients.copy_from(other._d_coefficients);
    if (move_error == FT_ERR_SUCCESS)
        other._x_values.clear();
    if (move_error == FT_ERR_SUCCESS)
        other._a_coefficients.clear();
    if (move_error == FT_ERR_SUCCESS)
        other._b_coefficients.clear();
    if (move_error == FT_ERR_SUCCESS)
        other._c_coefficients.clear();
    if (move_error == FT_ERR_SUCCESS)
        other._d_coefficients.clear();
    (void)pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != upper)
        (void)pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    return (move_error);
}

int32_t ft_cubic_spline::initialize(ft_cubic_spline &&other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("ft_cubic_spline::initialize(ft_cubic_spline &&) source",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_cubic_spline::destroy() noexcept
{
    int32_t first_error;
    int32_t destroy_error;
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    first_error = FT_ERR_SUCCESS;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = disable_error;
    destroy_error = this->_x_values.destroy();
    if (destroy_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = destroy_error;
    destroy_error = this->_a_coefficients.destroy();
    if (destroy_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = destroy_error;
    destroy_error = this->_b_coefficients.destroy();
    if (destroy_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = destroy_error;
    destroy_error = this->_c_coefficients.destroy();
    if (destroy_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = destroy_error;
    destroy_error = this->_d_coefficients.destroy();
    if (destroy_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = destroy_error;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (first_error);
}

ft_cubic_spline::~ft_cubic_spline() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}


static int32_t math_polynomial_validate_coefficients(const ft_vector<double> &coefficients) noexcept
{
    int32_t coefficients_error = FT_ERR_SUCCESS;
    if (coefficients_error != FT_ERR_SUCCESS)
        return (coefficients_error);
    if (coefficients.size() == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    return (FT_ERR_SUCCESS);
}

int32_t math_polynomial_evaluate(const ft_vector<double> &coefficients,
    double input_value,
    double &result) noexcept
{
    ft_size_t count;
    ft_size_t index;
    double value;
    int32_t validation_error;

    validation_error = math_polynomial_validate_coefficients(coefficients);
    if (validation_error != FT_ERR_SUCCESS)
    {
        result = 0.0;
        return (validation_error);
    }
    count = coefficients.size();
    value = coefficients[count - 1];
    if (count == 1)
    {
        result = value;
        return (FT_ERR_SUCCESS);
    }
    index = count - 1;
    while (index > 0)
    {
        value = value * input_value + coefficients[index - 1];
        index--;
    }
    result = value;
    return (FT_ERR_SUCCESS);
}

static int32_t math_polynomial_evaluate_with_derivative(const ft_vector<double> &coefficients,
    double input_value,
    double &value,
    double &derivative) noexcept
{
    ft_size_t count;
    ft_size_t index;
    int32_t validation_error;

    validation_error = math_polynomial_validate_coefficients(coefficients);
    if (validation_error != FT_ERR_SUCCESS)
    {
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
        return (FT_ERR_SUCCESS);
    }
    index = count - 1;
    while (index > 0)
    {
        derivative = derivative * input_value + value;
        value = value * input_value + coefficients[index - 1];
        index--;
    }
    return (FT_ERR_SUCCESS);
}

int32_t math_polynomial_find_root_newton(const ft_vector<double> &coefficients,
    double initial_guess,
    double tolerance,
    ft_size_t max_iterations,
    double &root) noexcept
{
    double current;
    double value;
    double derivative;
    ft_size_t iteration;
    int32_t evaluation_error;
    double difference;
    double epsilon;

    if (tolerance <= 0.0)
    {
        root = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (max_iterations == 0)
    {
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
        if (evaluation_error != FT_ERR_SUCCESS)
        {
            root = current;
            return (evaluation_error);
        }
        if (std::fabs(derivative) < epsilon)
        {
            root = current;
            return (FT_ERR_INVALID_OPERATION);
        }
        current = current - value / derivative;
        difference = std::fabs(value);
        if (difference <= tolerance)
        {
            root = current;
            return (FT_ERR_SUCCESS);
        }
        iteration++;
    }
    root = current;
    return (FT_ERR_INVALID_OPERATION);
}

int32_t math_polynomial_solve_quadratic(double coefficient_a,
    double coefficient_b,
    double coefficient_c,
    double &root_one,
    double &root_two) noexcept
{
    double discriminant;
    double epsilon;
    double stable_root_term;
    double sqrt_discriminant;

    epsilon = 0.000000000001;
    if (std::fabs(coefficient_a) <= epsilon)
    {
        if (std::fabs(coefficient_b) <= epsilon)
        {
            root_one = 0.0;
            root_two = 0.0;
            return (FT_ERR_INVALID_ARGUMENT);
        }
        root_one = -coefficient_c / coefficient_b;
        root_two = root_one;
        return (FT_ERR_SUCCESS);
    }
    discriminant = coefficient_b * coefficient_b - 4.0 * coefficient_a * coefficient_c;
    if (discriminant < 0.0)
    {
        root_one = 0.0;
        root_two = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    sqrt_discriminant = std::sqrt(discriminant);
    if (coefficient_b >= 0.0)
        stable_root_term = -0.5 * (coefficient_b + sqrt_discriminant);
    else
        stable_root_term = -0.5 * (coefficient_b - sqrt_discriminant);
    root_one = stable_root_term / coefficient_a;
    if (std::fabs(stable_root_term) <= epsilon)
        root_two = -coefficient_b / coefficient_a;
    else
        root_two = coefficient_c / stable_root_term;
    return (FT_ERR_SUCCESS);
}

int32_t math_polynomial_lagrange_interpolate(const ft_vector<double> &x_values_input,
    const ft_vector<double> &y_values,
    double input_value,
    double &result) noexcept
{
    ft_size_t count;
    ft_size_t index;
    ft_size_t inner_index;
    double term;
    double denominator;
    double epsilon;

    int32_t x_error = FT_ERR_SUCCESS;
    if (x_error != FT_ERR_SUCCESS)
    {
        result = 0.0;
        return (x_error);
    }
    int32_t y_error = FT_ERR_SUCCESS;
    if (y_error != FT_ERR_SUCCESS)
    {
        result = 0.0;
        return (y_error);
    }
    if (x_values_input.size() == 0 || x_values_input.size() != y_values.size())
    {
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    count = x_values_input.size();
    epsilon = 0.000000000001;
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
                denominator = x_values_input[index] - x_values_input[inner_index];
                if (std::fabs(denominator) <= epsilon)
                {
                    result = 0.0;
                    return (FT_ERR_INVALID_ARGUMENT);
                }
                term = term * (input_value - x_values_input[inner_index]) / denominator;
            }
            inner_index++;
        }
        result += term;
        index++;
    }
    return (FT_ERR_SUCCESS);
}

int32_t math_bezier_evaluate(const ft_vector<double> &control_points,
    double parameter,
    double &result) noexcept
{
    ft_vector<double> working;
    ft_size_t count;
    ft_size_t level;
    ft_size_t index;
    int32_t copy_error;

    int32_t control_error = FT_ERR_SUCCESS;
    if (control_error != FT_ERR_SUCCESS)
    {
        result = 0.0;
        return (control_error);
    }
    if (control_points.size() == 0)
    {
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (parameter < 0.0 || parameter > 1.0)
    {
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    count = control_points.size();
    math_polynomial_copy_vector(control_points, working, count, copy_error);
    if (copy_error != FT_ERR_SUCCESS)
    {
        result = 0.0;
        return (copy_error);
    }
    if (count == 1)
    {
        result = working[0];
        return (FT_ERR_SUCCESS);
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
    return (FT_ERR_SUCCESS);
}

static int32_t math_polynomial_extract_coordinates(const ft_vector<vector2> &control_points,
    ft_vector<double> &x_coordinates,
    ft_vector<double> &y_coordinates) noexcept
{
    ft_size_t count;
    ft_size_t index;
    double x_value;
    double y_value;

    int32_t control_error = FT_ERR_SUCCESS;
    if (control_error != FT_ERR_SUCCESS)
    {
        return (control_error);
    }
    count = control_points.size();
    x_coordinates.resize(count, 0.0);
    int32_t x_resize_error = FT_ERR_SUCCESS;
    if (x_resize_error != FT_ERR_SUCCESS)
    {
        return (x_resize_error);
    }
    y_coordinates.resize(count, 0.0);
    int32_t y_resize_error = FT_ERR_SUCCESS;
    if (y_resize_error != FT_ERR_SUCCESS)
    {
        return (y_resize_error);
    }
    index = 0;
    while (index < count)
    {
        const vector2 &point = control_points[index];

        x_value = point.get_x();
        int32_t point_error = FT_ERR_SUCCESS;
        if (point_error != FT_ERR_SUCCESS)
        {
            return (point_error);
        }
        y_value = point.get_y();
        point_error = FT_ERR_SUCCESS;
        if (point_error != FT_ERR_SUCCESS)
        {
            return (point_error);
        }
        x_coordinates[index] = x_value;
        y_coordinates[index] = y_value;
        index++;
    }
    return (FT_ERR_SUCCESS);
}

int32_t math_bezier_evaluate_vector2(const ft_vector<vector2> &control_points,
    double parameter,
    vector2 &result) noexcept
{
    ft_vector<double> x_coordinates;
    ft_vector<double> y_coordinates;
    double x_value;
    double y_value;
    int32_t coordinate_error;

    if (parameter < 0.0 || parameter > 1.0)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    coordinate_error = math_polynomial_extract_coordinates(control_points,
            x_coordinates,
            y_coordinates);
    if (coordinate_error != FT_ERR_SUCCESS)
    {
        return (coordinate_error);
    }
    int32_t evaluate_error;

    evaluate_error = math_bezier_evaluate(x_coordinates, parameter, x_value);
    if (evaluate_error != FT_ERR_SUCCESS)
    {
        return (evaluate_error);
    }
    evaluate_error = math_bezier_evaluate(y_coordinates, parameter, y_value);
    if (evaluate_error != FT_ERR_SUCCESS)
    {
        return (evaluate_error);
    }
    result.~vector2();
    new (&result) vector2(x_value, y_value);
    int32_t result_error = FT_ERR_SUCCESS;
    if (result_error != FT_ERR_SUCCESS)
    {
        return (result_error);
    }
    return (FT_ERR_SUCCESS);
}

static ft_cubic_spline *math_polynomial_destroy_spline_and_return_null(
    ft_cubic_spline *spline) noexcept
{
    if (spline != ft_nullptr)
        delete spline;
    return (ft_nullptr);
}

ft_cubic_spline *ft_cubic_spline_build(const ft_vector<double> &x_values_input,
    const ft_vector<double> &y_values) noexcept
{
    ft_cubic_spline *spline;
    int32_t spline_initialization_error;
    ft_size_t count;
    ft_size_t segment_count;
    ft_vector<double> h;
    ft_vector<double> alpha;
    ft_size_t index;
    int32_t error_code;
    double epsilon;

    spline = new (std::nothrow) ft_cubic_spline();
    if (spline == ft_nullptr)
        return (ft_nullptr);
    spline_initialization_error = spline->initialize();
    if (spline_initialization_error != FT_ERR_SUCCESS)
        return (math_polynomial_destroy_spline_and_return_null(spline));
    int32_t x_error = FT_ERR_SUCCESS;
    if (x_error != FT_ERR_SUCCESS)
    {
        return (math_polynomial_destroy_spline_and_return_null(spline));
    }
    int32_t y_error = FT_ERR_SUCCESS;
    if (y_error != FT_ERR_SUCCESS)
    {
        return (math_polynomial_destroy_spline_and_return_null(spline));
    }
    if (x_values_input.size() < 2 || x_values_input.size() != y_values.size())
    {
        return (math_polynomial_destroy_spline_and_return_null(spline));
    }
    count = x_values_input.size();
    epsilon = 0.000000000001;
    segment_count = count - 1;
    math_polynomial_copy_vector(x_values_input, spline->_x_values, count, error_code);
    if (error_code != FT_ERR_SUCCESS)
    {
        return (math_polynomial_destroy_spline_and_return_null(spline));
    }
    math_polynomial_copy_vector(y_values, spline->_a_coefficients, count, error_code);
    if (error_code != FT_ERR_SUCCESS)
    {
        return (math_polynomial_destroy_spline_and_return_null(spline));
    }
    spline->_b_coefficients.resize(segment_count, 0.0);
    {
        int32_t resize_error = FT_ERR_SUCCESS;
        if (resize_error != FT_ERR_SUCCESS)
        {
            error_code = resize_error;
            return (math_polynomial_destroy_spline_and_return_null(spline));
        }
    }
    spline->_c_coefficients.resize(count, 0.0);
    {
        int32_t resize_error = FT_ERR_SUCCESS;
        if (resize_error != FT_ERR_SUCCESS)
        {
            error_code = resize_error;
            return (math_polynomial_destroy_spline_and_return_null(spline));
        }
    }
    spline->_d_coefficients.resize(segment_count, 0.0);
    {
        int32_t resize_error = FT_ERR_SUCCESS;
        if (resize_error != FT_ERR_SUCCESS)
        {
            error_code = resize_error;
            return (math_polynomial_destroy_spline_and_return_null(spline));
        }
    }
    h.resize(segment_count, 0.0);
    {
        int32_t resize_error = FT_ERR_SUCCESS;
        if (resize_error != FT_ERR_SUCCESS)
        {
            error_code = resize_error;
            return (math_polynomial_destroy_spline_and_return_null(spline));
        }
    }
    alpha.resize(count, 0.0);
    {
        int32_t resize_error = FT_ERR_SUCCESS;
        if (resize_error != FT_ERR_SUCCESS)
        {
            error_code = resize_error;
            return (math_polynomial_destroy_spline_and_return_null(spline));
        }
    }
    index = 0;
    while (index < segment_count)
    {
        h[index] = spline->_x_values[index + 1] - spline->_x_values[index];
        if (h[index] <= 0.0)
        {
            return (math_polynomial_destroy_spline_and_return_null(spline));
        }
        index++;
    }
    index = 1;
    while (index < segment_count)
    {
        alpha[index] = 3.0 * (spline->_a_coefficients[index + 1] - spline->_a_coefficients[index]) / h[index]
            - 3.0 * (spline->_a_coefficients[index] - spline->_a_coefficients[index - 1]) / h[index - 1];
        index++;
    }
    if (segment_count == 1)
    {
        double slope;

        slope = (spline->_a_coefficients[1] - spline->_a_coefficients[0]) / h[0];
        spline->_b_coefficients[0] = slope;
        spline->_c_coefficients[0] = 0.0;
        spline->_c_coefficients[1] = 0.0;
        spline->_d_coefficients[0] = 0.0;
        return (spline);
    }
    if (segment_count == 2)
    {
        double denominator_two_segments;

        denominator_two_segments = 2.0 * (h[0] + h[1]);
        if (std::fabs(denominator_two_segments) <= epsilon)
        {
            return (math_polynomial_destroy_spline_and_return_null(spline));
        }
        spline->_c_coefficients[0] = 0.0;
        spline->_c_coefficients[1] = alpha[1] / denominator_two_segments;
        spline->_c_coefficients[2] = 0.0;
    }
    else
    {
        ft_vector<double> lower;
        ft_vector<double> diagonal;
        ft_vector<double> upper;
        ft_vector<double> rhs;
        ft_vector<double> interior_c;
        ft_size_t equation_count;
        ft_size_t equation_index;
        ft_size_t last_index;
        ft_size_t current_index;
        double pivot;
        double factor;
        double value;

        equation_count = count - 2;
        lower.resize(equation_count, 0.0);
        {
            int32_t resize_error = FT_ERR_SUCCESS;
            if (resize_error != FT_ERR_SUCCESS)
            {
                error_code = resize_error;
                return (math_polynomial_destroy_spline_and_return_null(spline));
            }
        }
        diagonal.resize(equation_count, 0.0);
        {
            int32_t resize_error = FT_ERR_SUCCESS;
            if (resize_error != FT_ERR_SUCCESS)
            {
                error_code = resize_error;
                return (math_polynomial_destroy_spline_and_return_null(spline));
            }
        }
        upper.resize(equation_count, 0.0);
        {
            int32_t resize_error = FT_ERR_SUCCESS;
            if (resize_error != FT_ERR_SUCCESS)
            {
                error_code = resize_error;
                return (math_polynomial_destroy_spline_and_return_null(spline));
            }
        }
        rhs.resize(equation_count, 0.0);
        {
            int32_t resize_error = FT_ERR_SUCCESS;
            if (resize_error != FT_ERR_SUCCESS)
            {
                error_code = resize_error;
                return (math_polynomial_destroy_spline_and_return_null(spline));
            }
        }
        interior_c.resize(equation_count, 0.0);
        {
            int32_t resize_error = FT_ERR_SUCCESS;
            if (resize_error != FT_ERR_SUCCESS)
            {
                error_code = resize_error;
                return (math_polynomial_destroy_spline_and_return_null(spline));
            }
        }
        diagonal[0] = (h[0] + h[1]) * (h[0] + 2.0 * h[1]);
        upper[0] = (h[1] * h[1]) - (h[0] * h[0]);
        rhs[0] = alpha[1] * h[1];
        equation_index = 1;
        while (equation_index + 1 < equation_count)
        {
            ft_size_t alpha_index;

            alpha_index = equation_index + 1;
            lower[equation_index] = h[alpha_index - 1];
            diagonal[equation_index] = 2.0 * (h[alpha_index - 1] + h[alpha_index]);
            upper[equation_index] = h[alpha_index];
            rhs[equation_index] = alpha[alpha_index];
            equation_index++;
        }
        last_index = equation_count - 1;
        lower[last_index] = (h[segment_count - 2] * h[segment_count - 2]) - (h[segment_count - 1] * h[segment_count - 1]);
        diagonal[last_index] = (h[segment_count - 2] + h[segment_count - 1]) * (2.0 * h[segment_count - 2] + h[segment_count - 1]);
        rhs[last_index] = alpha[count - 2] * h[segment_count - 2];
        equation_index = 1;
        while (equation_index < equation_count)
        {
            pivot = diagonal[equation_index - 1];
            if (std::fabs(pivot) <= epsilon)
            {
                return (math_polynomial_destroy_spline_and_return_null(spline));
            }
            factor = lower[equation_index] / pivot;
            diagonal[equation_index] -= factor * upper[equation_index - 1];
            rhs[equation_index] -= factor * rhs[equation_index - 1];
            lower[equation_index] = 0.0;
            equation_index++;
        }
        equation_index = equation_count;
        while (equation_index > 0)
        {
            equation_index--;
            current_index = equation_index;
            pivot = diagonal[current_index];
            if (std::fabs(pivot) <= epsilon)
            {
                return (math_polynomial_destroy_spline_and_return_null(spline));
            }
            value = rhs[current_index];
            if (current_index + 1 < equation_count)
                value -= upper[current_index] * interior_c[current_index + 1];
            value /= pivot;
            interior_c[current_index] = value;
        }
        spline->_c_coefficients[0] = ((h[0] + h[1]) * interior_c[0] - h[0] * interior_c[1]) / h[1];
        equation_index = 0;
        while (equation_index < equation_count)
        {
            spline->_c_coefficients[equation_index + 1] = interior_c[equation_index];
            equation_index++;
        }
        spline->_c_coefficients[count - 1] = ((h[segment_count - 2] + h[segment_count - 1]) * interior_c[equation_count - 1]
                - h[segment_count - 1] * interior_c[equation_count - 2]) / h[segment_count - 2];
    }
    index = segment_count;
    while (index > 0)
    {
        spline->_b_coefficients[index - 1] = (spline->_a_coefficients[index] - spline->_a_coefficients[index - 1]) / h[index - 1]
            - h[index - 1] * (spline->_c_coefficients[index] + 2.0 * spline->_c_coefficients[index - 1]) / 3.0;
        spline->_d_coefficients[index - 1] = (spline->_c_coefficients[index] - spline->_c_coefficients[index - 1]) / (3.0 * h[index - 1]);
        index--;
    }
    return (spline);
}

double ft_cubic_spline_evaluate(const ft_cubic_spline &spline,
    double input_value) noexcept
{
    ft_size_t segment_count;
    ft_size_t index;
    double clamped_x;
    double delta;
    double value;

    if (spline._x_values.size() < 2)
    {
        return (0.0);
    }
    clamped_x = input_value;
    if (clamped_x < spline._x_values[0])
        clamped_x = spline._x_values[0];
    if (clamped_x > spline._x_values[spline._x_values.size() - 1])
        clamped_x = spline._x_values[spline._x_values.size() - 1];
    segment_count = spline._x_values.size() - 1;
    index = segment_count - 1;
    while (index > 0 && clamped_x < spline._x_values[index])
        index--;
    delta = clamped_x - spline._x_values[index];
    value = spline._a_coefficients[index]
        + spline._b_coefficients[index] * delta
        + spline._c_coefficients[index] * delta * delta
        + spline._d_coefficients[index] * delta * delta * delta;
    return (value);
}

static int32_t math_integrate_trapezoidal_step(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    ft_size_t subdivisions,
    double &result) noexcept
{
    double step;
    ft_size_t index;
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
    return (FT_ERR_SUCCESS);
}

int32_t math_integrate_trapezoidal(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    ft_size_t subdivisions,
    double &result) noexcept
{
    double local_result;
    int32_t step_error;
    double orientation;
    double epsilon;

    if (function == ft_nullptr)
    {
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    epsilon = 0.000000000001;
    if (std::fabs(lower_bound - upper_bound) <= epsilon)
    {
        result = 0.0;
        return (FT_ERR_SUCCESS);
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
    if (step_error != FT_ERR_SUCCESS)
    {
        result = 0.0;
        return (step_error);
    }
    result = orientation * local_result;
    return (FT_ERR_SUCCESS);
}

static int32_t math_integrate_simpson_estimate(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    ft_size_t subdivisions,
    double &result) noexcept
{
    double step;
    ft_size_t index;
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
    return (FT_ERR_SUCCESS);
}

int32_t math_integrate_simpson(math_unary_function function,
    void *user_data,
    double lower_bound,
    double upper_bound,
    double tolerance,
    double &result) noexcept
{
    ft_size_t refinements;
    ft_size_t subdivisions;
    double previous;
    double current;
    int32_t estimate_error;
    double orientation;
    double difference;
    double epsilon;

    if (function == ft_nullptr)
    {
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (tolerance <= 0.0)
    {
        result = 0.0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    epsilon = 0.000000000001;
    if (std::fabs(lower_bound - upper_bound) <= epsilon)
    {
        result = 0.0;
        return (FT_ERR_SUCCESS);
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
        if (estimate_error != FT_ERR_SUCCESS)
        {
            result = 0.0;
            return (estimate_error);
        }
        if (refinements > 0)
        {
            difference = std::fabs(current - previous);
            if (difference <= tolerance + epsilon)
            {
                result = orientation * current;
                return (FT_ERR_SUCCESS);
            }
        }
        previous = current;
        subdivisions *= 2;
        refinements++;
    }
    result = orientation * current;
    return (FT_ERR_INVALID_OPERATION);
}
