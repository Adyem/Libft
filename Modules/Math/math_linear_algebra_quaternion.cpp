#include "quaternion.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "math_interval.hpp"

thread_local int32_t quaternion::_last_error = FT_ERR_SUCCESS;

int32_t quaternion::set_error(int32_t error_code) noexcept
{
    quaternion::_last_error = error_code;
    return (error_code);
}

int32_t quaternion::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "quaternion::get_error",
            "called while object is uninitialised");
    return (quaternion::_last_error);
}

const char *quaternion::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "quaternion::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(quaternion::_last_error));
}

void quaternion::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void quaternion::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
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
        (void)pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        (void)pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    return ;
}

int32_t quaternion::lock_pair(const quaternion &first, const quaternion &second,
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
    while (FT_TRUE)
    {
        int32_t lower_error = pt_recursive_mutex_lock_if_not_null(ordered_first->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        int32_t upper_error = pt_recursive_mutex_lock_if_not_null(ordered_second->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
            return (upper_error);
        }
        (void)pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
        quaternion_sleep_backoff();
    }
}

#if defined(__SSE2__)
#include <immintrin.h>
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

quaternion::quaternion() noexcept
{
    this->_w_component = 1.0;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

quaternion::quaternion(double scalar_w, double x_component, double y_component, double z_component) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = quaternion::_last_error;
    this->_w_component = 1.0;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    initialization_error = this->initialize(scalar_w, x_component, y_component, z_component);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    quaternion::set_error(previous_last_error);
    return ;
}

int32_t quaternion::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("quaternion::initialize",
            "called while object is already initialised");
        return (quaternion::set_error(FT_ERR_INVALID_STATE));
    }
    this->_w_component = 1.0;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

int32_t quaternion::initialize(double scalar_w, double x_component, double y_component, double z_component) noexcept
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (quaternion::set_error(initialization_error));
    this->_w_component = scalar_w;
    this->_x_component = x_component;
    this->_y_component = y_component;
    this->_z_component = z_component;
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

