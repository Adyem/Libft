#include "linear_algebra_quaternion.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

static void quaternion_print_lifecycle_error(const char *method_name,
    const char *reason)
{
    const char *resolved_method_name;
    const char *resolved_reason;

    resolved_method_name = method_name;
    resolved_reason = reason;
    if (resolved_method_name == ft_nullptr)
        resolved_method_name = "unknown";
    if (resolved_reason == ft_nullptr)
        resolved_reason = "unknown";
    pf_printf_fd(2, "quaternion lifecycle error: %s: %s\n",
        resolved_method_name, resolved_reason);
    su_abort();
    return ;
}

void quaternion::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    quaternion_print_lifecycle_error(method_name, reason);
    return ;
}

void quaternion::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == quaternion::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}
static void quaternion_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

void quaternion::unlock_pair(const quaternion *lower, const quaternion *upper)
{
    if (upper != ft_nullptr)
        pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
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
        return (pt_recursive_mutex_lock_if_not_null(ordered_first->_mutex));
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
        int lower_error = pt_recursive_mutex_lock_if_not_null(ordered_first->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        int upper_error = pt_recursive_mutex_lock_if_not_null(ordered_second->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
            return (upper_error);
        }
        pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
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
static void quaternion_compute_product_components(double &result_w,
    double &result_x, double &result_y, double &result_z,
    double first_w, double first_x, double first_y, double first_z,
    double second_w, double second_x, double second_y, double second_z)
{
    result_w = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_w, -second_x, -second_y, -second_z);
    result_x = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_x, second_w, second_z, -second_y);
    result_y = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_y, -second_z, second_w, second_x);
    result_z = quaternion_compute_dot(first_w, first_x, first_y, first_z,
            second_z, second_y, -second_x, second_w);
    return ;
}

quaternion::quaternion()
{
    this->_w = 1.0;
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_initialized_state = quaternion::_state_uninitialized;
    return ;
}

quaternion::quaternion(double w, double x, double y, double z)
{
    int initialization_error;

    this->_w = 1.0;
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_initialized_state = quaternion::_state_uninitialized;
    initialization_error = this->initialize(w, x, y, z);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialized_state == quaternion::_state_uninitialized)
        this->_initialized_state = quaternion::_state_destroyed;
    return ;
}

int quaternion::initialize() noexcept
{
    if (this->_initialized_state == quaternion::_state_initialized)
    {
        this->abort_lifecycle_error("quaternion::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_w = 1.0;
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_initialized_state = quaternion::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int quaternion::initialize(double w, double x, double y, double z) noexcept
{
    int initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    this->_w = w;
    this->_x = x;
    this->_y = y;
    this->_z = z;
    return (FT_ERR_SUCCESS);
}

int quaternion::initialize(const quaternion &other) noexcept
{
    int initialization_error;
    int lock_error;
    int unlock_error;

    if (other._initialized_state != quaternion::_state_initialized)
    {
        if (other._initialized_state == quaternion::_state_uninitialized)
            other.abort_lifecycle_error("quaternion::initialize(const quaternion &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("quaternion::initialize(const quaternion &) source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int quaternion::move(quaternion &other) noexcept
{
    const quaternion *lower;
    const quaternion *upper;
    int initialize_error;
    int lock_error;

    if (other._initialized_state != quaternion::_state_initialized)
    {
        if (other._initialized_state == quaternion::_state_uninitialized)
            other.abort_lifecycle_error("quaternion::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("quaternion::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != quaternion::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_w = other._w;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other._w = 1.0;
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    quaternion::unlock_pair(lower, upper);
    return (FT_ERR_SUCCESS);
}

int quaternion::initialize(quaternion &&other) noexcept
{
    int initialization_error;
    int move_error;

    if (other._initialized_state != quaternion::_state_initialized)
    {
        if (other._initialized_state == quaternion::_state_uninitialized)
            other.abort_lifecycle_error("quaternion::initialize(quaternion &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("quaternion::initialize(quaternion &&) source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

int quaternion::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != quaternion::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_w = 1.0;
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = quaternion::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

quaternion::~quaternion()
{
    if (this->_initialized_state == quaternion::_state_initialized)
        (void)this->destroy();
    return ;
}

double  quaternion::get_w() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_w;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

double  quaternion::get_x() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_x;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

double  quaternion::get_y() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_y;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

double  quaternion::get_z() const
{
    double value;
    int lock_error;
    int unlock_error;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    value = this->_z;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (value);
    }
    return (value);
}

quaternion  quaternion::add(const quaternion &other) const
{
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;
    double result_w;
    double result_x;
    double result_y;
    double result_z;

    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (quaternion());
    }
    result_w = this->_w + other._w;
    result_x = this->_x + other._x;
    result_y = this->_y + other._y;
    result_z = this->_z + other._z;
    quaternion::unlock_pair(lower, upper);
    return (quaternion(result_w, result_x, result_y, result_z));
}

quaternion  quaternion::multiply(const quaternion &other) const
{
    const quaternion *lower;
    const quaternion *upper;
    int lock_error;
    double result_w;
    double result_x;
    double result_y;
    double result_z;

    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (quaternion());
    }
    quaternion_compute_product_components(result_w, result_x, result_y, result_z,
        this->_w, this->_x, this->_y, this->_z,
        other._w, other._x, other._y, other._z);
    quaternion::unlock_pair(lower, upper);
    return (quaternion(result_w, result_x, result_y, result_z));
}

quaternion  quaternion::conjugate() const
{
    int lock_error;
    int unlock_error;
    double result_w;
    double result_x;
    double result_y;
    double result_z;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (quaternion());
    }
    result_w = this->_w;
    result_x = -this->_x;
    result_y = -this->_y;
    result_z = -this->_z;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (quaternion());
    }
    return (quaternion(result_w, result_x, result_y, result_z));
}

double  quaternion::length() const
{
    double length_value;
    int lock_error;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0.0);
    }
    length_value = quaternion_compute_dot(this->_w, this->_x,
        this->_y, this->_z,
        this->_w, this->_x, this->_y, this->_z);
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (math_sqrt(length_value));
    }
    return (math_sqrt(length_value));
}

quaternion  quaternion::normalize() const
{
    double length_value;
    double epsilon;
    double local_w;
    double local_x;
    double local_y;
    double local_z;
    int lock_error;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (quaternion());
    }
    local_w = this->_w;
    local_x = this->_x;
    local_y = this->_y;
    local_z = this->_z;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (quaternion());
    }
    length_value = math_sqrt(local_w * local_w + local_x * local_x
        + local_y * local_y + local_z * local_z);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        return (quaternion());
    }
    return (quaternion(local_w / length_value, local_x / length_value,
            local_y / length_value, local_z / length_value));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *quaternion::get_mutex_for_testing() noexcept
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

int quaternion::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("quaternion::enable_thread_safety");
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

int quaternion::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("quaternion::disable_thread_safety");
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

bool quaternion::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("quaternion::is_thread_safe");
    return (this->_mutex != ft_nullptr);
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
