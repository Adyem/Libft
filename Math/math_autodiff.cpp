#include "math_autodiff.hpp"

#include <cmath>
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"

ft_dual_number::ft_dual_number() noexcept
    : _value(0.0)
    , _derivative(0.0)
    , _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_dual_number::ft_dual_number(double value, double derivative) noexcept
    : _value(value)
    , _derivative(derivative)
    , _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_dual_number::ft_dual_number(const ft_dual_number &other) noexcept
    : _value(other._value)
    , _derivative(other._derivative)
    , _error_code(other._error_code)
{
    return ;
}

ft_dual_number::ft_dual_number(ft_dual_number &&other) noexcept
    : _value(other._value)
    , _derivative(other._derivative)
    , _error_code(other._error_code)
{
    other._value = 0.0;
    other._derivative = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    return ;
}

ft_dual_number::~ft_dual_number() noexcept
{
    return ;
}

ft_dual_number &ft_dual_number::operator=(const ft_dual_number &other) noexcept
{
    if (this == &other)
        return (*this);
    this->_value = other._value;
    this->_derivative = other._derivative;
    this->_error_code = other._error_code;
    return (*this);
}

ft_dual_number &ft_dual_number::operator=(ft_dual_number &&other) noexcept
{
    if (this == &other)
        return (*this);
    this->_value = other._value;
    this->_derivative = other._derivative;
    this->_error_code = other._error_code;
    other._value = 0.0;
    other._derivative = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    return (*this);
}

void ft_dual_number::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

ft_dual_number ft_dual_number::constant(double value) noexcept
{
    ft_dual_number result(value, 0.0);

    result._error_code = FT_ER_SUCCESSS;
    return (result);
}

ft_dual_number ft_dual_number::variable(double value) noexcept
{
    ft_dual_number result(value, 1.0);

    result._error_code = FT_ER_SUCCESSS;
    return (result);
}

double ft_dual_number::value() const noexcept
{
    return (this->_value);
}

double ft_dual_number::derivative() const noexcept
{
    return (this->_derivative);
}

ft_dual_number ft_dual_number::operator+(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;

    result._value = this->_value + other._value;
    result._derivative = this->_derivative + other._derivative;
    result._error_code = FT_ER_SUCCESSS;
    if (this->_error_code != FT_ER_SUCCESSS)
        result.set_error(this->_error_code);
    if (other._error_code != FT_ER_SUCCESSS)
        result.set_error(other._error_code);
    return (result);
}

ft_dual_number ft_dual_number::operator-(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;

    result._value = this->_value - other._value;
    result._derivative = this->_derivative - other._derivative;
    result._error_code = FT_ER_SUCCESSS;
    if (this->_error_code != FT_ER_SUCCESSS)
        result.set_error(this->_error_code);
    if (other._error_code != FT_ER_SUCCESSS)
        result.set_error(other._error_code);
    return (result);
}

ft_dual_number ft_dual_number::operator*(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;

    result._value = this->_value * other._value;
    result._derivative = this->_value * other._derivative
        + this->_derivative * other._value;
    result._error_code = FT_ER_SUCCESSS;
    if (this->_error_code != FT_ER_SUCCESSS)
        result.set_error(this->_error_code);
    if (other._error_code != FT_ER_SUCCESSS)
        result.set_error(other._error_code);
    return (result);
}

ft_dual_number ft_dual_number::operator/(const ft_dual_number &other) const noexcept
{
    ft_dual_number result;
    double epsilon;

    epsilon = 0.000000000001;
    if (std::fabs(other._value) <= epsilon)
    {
        result._value = 0.0;
        result._derivative = 0.0;
        result.set_error(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._value = this->_value / other._value;
    result._derivative = (this->_derivative * other._value
        - this->_value * other._derivative)
        / (other._value * other._value);
    result._error_code = FT_ER_SUCCESSS;
    if (this->_error_code != FT_ER_SUCCESSS)
        result.set_error(this->_error_code);
    if (other._error_code != FT_ER_SUCCESSS)
        result.set_error(other._error_code);
    return (result);
}

ft_dual_number ft_dual_number::apply_sin() const noexcept
{
    ft_dual_number result;

    result._value = std::sin(this->_value);
    result._derivative = std::cos(this->_value) * this->_derivative;
    result._error_code = this->_error_code;
    if (result._error_code != FT_ER_SUCCESSS)
        result.set_error(result._error_code);
    return (result);
}

ft_dual_number ft_dual_number::apply_cos() const noexcept
{
    ft_dual_number result;

    result._value = std::cos(this->_value);
    result._derivative = -std::sin(this->_value) * this->_derivative;
    result._error_code = this->_error_code;
    if (result._error_code != FT_ER_SUCCESSS)
        result.set_error(result._error_code);
    return (result);
}

ft_dual_number ft_dual_number::apply_exp() const noexcept
{
    ft_dual_number result;
    double exponential;

    exponential = std::exp(this->_value);
    result._value = exponential;
    result._derivative = exponential * this->_derivative;
    result._error_code = this->_error_code;
    if (result._error_code != FT_ER_SUCCESSS)
        result.set_error(result._error_code);
    return (result);
}

ft_dual_number ft_dual_number::apply_log() const noexcept
{
    ft_dual_number result;

    if (this->_value <= 0.0)
    {
        result._value = 0.0;
        result._derivative = 0.0;
        result.set_error(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._value = std::log(this->_value);
    result._derivative = this->_derivative / this->_value;
    result._error_code = this->_error_code;
    if (result._error_code != FT_ER_SUCCESSS)
        result.set_error(result._error_code);
    return (result);
}

int ft_dual_number::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_dual_number::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

static int math_autodiff_prepare_inputs(const ft_vector<double> &point,
    size_t active_index, ft_vector<ft_dual_number> &dual_inputs) noexcept
{
    size_t dimension;
    size_t index;

    dual_inputs.clear();
    dimension = point.size();
    dual_inputs.reserve(dimension);
    if (dual_inputs.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = dual_inputs.get_error();
        return (-1);
    }
    index = 0;
    while (index < dimension)
    {
        ft_dual_number variable;
        double value;

        value = point[index];
        if (dual_inputs.get_error() != FT_ER_SUCCESSS)
            return (-1);
        if (index == active_index)
            variable = ft_dual_number::variable(value);
        else
            variable = ft_dual_number::constant(value);
        dual_inputs.push_back(variable);
        if (dual_inputs.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = dual_inputs.get_error();
            return (-1);
        }
        index++;
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int math_autodiff_univariate(math_autodiff_univariate_function function,
    double point, double *value, double *derivative, void *user_data) noexcept
{
    ft_dual_number variable;
    ft_dual_number result;

    if (function == ft_nullptr || value == ft_nullptr || derivative == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    variable = ft_dual_number::variable(point);
    result = function(variable, user_data);
    if (result.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = result.get_error();
        return (-1);
    }
    *value = result.value();
    *derivative = result.derivative();
    ft_errno = FT_ER_SUCCESSS;
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
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    dimension = point.size();
    gradient.clear();
    gradient.reserve(dimension);
    if (gradient.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = gradient.get_error();
        return (-1);
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
        if (result.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = result.get_error();
            gradient.clear();
            return (-1);
        }
        if (!value_set && value != ft_nullptr)
        {
            *value = result.value();
            value_set = true;
        }
        gradient.push_back(result.derivative());
        if (gradient.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = gradient.get_error();
            gradient.clear();
            return (-1);
        }
        index++;
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}
