#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"

double  vector4::get_x() const
{
    return (this->_x);
}

double  vector4::get_y() const
{
    return (this->_y);
}

double  vector4::get_z() const
{
    return (this->_z);
}

double  vector4::get_w() const
{
    return (this->_w);
}

vector4 vector4::add(const vector4 &other) const
{
    vector4 result;

    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    result._w = this->_w + other._w;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

vector4 vector4::subtract(const vector4 &other) const
{
    vector4 result;

    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result._z = this->_z - other._z;
    result._w = this->_w - other._w;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

double  vector4::dot(const vector4 &other) const
{
    this->set_error(ER_SUCCESS);
    return (this->_x * other._x + this->_y * other._y + this->_z * other._z + this->_w * other._w);
}

double  vector4::length() const
{
    double result;

    result = this->_x * this->_x + this->_y * this->_y + this->_z * this->_z + this->_w * this->_w;
    this->set_error(ER_SUCCESS);
    return (math_sqrt(result));
}

vector4 vector4::normalize() const
{
    double len;
    double epsilon;
    vector4 result;

    len = this->length();
    epsilon = 0.0000001;
    if (math_absdiff(len, 0.0) <= epsilon)
    {
        result.set_error(FT_ERR_INVALID_ARGUMENT);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._x = this->_x / len;
    result._y = this->_y / len;
    result._z = this->_z / len;
    result._w = this->_w / len;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

void    vector4::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     vector4::get_error() const
{
    return (this->_error_code);
}

const char  *vector4::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

