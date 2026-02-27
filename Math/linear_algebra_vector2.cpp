#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <new>

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void vector2_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int vector2::lock_mutex() const noexcept
{
    this->abort_if_not_initialized("vector2::lock_mutex");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int vector2::unlock_mutex() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

#if defined(__SSE2__)
static double vector2_compute_dot(double first_x, double first_y,
    double second_x, double second_y)
{
    __m128d first_vector;
    __m128d second_vector;
    __m128d product;
    __m128d swapped;
    __m128d sum;

    first_vector = _mm_set_pd(first_y, first_x);
    second_vector = _mm_set_pd(second_y, second_x);
    product = _mm_mul_pd(first_vector, second_vector);
    swapped = _mm_shuffle_pd(product, product, 0x1);
    sum = _mm_add_sd(product, swapped);
    return (_mm_cvtsd_f64(sum));
}
#else
static double vector2_compute_dot(double first_x, double first_y,
    double second_x, double second_y)
{
    return (first_x * second_x + first_y * second_y);
}
#endif

int vector2::lock_pair(const vector2 &first, const vector2 &second,
    const vector2 *&lower, const vector2 *&upper)
{
    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    lower = &first;
    upper = &second;
    if (lower > upper)
    {
        const vector2 *temporary;

        temporary = lower;
        lower = upper;
        upper = temporary;
    }
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
        vector2_sleep_backoff();
    }
}

void vector2::unlock_pair(const vector2 *lower, const vector2 *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

double vector2::get_x() const
{
    int lock_error;
    double value;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_x;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

double vector2::get_y() const
{
    int lock_error;
    double value;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_y;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

vector2 vector2::add(const vector2 &other) const
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;
    double result_x;
    double result_y;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector2());
    }
    result_x = this->_x + other._x;
    result_y = this->_y + other._y;
    this->unlock_pair(lower, upper);
    return (vector2(result_x, result_y));
}

vector2 vector2::subtract(const vector2 &other) const
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;
    double result_x;
    double result_y;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector2());
    }
    result_x = this->_x - other._x;
    result_y = this->_y - other._y;
    this->unlock_pair(lower, upper);
    return (vector2(result_x, result_y));
}

double vector2::dot(const vector2 &other) const
{
    const vector2 *lower;
    const vector2 *upper;
    int lock_error;
    double result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    result = vector2_compute_dot(this->_x, this->_y, other._x, other._y);
    this->unlock_pair(lower, upper);
    return (result);
}

double vector2::length() const
{
    int lock_error;
    int unlock_error;
    double squared;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    squared = vector2_compute_dot(this->_x, this->_y, this->_x, this->_y);
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (math_sqrt(squared));
    }
    return (math_sqrt(squared));
}

vector2 vector2::normalize() const
{
    int lock_error;
    int unlock_error;
    double squared_length;
    double length_value;
    double epsilon;
    double normalized_x;
    double normalized_y;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector2());
    }
    squared_length = this->_x * this->_x + this->_y * this->_y;
    length_value = math_sqrt(squared_length);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
        {
            return (vector2());
        }
        return (vector2());
    }
    normalized_x = this->_x / length_value;
    normalized_y = this->_y / length_value;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (vector2());
    }
    return (vector2(normalized_x, normalized_y));
}

int vector2::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("vector2::enable_thread_safety");
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

int vector2::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("vector2::disable_thread_safety");
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

bool vector2::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("vector2::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *vector2::get_mutex_for_testing() noexcept
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
