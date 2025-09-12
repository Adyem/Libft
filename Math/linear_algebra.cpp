#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"

vector2 matrix2::transform(const vector2 &vector) const
{
    vector2 result(
        this->_m[0][0] * vector.get_x() + this->_m[0][1] * vector.get_y(),
        this->_m[1][0] * vector.get_x() + this->_m[1][1] * vector.get_y()
    );
    this->set_error(ER_SUCCESS);
    return (result);
}

matrix2 matrix2::multiply(const matrix2 &other) const
{
    matrix2 result;

    result._m[0][0] = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0];
    result._m[0][1] = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1];
    result._m[1][0] = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0];
    result._m[1][1] = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1];
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

matrix2 matrix2::invert() const
{
    double determinant;
    double epsilon;
    matrix2 result;

    determinant = this->_m[0][0] * this->_m[1][1] - this->_m[0][1] * this->_m[1][0];
    epsilon = 0.000001;
    if (math_fabs(determinant) < epsilon)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    result._m[0][0] = this->_m[1][1] / determinant;
    result._m[0][1] = -this->_m[0][1] / determinant;
    result._m[1][0] = -this->_m[1][0] / determinant;
    result._m[1][1] = this->_m[0][0] / determinant;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

void    matrix2::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     matrix2::get_error() const
{
    return (this->_error_code);
}

const char  *matrix2::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

vector3 matrix3::transform(const vector3 &vector) const
{
    vector3 result(
        this->_m[0][0] * vector.get_x() + this->_m[0][1] * vector.get_y() + this->_m[0][2] * vector.get_z(),
        this->_m[1][0] * vector.get_x() + this->_m[1][1] * vector.get_y() + this->_m[1][2] * vector.get_z(),
        this->_m[2][0] * vector.get_x() + this->_m[2][1] * vector.get_y() + this->_m[2][2] * vector.get_z()
    );
    this->set_error(ER_SUCCESS);
    return (result);
}

matrix3 matrix3::multiply(const matrix3 &other) const
{
    matrix3 result;

    result._m[0][0] = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0] + this->_m[0][2] * other._m[2][0];
    result._m[0][1] = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1] + this->_m[0][2] * other._m[2][1];
    result._m[0][2] = this->_m[0][0] * other._m[0][2] + this->_m[0][1] * other._m[1][2] + this->_m[0][2] * other._m[2][2];
    result._m[1][0] = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0] + this->_m[1][2] * other._m[2][0];
    result._m[1][1] = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1] + this->_m[1][2] * other._m[2][1];
    result._m[1][2] = this->_m[1][0] * other._m[0][2] + this->_m[1][1] * other._m[1][2] + this->_m[1][2] * other._m[2][2];
    result._m[2][0] = this->_m[2][0] * other._m[0][0] + this->_m[2][1] * other._m[1][0] + this->_m[2][2] * other._m[2][0];
    result._m[2][1] = this->_m[2][0] * other._m[0][1] + this->_m[2][1] * other._m[1][1] + this->_m[2][2] * other._m[2][1];
    result._m[2][2] = this->_m[2][0] * other._m[0][2] + this->_m[2][1] * other._m[1][2] + this->_m[2][2] * other._m[2][2];
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

matrix3 matrix3::invert() const
{
    double determinant;
    matrix3 result;
    double inv_det;

    determinant = this->_m[0][0] * (this->_m[1][1] * this->_m[2][2] - this->_m[1][2] * this->_m[2][1])
        - this->_m[0][1] * (this->_m[1][0] * this->_m[2][2] - this->_m[1][2] * this->_m[2][0])
        + this->_m[0][2] * (this->_m[1][0] * this->_m[2][1] - this->_m[1][1] * this->_m[2][0]);
    if (math_absdiff(determinant, 0.0) <= 0.000001)
    {
        result.set_error(FT_EINVAL);
        return (result);
    }
    inv_det = 1.0 / determinant;
    result._m[0][0] = (this->_m[1][1] * this->_m[2][2] - this->_m[1][2] * this->_m[2][1]) * inv_det;
    result._m[0][1] = (this->_m[0][2] * this->_m[2][1] - this->_m[0][1] * this->_m[2][2]) * inv_det;
    result._m[0][2] = (this->_m[0][1] * this->_m[1][2] - this->_m[0][2] * this->_m[1][1]) * inv_det;
    result._m[1][0] = (this->_m[1][2] * this->_m[2][0] - this->_m[1][0] * this->_m[2][2]) * inv_det;
    result._m[1][1] = (this->_m[0][0] * this->_m[2][2] - this->_m[0][2] * this->_m[2][0]) * inv_det;
    result._m[1][2] = (this->_m[0][2] * this->_m[1][0] - this->_m[0][0] * this->_m[1][2]) * inv_det;
    result._m[2][0] = (this->_m[1][0] * this->_m[2][1] - this->_m[1][1] * this->_m[2][0]) * inv_det;
    result._m[2][1] = (this->_m[0][1] * this->_m[2][0] - this->_m[0][0] * this->_m[2][1]) * inv_det;
    result._m[2][2] = (this->_m[0][0] * this->_m[1][1] - this->_m[0][1] * this->_m[1][0]) * inv_det;
    result.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (result);
}

void    matrix3::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     matrix3::get_error() const
{
    return (this->_error_code);
}

const char  *matrix3::get_error_str() const
{
    return (ft_strerror(this->_error_code));
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
            temp[row][column] /= pivot;
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
                    temp[other][column] -= factor * temp[row][column];
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

