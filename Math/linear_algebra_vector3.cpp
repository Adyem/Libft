#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"

double  vector3::get_x() const
{
    return (this->_x);
}

double  vector3::get_y() const
{
    return (this->_y);
}

double  vector3::get_z() const
{
    return (this->_z);
}

vector3 vector3::add(const vector3 &other) const
{
    vector3 result;

    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

vector3 vector3::subtract(const vector3 &other) const
{
    vector3 result;

    result._x = this->_x - other._x;
    result._y = this->_y - other._y;
    result._z = this->_z - other._z;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

double  vector3::dot(const vector3 &other) const
{
    this->set_error(ER_SUCCESS);
    return (this->_x * other._x + this->_y * other._y + this->_z * other._z);
}

vector3 vector3::cross(const vector3 &other) const
{
    vector3 result;

    result._x = this->_y * other._z - this->_z * other._y;
    result._y = this->_z * other._x - this->_x * other._z;
    result._z = this->_x * other._y - this->_y * other._x;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

double  vector3::length() const
{
    double result;

    result = this->_x * this->_x + this->_y * this->_y + this->_z * this->_z;
    this->set_error(ER_SUCCESS);
    return (math_sqrt(result));
}

vector3 vector3::normalize() const
{
    double len;
    double epsilon;
    vector3 result;

    len = this->length();
    epsilon = 0.0000001;
    if (math_absdiff(len, 0.0) <= epsilon)
    {
        result.set_error(FT_EINVAL);
        this->set_error(FT_EINVAL);
        return (result);
    }
    result._x = this->_x / len;
    result._y = this->_y / len;
    result._z = this->_z / len;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

void    vector3::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     vector3::get_error() const
{
    return (this->_error_code);
}

const char  *vector3::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

