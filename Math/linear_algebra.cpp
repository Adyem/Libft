#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"

vector2::vector2()
{
    this->_x = 0.0;
    this->_y = 0.0;
    this->set_error(ER_SUCCESS);
    return ;
}

vector2::vector2(double x, double y)
{
    this->_x = x;
    this->_y = y;
    this->set_error(ER_SUCCESS);
    return ;
}

vector2::~vector2()
{
    return ;
}

double  vector2::get_x() const
{
    return (this->_x);
}

double  vector2::get_y() const
{
    return (this->_y);
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
    vector2 result;

    len = this->length();
    if (len == 0.0)
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

vector3::vector3()
{
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->set_error(ER_SUCCESS);
    return ;
}

vector3::vector3(double x, double y, double z)
{
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->set_error(ER_SUCCESS);
    return ;
}

vector3::~vector3()
{
    return ;
}

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
    vector3 result;

    len = this->length();
    if (len == 0.0)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    result._x = this->_x / len;
    result._y = this->_y / len;
    result._z = this->_z / len;
    result.set_error(ER_SUCCESS);
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

vector4::vector4()
{
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_w = 0.0;
    this->set_error(ER_SUCCESS);
    return ;
}

vector4::vector4(double x, double y, double z, double w)
{
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->_w = w;
    this->set_error(ER_SUCCESS);
    return ;
}

vector4::~vector4()
{
    return ;
}

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
    vector4 result;

    len = this->length();
    if (len == 0.0)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    result._x = this->_x / len;
    result._y = this->_y / len;
    result._z = this->_z / len;
    result._w = this->_w / len;
    result.set_error(ER_SUCCESS);
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

matrix4::matrix4()
{
    int row;
    int column;

    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            if (row == column)
                this->_m[row][column] = 1.0;
            else
                this->_m[row][column] = 0.0;
            column++;
        }
        row++;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

matrix4::matrix4(double m00, double m01, double m02, double m03,
        double m10, double m11, double m12, double m13,
        double m20, double m21, double m22, double m23,
        double m30, double m31, double m32, double m33)
{
    this->_m[0][0] = m00;
    this->_m[0][1] = m01;
    this->_m[0][2] = m02;
    this->_m[0][3] = m03;
    this->_m[1][0] = m10;
    this->_m[1][1] = m11;
    this->_m[1][2] = m12;
    this->_m[1][3] = m13;
    this->_m[2][0] = m20;
    this->_m[2][1] = m21;
    this->_m[2][2] = m22;
    this->_m[2][3] = m23;
    this->_m[3][0] = m30;
    this->_m[3][1] = m31;
    this->_m[3][2] = m32;
    this->_m[3][3] = m33;
    this->set_error(ER_SUCCESS);
    return ;
}

matrix4::~matrix4()
{
    return ;
}

vector4 matrix4::transform(const vector4 &vector) const
{
    vector4 result(
        this->_m[0][0] * vector.get_x() + this->_m[0][1] * vector.get_y() + this->_m[0][2] * vector.get_z() + this->_m[0][3] * vector.get_w(),
        this->_m[1][0] * vector.get_x() + this->_m[1][1] * vector.get_y() + this->_m[1][2] * vector.get_z() + this->_m[1][3] * vector.get_w(),
        this->_m[2][0] * vector.get_x() + this->_m[2][1] * vector.get_y() + this->_m[2][2] * vector.get_z() + this->_m[2][3] * vector.get_w(),
        this->_m[3][0] * vector.get_x() + this->_m[3][1] * vector.get_y() + this->_m[3][2] * vector.get_z() + this->_m[3][3] * vector.get_w()
    );
    this->set_error(ER_SUCCESS);
    return (result);
}


matrix4 matrix4::multiply(const matrix4 &other) const
{
    matrix4 result;

    result._m[0][0] = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0] + this->_m[0][2] * other._m[2][0] + this->_m[0][3] * other._m[3][0];
    result._m[0][1] = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1] + this->_m[0][2] * other._m[2][1] + this->_m[0][3] * other._m[3][1];
    result._m[0][2] = this->_m[0][0] * other._m[0][2] + this->_m[0][1] * other._m[1][2] + this->_m[0][2] * other._m[2][2] + this->_m[0][3] * other._m[3][2];
    result._m[0][3] = this->_m[0][0] * other._m[0][3] + this->_m[0][1] * other._m[1][3] + this->_m[0][2] * other._m[2][3] + this->_m[0][3] * other._m[3][3];
    result._m[1][0] = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0] + this->_m[1][2] * other._m[2][0] + this->_m[1][3] * other._m[3][0];
    result._m[1][1] = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1] + this->_m[1][2] * other._m[2][1] + this->_m[1][3] * other._m[3][1];
    result._m[1][2] = this->_m[1][0] * other._m[0][2] + this->_m[1][1] * other._m[1][2] + this->_m[1][2] * other._m[2][2] + this->_m[1][3] * other._m[3][2];
    result._m[1][3] = this->_m[1][0] * other._m[0][3] + this->_m[1][1] * other._m[1][3] + this->_m[1][2] * other._m[2][3] + this->_m[1][3] * other._m[3][3];
    result._m[2][0] = this->_m[2][0] * other._m[0][0] + this->_m[2][1] * other._m[1][0] + this->_m[2][2] * other._m[2][0] + this->_m[2][3] * other._m[3][0];
    result._m[2][1] = this->_m[2][0] * other._m[0][1] + this->_m[2][1] * other._m[1][1] + this->_m[2][2] * other._m[2][1] + this->_m[2][3] * other._m[3][1];
    result._m[2][2] = this->_m[2][0] * other._m[0][2] + this->_m[2][1] * other._m[1][2] + this->_m[2][2] * other._m[2][2] + this->_m[2][3] * other._m[3][2];
    result._m[2][3] = this->_m[2][0] * other._m[0][3] + this->_m[2][1] * other._m[1][3] + this->_m[2][2] * other._m[2][3] + this->_m[2][3] * other._m[3][3];
    result._m[3][0] = this->_m[3][0] * other._m[0][0] + this->_m[3][1] * other._m[1][0] + this->_m[3][2] * other._m[2][0] + this->_m[3][3] * other._m[3][0];
    result._m[3][1] = this->_m[3][0] * other._m[0][1] + this->_m[3][1] * other._m[1][1] + this->_m[3][2] * other._m[2][1] + this->_m[3][3] * other._m[3][1];
    result._m[3][2] = this->_m[3][0] * other._m[0][2] + this->_m[3][1] * other._m[1][2] + this->_m[3][2] * other._m[2][2] + this->_m[3][3] * other._m[3][2];
    result._m[3][3] = this->_m[3][0] * other._m[0][3] + this->_m[3][1] * other._m[1][3] + this->_m[3][2] * other._m[2][3] + this->_m[3][3] * other._m[3][3];
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

