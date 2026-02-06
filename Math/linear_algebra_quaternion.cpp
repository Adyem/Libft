#include "linear_algebra_quaternion.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
int quaternion::lock_mutex() const noexcept
{
    return (pt_recursive_mutex_lock_if_enabled(
        this->_mutex,
        this->_mutex != ft_nullptr
    ));
}

int quaternion::unlock_mutex() const noexcept
{
    return (pt_recursive_mutex_unlock_if_enabled(
        this->_mutex,
        this->_mutex != ft_nullptr
    ));
}

static void quaternion_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

void quaternion::unlock_pair(const quaternion *lower, const quaternion *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

int quaternion::lock_pair(const quaternion &first, const quaternion &second,
        const quaternion *&lower, const quaternion *&upper)
{
    const quaternion *ordered_first = &first;
    const quaternion *ordered_second = &second;

    if (ordered_first == ordered_second)
    {
        lower = &first;
        upper = &first;
        return (ordered_first->lock_mutex());
    }
    if (ordered_first > ordered_second)
    {
        const quaternion *temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = ordered_first->lock_mutex();
        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = ordered_second->lock_mutex();
        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_mutex();
            return (upper_error);
        }
        ordered_first->unlock_mutex();
        quaternion_sleep_backoff();
    }
}

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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

quaternion::quaternion(double w, double x, double y, double z)
{
    this->_w = w;
    this->_x = x;
    this->_y = y;
    this->_z = z;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

quaternion::quaternion(const quaternion &other)
{
    int lock_error;

    lock_error = other.lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other.unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

quaternion &quaternion::operator=(const quaternion &other)
{
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    quaternion::unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

quaternion::quaternion(quaternion &&other) noexcept
{
    int lock_error;

    lock_error = other.lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other._w = 1.0;
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    other.unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

quaternion &quaternion::operator=(quaternion &&other) noexcept
{
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other._w = 1.0;
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    quaternion::unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

quaternion::~quaternion()
{
    this->disable_thread_safety();
    return ;
}

double  quaternion::get_w() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    value = this->_w;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  quaternion::get_x() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    value = this->_x;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  quaternion::get_y() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    value = this->_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

double  quaternion::get_z() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    value = this->_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value);
}

quaternion  quaternion::add(const quaternion &other) const
{
    quaternion result;
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    result._w = this->_w + other._w;
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    quaternion::unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

quaternion  quaternion::multiply(const quaternion &other) const
{
    quaternion result;
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    quaternion_compute_product_components(result,
        this->_w, this->_x, this->_y, this->_z,
        other._w, other._x, other._y, other._z);
    quaternion::unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

quaternion  quaternion::conjugate() const
{
    quaternion result;
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    result._w = this->_w;
    result._x = -this->_x;
    result._y = -this->_y;
    result._z = -this->_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

double  quaternion::length() const
{
    double length_value;
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0.0);
    }
    length_value = quaternion_compute_dot(this->_w, this->_x,
        this->_y, this->_z,
        this->_w, this->_x, this->_y, this->_z);
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (math_sqrt(length_value));
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
    int lock_error;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    local_w = this->_w;
    local_x = this->_x;
    local_y = this->_y;
    local_z = this->_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    length_value = math_sqrt(local_w * local_w + local_x * local_x
        + local_y * local_y + local_z * local_z);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._w = local_w / length_value;
    result._x = local_x / length_value;
    result._y = local_y / length_value;
    result._z = local_z / length_value;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *quaternion::get_mutex_for_testing() noexcept
{
    if (this->_mutex == ft_nullptr)
        this->prepare_thread_safety();
    return (this->_mutex);
}
#endif

int quaternion::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void quaternion::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool quaternion::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int quaternion::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void quaternion::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
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
