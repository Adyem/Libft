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

matrix2::matrix2()
{
    int row;
    int column;

    row = 0;
    while (row < 2)
    {
        column = 0;
        while (column < 2)
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

matrix2::matrix2(double m00, double m01,
        double m10, double m11)
{
    this->_m[0][0] = m00;
    this->_m[0][1] = m01;
    this->_m[1][0] = m10;
    this->_m[1][1] = m11;
    this->set_error(ER_SUCCESS);
    return ;
}

matrix2::~matrix2()
{
    return ;
}

matrix3::matrix3()
{
    int row;
    int column;

    row = 0;
    while (row < 3)
    {
        column = 0;
        while (column < 3)
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

matrix3::matrix3(double m00, double m01, double m02,
        double m10, double m11, double m12,
        double m20, double m21, double m22)
{
    this->_m[0][0] = m00;
    this->_m[0][1] = m01;
    this->_m[0][2] = m02;
    this->_m[1][0] = m10;
    this->_m[1][1] = m11;
    this->_m[1][2] = m12;
    this->_m[2][0] = m20;
    this->_m[2][1] = m21;
    this->_m[2][2] = m22;
    this->set_error(ER_SUCCESS);
    return ;
}

matrix3::~matrix3()
{
    return ;
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
