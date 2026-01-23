#include "linear_algebra_quaternion.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread.hpp"
static int math_quaternion_lock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static int math_quaternion_unlock_mutex(const pt_recursive_mutex &mutex)
{
    int error;

    error = mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static void quaternion_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static pt_recursive_mutex *quaternion_mutex_for(const quaternion &value)
{
    return (value.get_mutex_for_validation());
}

static void quaternion_unlock_pair(const quaternion *lower, const quaternion *upper)
{
    if (upper != ft_nullptr)
        math_quaternion_unlock_mutex(*quaternion_mutex_for(*upper));
    if (lower != ft_nullptr && lower != upper)
        math_quaternion_unlock_mutex(*quaternion_mutex_for(*lower));
    return ;
}

static int quaternion_lock_pair(const quaternion &first, const quaternion &second,
        const quaternion *&lower, const quaternion *&upper)
{
    const quaternion *ordered_first = &first;
    const quaternion *ordered_second = &second;
    pt_recursive_mutex *ordered_first_mutex = quaternion_mutex_for(first);
    pt_recursive_mutex *ordered_second_mutex = quaternion_mutex_for(second);

    if (ordered_first == ordered_second)
    {
        lower = &first;
        upper = &first;
        return (math_quaternion_lock_mutex(*ordered_first_mutex));
    }
    if (ordered_first > ordered_second)
    {
        const quaternion *temporary = ordered_first;
        pt_recursive_mutex *temporary_mutex = ordered_first_mutex;

        ordered_first = ordered_second;
        ordered_second = temporary;
        ordered_first_mutex = ordered_second_mutex;
        ordered_second_mutex = temporary_mutex;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = math_quaternion_lock_mutex(*ordered_first_mutex);
        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = math_quaternion_lock_mutex(*ordered_second_mutex);
        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            math_quaternion_unlock_mutex(*ordered_first_mutex);
            return (upper_error);
        }
        math_quaternion_unlock_mutex(*ordered_first_mutex);
        quaternion_sleep_backoff();
    }
}

void quaternion::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors,
            error_code, operation_id);
    return ;
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
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

