#include "math_autodiff.hpp"

#include <cmath>
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread_internal.hpp"

static int math_autodiff_lock_pair(pt_recursive_mutex *first_mutex,
        pt_recursive_mutex *second_mutex,
        pt_recursive_mutex *&lower_mutex,
        pt_recursive_mutex *&upper_mutex)
{
    lower_mutex = first_mutex;
    upper_mutex = second_mutex;
    if (lower_mutex > upper_mutex && lower_mutex != upper_mutex)
    {
        pt_recursive_mutex *temporary = lower_mutex;
        lower_mutex = upper_mutex;
        upper_mutex = temporary;
    }
    int lock_error = pt_recursive_mutex_lock_if_valid(lower_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    if (lower_mutex == upper_mutex)
        return (FT_ERR_SUCCESSS);
    lock_error = pt_recursive_mutex_lock_if_valid(upper_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        int unlock_error = pt_recursive_mutex_unlock_if_valid(lower_mutex);
        if (unlock_error == FT_ERR_SUCCESSS)
            ft_global_error_stack_drop_last_error();
        return (lock_error);
    }
    return (FT_ERR_SUCCESSS);
}

static void math_autodiff_unlock_pair(pt_recursive_mutex *lower_mutex,
        pt_recursive_mutex *upper_mutex)
{
    if (upper_mutex != lower_mutex)
        pt_recursive_mutex_unlock_if_valid(upper_mutex);
    pt_recursive_mutex_unlock_if_valid(lower_mutex);
}

ft_dual_number::ft_dual_number() noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
{
    return ;
}

ft_dual_number::ft_dual_number(double value, double derivative) noexcept
    : _value(value)
    , _derivative(derivative)
    , _mutex(ft_nullptr)
{
    return ;
}

ft_dual_number::ft_dual_number(const ft_dual_number &other) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
{
    *this = other;
    return ;
}

ft_dual_number::ft_dual_number(ft_dual_number &&other) noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _mutex(ft_nullptr)
{
    *this = ft_move(other);
    return ;
}

ft_dual_number::~ft_dual_number() noexcept
{
    this->disable_thread_safety();
    return ;
}

ft_dual_number &ft_dual_number::operator=(const ft_dual_number &other) noexcept
{
    if (this == &other)
        return (*this);
    pt_recursive_mutex *lower_mutex = ft_nullptr;
    pt_recursive_mutex *upper_mutex = ft_nullptr;
    int lock_error = math_autodiff_lock_pair(this->_mutex, other._mutex, lower_mutex, upper_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (*this);
    this->_value = other._value;
    this->_derivative = other._derivative;
    math_autodiff_unlock_pair(lower_mutex, upper_mutex);
    return (*this);
}

ft_dual_number &ft_dual_number::operator=(ft_dual_number &&other) noexcept
{
    if (this == &other)
        return (*this);
    pt_recursive_mutex *lower_mutex = ft_nullptr;
    pt_recursive_mutex *upper_mutex = ft_nullptr;
    int lock_error = math_autodiff_lock_pair(this->_mutex, other._mutex, lower_mutex, upper_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (*this);
    this->_value = other._value;
    this->_derivative = other._derivative;
    other._value = 0.0;
    other._derivative = 0.0;
    math_autodiff_unlock_pair(lower_mutex, upper_mutex);
    return (*this);
}

ft_dual_number ft_dual_number::constant(double value) noexcept
{
    return (ft_dual_number(value, 0.0));
}

ft_dual_number ft_dual_number::variable(double value) noexcept
{
    return (ft_dual_number(value, 1.0));
}

int ft_dual_number::lock_mutex() const noexcept
{
    return (pt_recursive_mutex_lock_if_enabled(
        this->_mutex,
        this->_mutex != ft_nullptr
    ));
}

int ft_dual_number::unlock_mutex() const noexcept
{
    return (pt_recursive_mutex_unlock_if_enabled(
        this->_mutex,
        this->_mutex != ft_nullptr
    ));
}

int ft_dual_number::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_dual_number::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_dual_number::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void ft_dual_number::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_dual_number::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

double ft_dual_number::value() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (0.0);
    double result = this->_value;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (result);
    return (result);
}

double ft_dual_number::derivative() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (0.0);
    double result = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (result);
    return (result);
}

ft_dual_number ft_dual_number::operator+(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    pt_recursive_mutex *lower_mutex = ft_nullptr;
    pt_recursive_mutex *upper_mutex = ft_nullptr;
    int lock_error = math_autodiff_lock_pair(this->_mutex, other._mutex, lower_mutex, upper_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    result._value = this->_value + other._value;
    result._derivative = this->_derivative + other._derivative;
    math_autodiff_unlock_pair(lower_mutex, upper_mutex);
    return (result);
}

ft_dual_number ft_dual_number::operator-(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    pt_recursive_mutex *lower_mutex = ft_nullptr;
    pt_recursive_mutex *upper_mutex = ft_nullptr;
    int lock_error = math_autodiff_lock_pair(this->_mutex, other._mutex, lower_mutex, upper_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    result._value = this->_value - other._value;
    result._derivative = this->_derivative - other._derivative;
    math_autodiff_unlock_pair(lower_mutex, upper_mutex);
    return (result);
}

ft_dual_number ft_dual_number::operator*(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    pt_recursive_mutex *lower_mutex = ft_nullptr;
    pt_recursive_mutex *upper_mutex = ft_nullptr;
    int lock_error = math_autodiff_lock_pair(this->_mutex, other._mutex, lower_mutex, upper_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    result._value = this->_value * other._value;
    result._derivative = this->_value * other._derivative
        + this->_derivative * other._value;
    math_autodiff_unlock_pair(lower_mutex, upper_mutex);
    return (result);
}

ft_dual_number ft_dual_number::operator/(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    double epsilon = 0.000000000001;
    double denominator;
    double value;
    double derivative;
    double other_derivative;
    bool invalid_divisor = false;
    pt_recursive_mutex *lower_mutex = ft_nullptr;
    pt_recursive_mutex *upper_mutex = ft_nullptr;

    int lock_error = math_autodiff_lock_pair(this->_mutex, other._mutex, lower_mutex, upper_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    denominator = other._value;
    value = this->_value;
    derivative = this->_derivative;
    other_derivative = other._derivative;
    if (std::fabs(denominator) <= epsilon)
    {
        invalid_divisor = true;
        result._value = 0.0;
        result._derivative = 0.0;
    }
    else
    {
        result._value = value / denominator;
        result._derivative = (derivative * denominator - value * other_derivative)
            / (denominator * denominator);
    }
    math_autodiff_unlock_pair(lower_mutex, upper_mutex);
    if (invalid_divisor)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    return (result);
}

ft_dual_number ft_dual_number::apply_sin() const noexcept
{
    ft_dual_number result;
    double value;
    double derivative_value;
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (result);
    result._value = std::sin(value);
    result._derivative = std::cos(value) * derivative_value;
    return (result);
}

ft_dual_number ft_dual_number::apply_cos() const noexcept
{
    ft_dual_number result;
    double value;
    double derivative_value;
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (result);
    result._value = std::cos(value);
    result._derivative = -std::sin(value) * derivative_value;
    return (result);
}

ft_dual_number ft_dual_number::apply_exp() const noexcept
{
    ft_dual_number result;
    double exponential;
    double value;
    double derivative_value;
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (result);
    exponential = std::exp(value);
    result._value = exponential;
    result._derivative = exponential * derivative_value;
    return (result);
}

ft_dual_number ft_dual_number::apply_log() const noexcept
{
    ft_dual_number result;
    double value;
    double derivative_value;
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (result);
    if (value <= 0.0)
    {
        result._value = 0.0;
        result._derivative = 0.0;
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._value = std::log(value);
    result._derivative = derivative_value / value;
    return (result);
}

pt_recursive_mutex *ft_dual_number::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}

static int math_autodiff_prepare_inputs(const ft_vector<double> &point,
    size_t active_index, ft_vector<ft_dual_number> &dual_inputs) noexcept
{
    size_t dimension;
    size_t index;

    dual_inputs.clear();
    dimension = point.size();
    dual_inputs.reserve(dimension);
    {
        int reserve_error = ft_global_error_stack_peek_last_error();
        if (reserve_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(reserve_error);
            return (-1);
        }
    }
    index = 0;
    while (index < dimension)
    {
        ft_dual_number variable;
        double value;

        value = point[index];
        if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
            return (-1);
        if (index == active_index)
            variable = ft_dual_number::variable(value);
        else
            variable = ft_dual_number::constant(value);
        dual_inputs.push_back(variable);
        {
            int push_error = ft_global_error_stack_peek_last_error();
            if (push_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(push_error);
                return (-1);
            }
        }
        index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int math_autodiff_univariate(math_autodiff_univariate_function function,
    double point, double *value, double *derivative, void *user_data) noexcept
{
    ft_dual_number variable;
    ft_dual_number result;

    if (function == ft_nullptr || value == ft_nullptr || derivative == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    variable = ft_dual_number::variable(point);
    result = function(variable, user_data);
    {
        int result_error = ft_global_error_stack_peek_last_error();

        if (result_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(result_error);
            return (-1);
        }
    }
    *value = result.value();
    *derivative = result.derivative();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int math_autodiff_gradient(math_autodiff_multivariate_function function,
    const ft_vector<double> &point, ft_vector<double> &gradient,
    double *value, void *user_data) noexcept
{
    size_t dimension;
    size_t index;
    bool value_set;

    if (function == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    dimension = point.size();
    gradient.clear();
    gradient.reserve(dimension);
    {
        int reserve_error = ft_global_error_stack_peek_last_error();
        if (reserve_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(reserve_error);
            return (-1);
        }
    }
    index = 0;
    value_set = false;
    while (index < dimension)
    {
        ft_vector<ft_dual_number> dual_inputs;
        ft_dual_number result;

        if (math_autodiff_prepare_inputs(point, index, dual_inputs) != 0)
        {
            gradient.clear();
            return (-1);
        }
        result = function(dual_inputs, user_data);
        {
            int result_error = ft_global_error_stack_peek_last_error();

            if (result_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(result_error);
                gradient.clear();
                return (-1);
            }
        }
        if (!value_set && value != ft_nullptr)
        {
            *value = result.value();
            value_set = true;
        }
        gradient.push_back(result.derivative());
        {
            int push_error = ft_global_error_stack_peek_last_error();
            if (push_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(push_error);
                gradient.clear();
                return (-1);
            }
        }
        index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
