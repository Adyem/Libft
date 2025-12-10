#include "linear_algebra_quaternion.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static double quaternion_compute_dot(double first_w, double first_x,
    double first_y, double first_z,
    double second_w, double second_x,
    double second_y, double second_z);

static void quaternion_compute_product_components(quaternion &result,
    double first_w, double first_x, double first_y, double first_z,
    double second_w, double second_x, double second_y, double second_z)
{
    double w_value;
    double x_value;
    double y_value;
    double z_value;
    w_value = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_w, -second_x, -second_y, -second_z);
    x_value = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_x, second_w, second_z, -second_y);
    y_value = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_y, -second_z, second_w, second_x);
    z_value = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_z, second_y, -second_x, second_w);
    quaternion composed_values_temp(w_value, x_value, y_value, z_value);
    result = composed_values_temp;
    return ;
}

quaternion::quaternion()
{
    this->_w = 1.0;
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_error_code = FT_ER_SUCCESSS;
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

quaternion::quaternion(double w, double x, double y, double z)
{
    this->_w = w;
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->_error_code = FT_ER_SUCCESSS;
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

quaternion::quaternion(const quaternion &other)
{
    std::lock_guard<std::mutex> other_lock(other._mutex);

    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

quaternion &quaternion::operator=(const quaternion &other)
{
    if (this == &other)
        return (*this);
    std::lock(this->_mutex, other._mutex);
    std::lock_guard<std::mutex> this_lock(this->_mutex, std::adopt_lock);
    std::lock_guard<std::mutex> other_lock(other._mutex, std::adopt_lock);
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    ft_errno = FT_ER_SUCCESSS;
    return (*this);
}

quaternion::quaternion(quaternion &&other) noexcept
{
    std::lock_guard<std::mutex> other_lock(other._mutex);

    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    other._w = 1.0;
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

quaternion &quaternion::operator=(quaternion &&other) noexcept
{
    if (this == &other)
        return (*this);
    std::lock(this->_mutex, other._mutex);
    std::lock_guard<std::mutex> this_lock(this->_mutex, std::adopt_lock);
    std::lock_guard<std::mutex> other_lock(other._mutex, std::adopt_lock);
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    other._w = 1.0;
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    ft_errno = FT_ER_SUCCESSS;
    return (*this);
}

quaternion::~quaternion()
{
    return ;
}

double  quaternion::get_w() const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);
    double value;

    value = this->_w;
    return (value);
}

double  quaternion::get_x() const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);
    double value;

    value = this->_x;
    return (value);
}

double  quaternion::get_y() const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);
    double value;

    value = this->_y;
    return (value);
}

double  quaternion::get_z() const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);
    double value;

    value = this->_z;
    return (value);
}

quaternion  quaternion::add(const quaternion &other) const
{
    quaternion result;

    if (&other == this)
    {
        std::lock_guard<std::mutex> this_lock(this->_mutex);

        result._w = this->_w + this->_w;
        result._x = this->_x + this->_x;
        result._y = this->_y + this->_y;
        result._z = this->_z + this->_z;
    }
    else
    {
        std::lock(this->_mutex, other._mutex);
        std::lock_guard<std::mutex> this_lock(this->_mutex, std::adopt_lock);
        std::lock_guard<std::mutex> other_lock(other._mutex, std::adopt_lock);

        result._w = this->_w + other._w;
        result._x = this->_x + other._x;
        result._y = this->_y + other._y;
        result._z = this->_z + other._z;
    }
    result.set_error(FT_ER_SUCCESSS);
    this->set_error(FT_ER_SUCCESSS);
    return (result);
}

quaternion  quaternion::multiply(const quaternion &other) const
{
    quaternion result;

    if (&other == this)
    {
        std::lock_guard<std::mutex> this_lock(this->_mutex);

        quaternion_compute_product_components(result,
            this->_w, this->_x, this->_y, this->_z,
            this->_w, this->_x, this->_y, this->_z);
    }
    else
    {
        std::lock(this->_mutex, other._mutex);
        std::lock_guard<std::mutex> this_lock(this->_mutex, std::adopt_lock);
        std::lock_guard<std::mutex> other_lock(other._mutex, std::adopt_lock);

        quaternion_compute_product_components(result,
            this->_w, this->_x, this->_y, this->_z,
            other._w, other._x, other._y, other._z);
    }
    result.set_error(FT_ER_SUCCESSS);
    this->set_error(FT_ER_SUCCESSS);
    return (result);
}

quaternion  quaternion::conjugate() const
{
    quaternion result;

    {
        std::lock_guard<std::mutex> this_lock(this->_mutex);

        result._w = this->_w;
        result._x = -this->_x;
        result._y = -this->_y;
        result._z = -this->_z;
    }
    result.set_error(FT_ER_SUCCESSS);
    this->set_error(FT_ER_SUCCESSS);
    return (result);
}

double  quaternion::length() const
{
    double length_value;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        length_value = quaternion_compute_dot(this->_w, this->_x,
            this->_y, this->_z,
            this->_w, this->_x, this->_y, this->_z);
    }
    this->set_error(FT_ER_SUCCESSS);
    return (math_sqrt(length_value));
}

quaternion  quaternion::normalize() const
{
    quaternion result;
    double length_value;
    double epsilon;
    double local_w;
    double local_x;
    double local_y;
    double local_z;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        local_w = this->_w;
        local_x = this->_x;
        local_y = this->_y;
        local_z = this->_z;
    }
    length_value = math_sqrt(local_w * local_w + local_x * local_x
        + local_y * local_y + local_z * local_z);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        result.set_error(FT_ERR_INVALID_ARGUMENT);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._w = local_w / length_value;
    result._x = local_x / length_value;
    result._y = local_y / length_value;
    result._z = local_z / length_value;
    result.set_error(FT_ER_SUCCESSS);
    this->set_error(FT_ER_SUCCESSS);
    return (result);
}

void    quaternion::set_error(int error_code) const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);

    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     quaternion::get_error() const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);
    int error_code;

    error_code = this->_error_code;
    return (error_code);
}

const char  *quaternion::get_error_str() const
{
    int error_code;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        error_code = this->_error_code;
    }
    return (ft_strerror(error_code));
}

#if defined(__SSE2__)
static double quaternion_compute_dot(double first_w, double first_x,
    double first_y, double first_z,
    double second_w, double second_x,
    double second_y, double second_z)
{
    __m128d first_low;
    __m128d second_low;
    __m128d product_low;
    __m128d first_high;
    __m128d second_high;
    __m128d product_high;
    __m128d sum;
    __m128d swapped;
    __m128d total;

    first_low = _mm_set_pd(first_x, first_w);
    second_low = _mm_set_pd(second_x, second_w);
    product_low = _mm_mul_pd(first_low, second_low);
    first_high = _mm_set_pd(first_z, first_y);
    second_high = _mm_set_pd(second_z, second_y);
    product_high = _mm_mul_pd(first_high, second_high);
    sum = _mm_add_pd(product_low, product_high);
    swapped = _mm_shuffle_pd(sum, sum, 0x1);
    total = _mm_add_sd(sum, swapped);
    return (_mm_cvtsd_f64(total));
}
#else
static double quaternion_compute_dot(double first_w, double first_x,
    double first_y, double first_z,
    double second_w, double second_x,
    double second_y, double second_z)
{
    return (first_w * second_w + first_x * second_x
        + first_y * second_y + first_z * second_z);
}
#endif