quaternion::quaternion(double w, double x, double y, double z)
{
    this->_w = w;
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

quaternion::quaternion(const quaternion &other)
{
    pt_recursive_mutex *other_mutex = other.get_mutex_for_validation();
    int lock_error;

    lock_error = math_quaternion_lock_mutex(*other_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    math_quaternion_unlock_mutex(*other_mutex);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

quaternion &quaternion::operator=(const quaternion &other)
{
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = quaternion_lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    quaternion_unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

quaternion::quaternion(quaternion &&other) noexcept
{
    pt_recursive_mutex *other_mutex = other.get_mutex_for_validation();
    int lock_error;

    lock_error = math_quaternion_lock_mutex(*other_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
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
    math_quaternion_unlock_mutex(*other_mutex);
    other.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

quaternion &quaternion::operator=(quaternion &&other) noexcept
{
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = quaternion_lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
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
    quaternion_unlock_pair(lower, upper);
    this->record_operation_error(FT_ERR_SUCCESSS);
    other.record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

quaternion::~quaternion()
{
    return ;
}

double  quaternion::get_w() const
{
    double value;
    int lock_error;
    int unlock_error;
    pt_recursive_mutex *self_mutex = this->get_mutex_for_validation();

    lock_error = math_quaternion_lock_mutex(*self_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_w;
    unlock_error = math_quaternion_unlock_mutex(*self_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  quaternion::get_x() const
{
    double value;
    int lock_error;
    int unlock_error;
    pt_recursive_mutex *self_mutex = this->get_mutex_for_validation();

    lock_error = math_quaternion_lock_mutex(*self_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_x;
    unlock_error = math_quaternion_unlock_mutex(*self_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  quaternion::get_y() const
{
    double value;
    int lock_error;
    int unlock_error;
    pt_recursive_mutex *self_mutex = this->get_mutex_for_validation();

    lock_error = math_quaternion_lock_mutex(*self_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_y;
    unlock_error = math_quaternion_unlock_mutex(*self_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

double  quaternion::get_z() const
{
    double value;
    int lock_error;
    int unlock_error;
    pt_recursive_mutex *self_mutex = this->get_mutex_for_validation();

    lock_error = math_quaternion_lock_mutex(*self_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    value = this->_z;
    unlock_error = math_quaternion_unlock_mutex(*self_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (value);
}

quaternion  quaternion::add(const quaternion &other) const
{
    quaternion result;
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    lock_error = quaternion_lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    result._w = this->_w + other._w;
    result._x = this->_x + other._x;
    result._y = this->_y + other._y;
    result._z = this->_z + other._z;
    quaternion_unlock_pair(lower, upper);
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

quaternion  quaternion::multiply(const quaternion &other) const
{
    quaternion result;
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;

    lock_error = quaternion_lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    quaternion_compute_product_components(result,
        this->_w, this->_x, this->_y, this->_z,
        other._w, other._x, other._y, other._z);
    quaternion_unlock_pair(lower, upper);
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

quaternion  quaternion::conjugate() const
{
    quaternion result;
    int lock_error;
    int unlock_error;

    pt_recursive_mutex *self_mutex = this->get_mutex_for_validation();
    lock_error = math_quaternion_lock_mutex(*self_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    result._w = this->_w;
    result._x = -this->_x;
    result._y = -this->_y;
    result._z = -this->_z;
    unlock_error = math_quaternion_unlock_mutex(*self_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(unlock_error);
        this->record_operation_error(unlock_error);
        return (result);
    }
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

double  quaternion::length() const
{
    double length_value;
    int lock_error;
    int unlock_error;

    pt_recursive_mutex *self_mutex = this->get_mutex_for_validation();
    lock_error = math_quaternion_lock_mutex(*self_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0.0);
    }
    length_value = quaternion_compute_dot(this->_w, this->_x,
        this->_y, this->_z,
        this->_w, this->_x, this->_y, this->_z);
    unlock_error = math_quaternion_unlock_mutex(*self_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(unlock_error);
        return (math_sqrt(length_value));
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
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
    pt_recursive_mutex *self_mutex = this->get_mutex_for_validation();
    int lock_error;
    int unlock_error;

    lock_error = math_quaternion_lock_mutex(*self_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(lock_error);
        this->record_operation_error(lock_error);
        return (result);
    }
    local_w = this->_w;
    local_x = this->_x;
    local_y = this->_y;
    local_z = this->_z;
    unlock_error = math_quaternion_unlock_mutex(*self_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        result.record_operation_error(unlock_error);
        this->record_operation_error(unlock_error);
        return (result);
    }
    length_value = math_sqrt(local_w * local_w + local_x * local_x
        + local_y * local_y + local_z * local_z);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        result.record_operation_error(FT_ERR_INVALID_ARGUMENT);
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._w = local_w / length_value;
    result._x = local_x / length_value;
    result._y = local_y / length_value;
    result._z = local_z / length_value;
    result.record_operation_error(FT_ERR_SUCCESSS);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

ft_operation_error_stack *quaternion::get_operation_error_stack_for_validation() noexcept
{
    return (&this->_operation_errors);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *quaternion::get_mutex_for_testing() noexcept
{
    return (this->get_mutex_for_validation());
}
#endif

int quaternion::last_operation_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

const char *quaternion::last_operation_error_str() const noexcept
{
    int error_code;
    const char *error_string;

    error_code = ft_operation_error_stack_last_error(&this->_operation_errors);
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

int quaternion::operation_error_at(ft_size_t index) const noexcept
{
    return (ft_operation_error_stack_error_at(&this->_operation_errors, index));
}

const char *quaternion::operation_error_str_at(ft_size_t index) const noexcept
{
    int error_code;
    const char *error_string;

    error_code = ft_operation_error_stack_error_at(&this->_operation_errors, index);
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

void quaternion::pop_operation_errors() noexcept
{
    ft_operation_error_stack_pop_all(&this->_operation_errors);
    ft_global_error_stack_pop_all();
    return ;
}

int quaternion::pop_oldest_operation_error() noexcept
{
    ft_global_error_stack_pop_last();
    return (ft_operation_error_stack_pop_last(&this->_operation_errors));
}

int quaternion::pop_newest_operation_error() noexcept
{
    ft_global_error_stack_pop_newest();
    return (ft_operation_error_stack_pop_newest(&this->_operation_errors));
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
