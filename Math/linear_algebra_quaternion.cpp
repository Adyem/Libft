#include "linear_algebra_quaternion.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"

quaternion::quaternion()
{
    this->_w = 1.0;
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->set_error(ER_SUCCESS);
    return ;
}

quaternion::quaternion(double w, double x, double y, double z)
{
    this->_w = w;
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->set_error(ER_SUCCESS);
    return ;
}

quaternion::~quaternion()
{
    return ;
}

double  quaternion::get_w() const
{
    return (this->_w);
}

double  quaternion::get_x() const
{
    return (this->_x);
}

double  quaternion::get_y() const
{
    return (this->_y);
}

double  quaternion::get_z() const
{
    return (this->_z);
}

quaternion  quaternion::add(const quaternion &other) const
{
    quaternion result;

    result._w = this->_w + other._w;
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

quaternion  quaternion::multiply(const quaternion &other) const
{
    quaternion result;

    result._w = this->_w * other._w - this->_x * other._x
        - this->_y * other._y - this->_z * other._z;
    result._x = this->_w * other._x + this->_x * other._w
        + this->_y * other._z - this->_z * other._y;
    result._y = this->_w * other._y - this->_x * other._z
        + this->_y * other._w + this->_z * other._x;
    result._z = this->_w * other._z + this->_x * other._y
        - this->_y * other._x + this->_z * other._w;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

quaternion  quaternion::conjugate() const
{
    quaternion result;

    result._w = this->_w;
    result._x = -this->_x;
    result._y = -this->_y;
    result._z = -this->_z;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

double  quaternion::length() const
{
    double length_value;

    length_value = this->_w * this->_w + this->_x * this->_x + this->_y * this->_y + this->_z * this->_z;
    this->set_error(ER_SUCCESS);
    return (math_sqrt(length_value));
}

quaternion  quaternion::normalize() const
{
    quaternion result;
    double length_value;
    double epsilon;

    length_value = this->length();
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    result._w = this->_w / length_value;
    result._x = this->_x / length_value;
    result._y = this->_y / length_value;
    result._z = this->_z / length_value;
    result.set_error(ER_SUCCESS);
    return (result);
}

void    quaternion::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     quaternion::get_error() const
{
    return (this->_error_code);
}

const char  *quaternion::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