matrix4 matrix4::invert() const
{
    double temp[4][8];
    matrix4 result;
    int row;
    int column;
    int other;
    double pivot;
    double factor;

    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            temp[row][column] = this->_m[row][column];
            column++;
        }
        column = 0;
        while (column < 4)
        {
            if (row == column)
                temp[row][column + 4] = 1.0;
            else
                temp[row][column + 4] = 0.0;
            column++;
        }
        row++;
    }
    row = 0;
    while (row < 4)
    {
        pivot = temp[row][row];
        if (math_fabs(pivot) < 0.000001)
        {
            result.set_error(FT_EINVAL);
            return (result);
        }
        column = 0;
        while (column < 8)
        {
            temp[row][column] = temp[row][column] / pivot;
            column++;
        }
        other = 0;
        while (other < 4)
        {
            if (other != row)
            {
                factor = temp[other][row];
                column = 0;
                while (column < 8)
                {
                    temp[other][column] = temp[other][column] - factor * temp[row][column];
                    column++;
                }
            }
            other++;
        }
        row++;
    }
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            result._m[row][column] = temp[row][column + 4];
            column++;
        }
        row++;
    }
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

void    matrix4::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     matrix4::get_error() const
{
    return (this->_error_code);
}

const char  *matrix4::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

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

double      quaternion::get_w() const
{
    return (this->_w);
}

double      quaternion::get_x() const
{
    return (this->_x);
}

double      quaternion::get_y() const
{
    return (this->_y);
}

double      quaternion::get_z() const
{
    return (this->_z);
}

quaternion  quaternion::multiply(const quaternion &other) const
{
    quaternion result;

    result._w = this->_w * other._w - this->_x * other._x - this->_y * other._y - this->_z * other._z;
    result._x = this->_w * other._x + this->_x * other._w + this->_y * other._z - this->_z * other._y;
    result._y = this->_w * other._y - this->_x * other._z + this->_y * other._w + this->_z * other._x;
    result._z = this->_w * other._z + this->_x * other._y - this->_y * other._x + this->_z * other._w;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

double      quaternion::length() const
{
    double result;

    result = this->_w * this->_w + this->_x * this->_x + this->_y * this->_y + this->_z * this->_z;
    this->set_error(ER_SUCCESS);
    return (math_sqrt(result));
}

quaternion  quaternion::normalize() const
{
    double len;
    quaternion result;

    len = this->length();
    if (len == 0.0)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    result._w = this->_w / len;
    result._x = this->_x / len;
    result._y = this->_y / len;
    result._z = this->_z / len;
    result.set_error(ER_SUCCESS);
    return (result);
}

quaternion  quaternion::invert() const
{
    double len_sq;
    quaternion result;

    len_sq = this->_w * this->_w + this->_x * this->_x + this->_y * this->_y + this->_z * this->_z;
    if (len_sq == 0.0)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    result._w = this->_w / len_sq;
    result._x = -this->_x / len_sq;
    result._y = -this->_y / len_sq;
    result._z = -this->_z / len_sq;
    result.set_error(ER_SUCCESS);
    return (result);
}

vector3     quaternion::transform(const vector3 &vector) const
{
    quaternion vec_q;
    quaternion inv;
    quaternion res;

    vec_q._w = 0.0;
    vec_q._x = vector.get_x();
    vec_q._y = vector.get_y();
    vec_q._z = vector.get_z();
    inv = this->invert();
    if (inv.get_error() != ER_SUCCESS)
    {
        this->set_error(inv.get_error());
        return (vector3());
    }
    res = this->multiply(vec_q).multiply(inv);
    this->set_error(ER_SUCCESS);
    return (vector3(res._x, res._y, res._z));
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
