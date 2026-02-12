#include "math_autodiff.hpp"

#include <cmath>
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <new>
#include "../Template/move.hpp"

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
    int lock_error;

    if (lower_mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = lower_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lower_mutex == upper_mutex)
        return (FT_ERR_SUCCESS);
    if (upper_mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = upper_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        int unlock_error;

        if (lower_mutex == ft_nullptr)
            unlock_error = FT_ERR_SUCCESS;
        else
            unlock_error = lower_mutex->unlock();
        if (unlock_error != FT_ERR_SUCCESS)
            return (lock_error);
        return (lock_error);
    }
    return (FT_ERR_SUCCESS);
}

static void math_autodiff_unlock_pair(pt_recursive_mutex *lower_mutex,
        pt_recursive_mutex *upper_mutex)
{    if (upper_mutex != lower_mutex && upper_mutex != ft_nullptr)
        upper_mutex->unlock();
    if (lower_mutex != ft_nullptr)
        lower_mutex->unlock();
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
    if (lock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
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
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_dual_number::unlock_mutex() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_dual_number::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

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

void ft_dual_number::teardown_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
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
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    double result = this->_value;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (result);
    return (result);
}

double ft_dual_number::derivative() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    double result = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (result);
    return (result);
}

ft_dual_number ft_dual_number::operator+(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    pt_recursive_mutex *lower_mutex = ft_nullptr;
    pt_recursive_mutex *upper_mutex = ft_nullptr;
    int lock_error = math_autodiff_lock_pair(this->_mutex, other._mutex, lower_mutex, upper_mutex);
    if (lock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
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
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    value = this->_value;
    derivative_value = this->_derivative;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (result);
    if (value <= 0.0)
    {
        result._value = 0.0;
        result._derivative = 0.0;
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
        int reserve_error = FT_ERR_SUCCESS;
        if (reserve_error != FT_ERR_SUCCESS)
        {
            return (-1);
        }
    }
    index = 0;
    while (index < dimension)
    {
        ft_dual_number variable;
        double value;

        value = point[index];
        if (FT_ERR_SUCCESS != FT_ERR_SUCCESS)
            return (-1);
        if (index == active_index)
            variable = ft_dual_number::variable(value);
        else
            variable = ft_dual_number::constant(value);
        dual_inputs.push_back(variable);
        {
            int push_error = FT_ERR_SUCCESS;
            if (push_error != FT_ERR_SUCCESS)
            {
                return (-1);
            }
        }
        index++;
    }
    return (0);
}

int math_autodiff_univariate(math_autodiff_univariate_function function,
    double point, double *value, double *derivative, void *user_data) noexcept
{
    ft_dual_number variable;
    ft_dual_number result;

    if (function == ft_nullptr || value == ft_nullptr || derivative == ft_nullptr)
    {
        return (-1);
    }
    variable = ft_dual_number::variable(point);
    result = function(variable, user_data);
    {
        int result_error = FT_ERR_SUCCESS;

        if (result_error != FT_ERR_SUCCESS)
        {
            return (-1);
        }
    }
    *value = result.value();
    *derivative = result.derivative();
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
        return (-1);
    }
    dimension = point.size();
    gradient.clear();
    gradient.reserve(dimension);
    {
        int reserve_error = FT_ERR_SUCCESS;
        if (reserve_error != FT_ERR_SUCCESS)
        {
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
            int result_error = FT_ERR_SUCCESS;

            if (result_error != FT_ERR_SUCCESS)
            {
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
            int push_error = FT_ERR_SUCCESS;
            if (push_error != FT_ERR_SUCCESS)
            {
                gradient.clear();
                return (-1);
            }
        }
        index++;
    }
    return (0);
}