int32_t quaternion::initialize(const quaternion &other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("quaternion::initialize(const quaternion &) source",
            "called with uninitialised source object");
        return (quaternion::set_error(FT_ERR_INVALID_STATE));
    }
    if (this == &other)
        return (quaternion::set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (quaternion::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_w_component = 1.0;
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (quaternion::set_error(FT_ERR_SUCCESS));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (quaternion::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (quaternion::set_error(initialization_error));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (quaternion::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_w_component = other._w_component;
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    this->_z_component = other._z_component;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

int32_t quaternion::move(quaternion &other) noexcept
{
    const quaternion *lower;
    const quaternion *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("quaternion::move source",
            "called with uninitialised source object");
        return (quaternion::set_error(FT_ERR_INVALID_STATE));
    }
    if (this == &other)
        return (quaternion::set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (quaternion::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_w_component = 1.0;
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (quaternion::set_error(FT_ERR_SUCCESS));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (quaternion::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (quaternion::set_error(initialize_error));
    }
    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_w_component = 1.0;
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (quaternion::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_w_component = other._w_component;
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    this->_z_component = other._z_component;
    other._w_component = 1.0;
    other._x_component = 0.0;
    other._y_component = 0.0;
    other._z_component = 0.0;
    quaternion::unlock_pair(lower, upper);
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

int32_t quaternion::initialize(quaternion &&other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("quaternion::initialize(quaternion &&) source",
            "called with uninitialised source object");
        return (quaternion::set_error(FT_ERR_INVALID_STATE));
    }
    if (this == &other)
        return (quaternion::set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (quaternion::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_w_component = 1.0;
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (quaternion::set_error(FT_ERR_SUCCESS));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (quaternion::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (quaternion::set_error(initialization_error));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (quaternion::set_error(move_error));
    }
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

uint32_t quaternion::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (quaternion::set_error(FT_ERR_SUCCESS));
    disable_error = this->disable_thread_safety();
    this->_w_component = 1.0;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (quaternion::set_error(disable_error));
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

quaternion::~quaternion() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = quaternion::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    quaternion::set_error(previous_last_error);
    return ;
}

double  quaternion::get_w() const
{
    double value;
    int32_t lock_error;

    this->abort_if_not_initialised("quaternion::get_w");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (0.0);
    }
    value = this->_w_component;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    quaternion::set_error(FT_ERR_SUCCESS);
    return (value);
}

double  quaternion::get_x() const
{
    double value;
    int32_t lock_error;

    this->abort_if_not_initialised("quaternion::get_x");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (0.0);
    }
    value = this->_x_component;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    quaternion::set_error(FT_ERR_SUCCESS);
    return (value);
}

double  quaternion::get_y() const
{
    double value;
    int32_t lock_error;

    this->abort_if_not_initialised("quaternion::get_y");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (0.0);
    }
    value = this->_y_component;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    quaternion::set_error(FT_ERR_SUCCESS);
    return (value);
}

double  quaternion::get_z() const
{
    double value;
    int32_t lock_error;

    this->abort_if_not_initialised("quaternion::get_z");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (0.0);
    }
    value = this->_z_component;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    quaternion::set_error(FT_ERR_SUCCESS);
    return (value);
}

quaternion  quaternion::add(const quaternion &other) const
{
    const quaternion *lower;
    const quaternion *upper;
    int32_t lock_error;
    double result_w;
    double result_x;
    double result_y;
    double result_z;

    this->abort_if_not_initialised("quaternion::add");
    other.abort_if_not_initialised("quaternion::add source");
    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (quaternion());
    }
    result_w = this->_w_component + other._w_component;
    result_x = this->_x_component + other._x_component;
    result_y = this->_y_component + other._y_component;
    result_z = this->_z_component + other._z_component;
    quaternion::unlock_pair(lower, upper);
    quaternion::set_error(FT_ERR_SUCCESS);
    return (quaternion(result_w, result_x, result_y, result_z));
}

quaternion  quaternion::multiply(const quaternion &other) const
{
    const quaternion *lower;
    const quaternion *upper;
    int32_t lock_error;
    double result_w;
    double result_x;
    double result_y;
    double result_z;

    this->abort_if_not_initialised("quaternion::multiply");
    other.abort_if_not_initialised("quaternion::multiply source");
    lock_error = quaternion::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (quaternion());
    }
    quaternion_compute_product_components(result_w, result_x, result_y, result_z,
        this->_w_component, this->_x_component, this->_y_component, this->_z_component,
        other._w_component, other._x_component, other._y_component, other._z_component);
    quaternion::unlock_pair(lower, upper);
    quaternion::set_error(FT_ERR_SUCCESS);
    return (quaternion(result_w, result_x, result_y, result_z));
}

quaternion  quaternion::conjugate() const
{
    int32_t lock_error;
    double result_w;
    double result_x;
    double result_y;
    double result_z;

    this->abort_if_not_initialised("quaternion::conjugate");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (quaternion());
    }
    result_w = this->_w_component;
    result_x = -this->_x_component;
    result_y = -this->_y_component;
    result_z = -this->_z_component;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    quaternion::set_error(FT_ERR_SUCCESS);
    return (quaternion(result_w, result_x, result_y, result_z));
}

double  quaternion::length() const
{
    double length_value;
    int32_t lock_error;

    this->abort_if_not_initialised("quaternion::length");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (0.0);
    }
    length_value = quaternion_compute_dot(this->_w_component, this->_x_component,
        this->_y_component, this->_z_component,
        this->_w_component, this->_x_component, this->_y_component, this->_z_component);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    quaternion::set_error(FT_ERR_SUCCESS);
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
    int32_t lock_error;

    this->abort_if_not_initialised("quaternion::normalize");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        quaternion::set_error(static_cast<uint32_t>(lock_error));
        return (quaternion());
    }
    local_w = this->_w_component;
    local_x = this->_x_component;
    local_y = this->_y_component;
    local_z = this->_z_component;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    length_value = math_sqrt(local_w * local_w + local_x * local_x
        + local_y * local_y + local_z * local_z);
    epsilon = 0.0000001;
    if (math_absdiff(length_value, 0.0) <= epsilon)
    {
        quaternion::set_error(FT_ERR_SUCCESS);
        return (quaternion());
    }
    quaternion::set_error(FT_ERR_SUCCESS);
    return (quaternion(local_w / length_value, local_x / length_value,
            local_y / length_value, local_z / length_value));
}


uint32_t quaternion::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "quaternion::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (quaternion::set_error(FT_ERR_SUCCESS));
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (quaternion::set_error(FT_ERR_NO_MEMORY));
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (quaternion::set_error(static_cast<uint32_t>(mutex_error)));
    }
    this->_mutex = mutex_pointer;
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

uint32_t quaternion::disable_thread_safety() noexcept
{
    int32_t mutex_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "quaternion::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
        if (mutex_error != FT_ERR_SUCCESS)
            return (quaternion::set_error(static_cast<uint32_t>(mutex_error)));
    }
    return (quaternion::set_error(FT_ERR_SUCCESS));
}

ft_bool quaternion::is_thread_safe() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "quaternion::is_thread_safe");
    quaternion::set_error(FT_ERR_SUCCESS);
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
