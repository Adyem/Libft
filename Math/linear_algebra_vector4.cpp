#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <new>

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void vector4_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int vector4::lock_mutex() const noexcept
{
    this->abort_if_not_initialized("vector4::lock_mutex");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int vector4::unlock_mutex() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

#if defined(__SSE2__)
static double vector4_compute_dot(double first_x, double first_y,
    double first_z, double first_w,
    double second_x, double second_y,
    double second_z, double second_w)
{
    __m128d first_xy;
    __m128d second_xy;
    __m128d product_xy;
    __m128d first_zw;
    __m128d second_zw;
    __m128d product_zw;
    __m128d sum;
    __m128d swapped;
    __m128d total;

    first_xy = _mm_set_pd(first_y, first_x);
    second_xy = _mm_set_pd(second_y, second_x);
    product_xy = _mm_mul_pd(first_xy, second_xy);
    first_zw = _mm_set_pd(first_w, first_z);
    second_zw = _mm_set_pd(second_w, second_z);
    product_zw = _mm_mul_pd(first_zw, second_zw);
    sum = _mm_add_pd(product_xy, product_zw);
    swapped = _mm_shuffle_pd(sum, sum, 0x1);
    total = _mm_add_sd(sum, swapped);
    return (_mm_cvtsd_f64(total));
}
#else
static double vector4_compute_dot(double first_x, double first_y,
    double first_z, double first_w,
    double second_x, double second_y,
    double second_z, double second_w)
{
    return (first_x * second_x + first_y * second_y
        + first_z * second_z + first_w * second_w);
}
#endif

int vector4::lock_pair(const vector4 &first, const vector4 &second,
    const vector4 *&lower, const vector4 *&upper)
{
    const vector4 *ordered_first;
    const vector4 *ordered_second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    ordered_first = &first;
    ordered_second = &second;
    if (ordered_first > ordered_second)
    {
        const vector4 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = lower->lock_mutex();
        if (lower_error != FT_ERR_SUCCESS)
        {
            return (lower_error);
        }
        int upper_error = upper->lock_mutex();
        if (upper_error == FT_ERR_SUCCESS)
        {
            return (FT_ERR_SUCCESS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            return (upper_error);
        }
        lower->unlock_mutex();
        vector4_sleep_backoff();
    }
}

void vector4::unlock_pair(const vector4 *lower, const vector4 *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

double vector4::get_x() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

double vector4::get_y() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

double vector4::get_z() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

double vector4::get_w() const
{
    int lock_error;
    double value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_w;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

vector4 vector4::add(const vector4 &other) const
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;
    double result_x;
    double result_y;
    double result_z;
    double result_w;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector4());
    }
    result_x = this->_x + other._x;
    result_y = this->_y + other._y;
    result_z = this->_z + other._z;
    result_w = this->_w + other._w;
    this->unlock_pair(lower, upper);
    return (vector4(result_x, result_y, result_z, result_w));
}

vector4 vector4::subtract(const vector4 &other) const
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;
    double result_x;
    double result_y;
    double result_z;
    double result_w;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector4());
    }
    result_x = this->_x - other._x;
    result_y = this->_y - other._y;
    result_z = this->_z - other._z;
    result_w = this->_w - other._w;
    this->unlock_pair(lower, upper);
    return (vector4(result_x, result_y, result_z, result_w));
}

double vector4::dot(const vector4 &other) const
{
    const vector4 *lower;
    const vector4 *upper;
    int lock_error;
    double result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    result = vector4_compute_dot(this->_x, this->_y, this->_z, this->_w,
            other._x, other._y, other._z, other._w);
    this->unlock_pair(lower, upper);
    return (result);
}

double vector4::length() const
{
    int lock_error;
    double squared;
    double result_value;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    squared = vector4_compute_dot(this->_x, this->_y, this->_z, this->_w,
            this->_x, this->_y, this->_z, this->_w);
    result_value = math_sqrt(squared);
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (result_value);
    }
    return (result_value);
}

vector4 vector4::normalize() const
{
    int lock_error;
    double squared_length;
    double length_value;
    double epsilon;
    int unlock_error;
    double normalized_x;
    double normalized_y;
    double normalized_z;
    double normalized_w;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector4());
    }
    squared_length = this->_x * this->_x + this->_y * this->_y
        + this->_z * this->_z + this->_w * this->_w;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
        {
            return (vector4());
        }
        return (vector4());
    }
    normalized_x = this->_x / length_value;
    normalized_y = this->_y / length_value;
    normalized_z = this->_z / length_value;
    normalized_w = this->_w / length_value;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (vector4());
    }
    return (vector4(normalized_x, normalized_y, normalized_z, normalized_w));
}

int vector4::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("vector4::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
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

int vector4::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("vector4::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        if (mutex_error != FT_ERR_SUCCESS)
            return (mutex_error);
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return (FT_ERR_SUCCESS);
}

bool vector4::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("vector4::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *vector4::get_mutex_for_testing() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_recursive_mutex();
        if (mutex_pointer == ft_nullptr)
            return (ft_nullptr);
        mutex_error = mutex_pointer->initialize();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (ft_nullptr);
        }
        this->_mutex = mutex_pointer;
    }
    return (this->_mutex);
}
#endif
