#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"

double  vector2::get_x() const
{
    return (this->_x);
}

double  vector2::get_y() const
{
    return (this->_y);
}

vector2 vector2::add(const vector2 &other) const
{
    vector2 result;

    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

vector2 vector2::subtract(const vector2 &other) const
{
    vector2 result;

    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

double  vector2::dot(const vector2 &other) const
{
    this->set_error(ER_SUCCESS);
    return (this->_x * other._x + this->_y * other._y);
}

double  vector2::length() const
{
    double result;

    result = this->_x * this->_x + this->_y * this->_y;
    this->set_error(ER_SUCCESS);
    return (math_sqrt(result));
}

vector2 vector2::normalize() const
{
    double len;
    double epsilon;
    vector2 result;

    len = this->length();
    epsilon = 0.0000001;
    if (math_absdiff(len, 0.0) <= epsilon)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    result._x = this->_x / len;
    result._y = this->_y / len;
    result.set_error(ER_SUCCESS);
    return (result);
}

void    vector2::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     vector2::get_error() const
{
    return (this->_error_code);
}

const char  *vector2::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

