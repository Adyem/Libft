#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <new>

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void vector3_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int vector3::lock_mutex() const noexcept
{
    this->abort_if_not_initialized("vector3::lock_mutex");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int vector3::unlock_mutex() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

#if defined(__SSE2__)
static double vector3_compute_dot(double first_x, double first_y, double first_z,
    double second_x, double second_y, double second_z)
{
    __m128d first_xy;
    __m128d second_xy;
    __m128d product_xy;
    __m128d swapped;
    __m128d sum_xy;
    __m128d first_z_vector;
    __m128d second_z_vector;
    __m128d product_z;
    __m128d total;

    first_xy = _mm_set_pd(first_y, first_x);
    second_xy = _mm_set_pd(second_y, second_x);
    product_xy = _mm_mul_pd(first_xy, second_xy);
    swapped = _mm_shuffle_pd(product_xy, product_xy, 0x1);
    sum_xy = _mm_add_sd(product_xy, swapped);
    first_z_vector = _mm_set_sd(first_z);
    second_z_vector = _mm_set_sd(second_z);
    product_z = _mm_mul_sd(first_z_vector, second_z_vector);
    total = _mm_add_sd(sum_xy, product_z);
    return (_mm_cvtsd_f64(total));
}
#else
static double vector3_compute_dot(double first_x, double first_y, double first_z,
    double second_x, double second_y, double second_z)
{
    return (first_x * second_x + first_y * second_y + first_z * second_z);
}
#endif

int vector3::lock_pair(const vector3 &first, const vector3 &second,
    const vector3 *&lower, const vector3 *&upper)
{
    const vector3 *ordered_first;
    const vector3 *ordered_second;

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
        const vector3 *temporary;

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
        vector3_sleep_backoff();
    }
}

void vector3::unlock_pair(const vector3 *lower, const vector3 *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

double vector3::get_x() const
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

double vector3::get_y() const
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

double vector3::get_z() const
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

vector3 vector3::add(const vector3 &other) const
{
    const vector3 *lower;
    const vector3 *upper;
    int lock_error;
    double result_x;
    double result_y;
    double result_z;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector3());
    }
    result_x = this->_x + other._x;
    result_y = this->_y + other._y;
    result_z = this->_z + other._z;
    this->unlock_pair(lower, upper);
    return (vector3(result_x, result_y, result_z));
}

vector3 vector3::subtract(const vector3 &other) const
{
    const vector3 *lower;
    const vector3 *upper;
    int lock_error;
    double result_x;
    double result_y;
    double result_z;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector3());
    }
    result_x = this->_x - other._x;
    result_y = this->_y - other._y;
    result_z = this->_z - other._z;
    this->unlock_pair(lower, upper);
    return (vector3(result_x, result_y, result_z));
}

double vector3::dot(const vector3 &other) const
{
    const vector3 *lower;
    const vector3 *upper;
    int lock_error;
    double result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    result = vector3_compute_dot(this->_x, this->_y, this->_z,
            other._x, other._y, other._z);
    this->unlock_pair(lower, upper);
    return (result);
}

vector3 vector3::cross(const vector3 &other) const
{
    const vector3 *lower;
    const vector3 *upper;
    int lock_error;
    double result_x;
    double result_y;
    double result_z;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector3());
    }
    result_x = this->_y * other._z - this->_z * other._y;
    result_y = this->_z * other._x - this->_x * other._z;
    result_z = this->_x * other._y - this->_y * other._x;
    this->unlock_pair(lower, upper);
    return (vector3(result_x, result_y, result_z));
}

double vector3::length() const
{
    int lock_error;
    double squared;
    double result;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    squared = vector3_compute_dot(this->_x, this->_y, this->_z,
            this->_x, this->_y, this->_z);
    result = math_sqrt(squared);
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (result);
    }
    return (result);
}

vector3 vector3::normalize() const
{
    int lock_error;
    double squared_length;
    double length_value;
    double epsilon;
    int unlock_error;
    double normalized_x;
    double normalized_y;
    double normalized_z;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector3());
    }
    squared_length = this->_x * this->_x + this->_y * this->_y + this->_z * this->_z;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
        {
            return (vector3());
        }
        return (vector3());
    }
    normalized_x = this->_x / length_value;
    normalized_y = this->_y / length_value;
    normalized_z = this->_z / length_value;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (vector3());
    }
    return (vector3(normalized_x, normalized_y, normalized_z));
}

int vector3::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("vector3::enable_thread_safety");
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

void vector3::disable_thread_safety() noexcept
{
    this->abort_if_not_initialized("vector3::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}

bool vector3::is_thread_safe_enabled() const noexcept
{
    this->abort_if_not_initialized("vector3::is_thread_safe_enabled");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *vector3::get_mutex_for_testing() noexcept
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
