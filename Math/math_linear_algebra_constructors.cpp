#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"

thread_local uint32_t vector2::_last_error = FT_ERR_SUCCESS;
thread_local uint32_t vector3::_last_error = FT_ERR_SUCCESS;
thread_local uint32_t vector4::_last_error = FT_ERR_SUCCESS;
thread_local uint32_t matrix2::_last_error = FT_ERR_SUCCESS;
thread_local uint32_t matrix3::_last_error = FT_ERR_SUCCESS;
thread_local uint32_t matrix4::_last_error = FT_ERR_SUCCESS;

uint32_t vector2::set_error(uint32_t error_code) noexcept
{
    vector2::_last_error = error_code;
    return (error_code);
}

uint32_t vector2::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "vector2::get_error",
            "called while object is uninitialised");
    return (vector2::_last_error);
}

const char *vector2::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "vector2::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(vector2::_last_error));
}

uint32_t vector3::set_error(uint32_t error_code) noexcept
{
    vector3::_last_error = error_code;
    return (error_code);
}

uint32_t vector3::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "vector3::get_error",
            "called while object is uninitialised");
    return (vector3::_last_error);
}

const char *vector3::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "vector3::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(vector3::_last_error));
}

uint32_t vector4::set_error(uint32_t error_code) noexcept
{
    vector4::_last_error = error_code;
    return (error_code);
}

uint32_t vector4::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "vector4::get_error",
            "called while object is uninitialised");
    return (vector4::_last_error);
}

const char *vector4::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "vector4::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(vector4::_last_error));
}

uint32_t matrix2::set_error(uint32_t error_code) noexcept
{
    matrix2::_last_error = error_code;
    return (error_code);
}

uint32_t matrix2::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "matrix2::get_error",
            "called while object is uninitialised");
    return (matrix2::_last_error);
}

const char *matrix2::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "matrix2::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(matrix2::_last_error));
}

uint32_t matrix3::set_error(uint32_t error_code) noexcept
{
    matrix3::_last_error = error_code;
    return (error_code);
}

uint32_t matrix3::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "matrix3::get_error",
            "called while object is uninitialised");
    return (matrix3::_last_error);
}

const char *matrix3::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "matrix3::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(matrix3::_last_error));
}

uint32_t matrix4::set_error(uint32_t error_code) noexcept
{
    matrix4::_last_error = error_code;
    return (error_code);
}

uint32_t matrix4::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "matrix4::get_error",
            "called while object is uninitialised");
    return (matrix4::_last_error);
}

const char *matrix4::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "matrix4::get_error_str",
            "called while object is uninitialised");
    return (ft_strerror(matrix4::_last_error));
}

static void matrix2_set_identity(double matrix_values[2][2])
{
    int32_t row;
    int32_t column;

    row = 0;
    while (row < 2)
    {
        column = 0;
        while (column < 2)
        {
            if (row == column)
                matrix_values[row][column] = 1.0;
            else
                matrix_values[row][column] = 0.0;
            column++;
        }
        row++;
    }
    return ;
}

static void matrix3_set_identity(double matrix_values[3][3])
{
    int32_t row;
    int32_t column;

    row = 0;
    while (row < 3)
    {
        column = 0;
        while (column < 3)
        {
            if (row == column)
                matrix_values[row][column] = 1.0;
            else
                matrix_values[row][column] = 0.0;
            column++;
        }
        row++;
    }
    return ;
}

static void matrix4_set_identity(double matrix_values[4][4])
{
    int32_t row;
    int32_t column;

    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            if (row == column)
                matrix_values[row][column] = 1.0;
            else
                matrix_values[row][column] = 0.0;
            column++;
        }
        row++;
    }
    return ;
}

void vector2::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void vector2::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t vector2::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("vector2::initialize",
            "called while object is already initialised");
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector2::initialize(double x_component, double y_component) noexcept
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector2::set_error(static_cast<uint32_t>(initialization_error)));
    this->_x_component = x_component;
    this->_y_component = y_component;
    return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector2::initialize(const vector2 &other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector2::initialize(const vector2 &) source",
            "called with uninitialised source object");
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector2::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector2::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector2::set_error(static_cast<uint32_t>(initialization_error)));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (vector2::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t vector2::move(vector2 &other) noexcept
{
    const vector2 *lower;
    const vector2 *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector2::move source",
            "called with uninitialised source object");
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector2::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector2::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (vector2::set_error(static_cast<uint32_t>(initialize_error)));
    }
    lock_error = vector2::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector2::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    other._x_component = 0.0;
    other._y_component = 0.0;
    vector2::unlock_pair(lower, upper);
    return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector2::initialize(vector2 &&other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector2::initialize(vector2 &&) source",
            "called with uninitialised source object");
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector2::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector2::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector2::set_error(static_cast<uint32_t>(initialization_error)));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (vector2::set_error(static_cast<uint32_t>(move_error)));
    }
    return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t vector2::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    disable_error = this->disable_thread_safety();
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (vector2::set_error(static_cast<uint32_t>(disable_error)));
    return (vector2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

vector2::vector2() noexcept
{
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

vector2::vector2(double x_component, double y_component) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector2::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    initialization_error = this->initialize(x_component, y_component);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector2::set_error(previous_last_error);
    return ;
}

vector2::vector2(const vector2 &other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector2::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector2::vector2(const vector2 &) source",
            "called with uninitialised source object");
        vector2::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        vector2::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(other);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector2::set_error(previous_last_error);
    return ;
}

vector2::vector2(vector2 &&other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector2::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector2::vector2(vector2 &&) source",
            "called with uninitialised source object");
        vector2::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        vector2::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(static_cast<vector2 &&>(other));
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector2::set_error(previous_last_error);
    return ;
}

vector2::~vector2() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = vector2::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    vector2::set_error(previous_last_error);
    return ;
}

void vector3::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void vector3::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t vector3::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("vector3::initialize",
            "called while object is already initialised");
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector3::initialize(double x_component, double y_component, double z_component) noexcept
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector3::set_error(static_cast<uint32_t>(initialization_error)));
    this->_x_component = x_component;
    this->_y_component = y_component;
    this->_z_component = z_component;
    return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector3::initialize(const vector3 &other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector3::initialize(const vector3 &) source",
            "called with uninitialised source object");
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector3::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector3::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector3::set_error(static_cast<uint32_t>(initialization_error)));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (vector3::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    this->_z_component = other._z_component;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t vector3::move(vector3 &other) noexcept
{
    const vector3 *lower;
    const vector3 *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector3::move source",
            "called with uninitialised source object");
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector3::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector3::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (vector3::set_error(static_cast<uint32_t>(initialize_error)));
    }
    lock_error = vector3::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector3::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    this->_z_component = other._z_component;
    other._x_component = 0.0;
    other._y_component = 0.0;
    other._z_component = 0.0;
    vector3::unlock_pair(lower, upper);
    return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector3::initialize(vector3 &&other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector3::initialize(vector3 &&) source",
            "called with uninitialised source object");
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector3::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector3::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector3::set_error(static_cast<uint32_t>(initialization_error)));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (vector3::set_error(static_cast<uint32_t>(move_error)));
    }
    return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t vector3::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    disable_error = this->disable_thread_safety();
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (vector3::set_error(static_cast<uint32_t>(disable_error)));
    return (vector3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

vector3::vector3() noexcept
{
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

vector3::vector3(double x_component, double y_component, double z_component) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector3::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    initialization_error = this->initialize(x_component, y_component, z_component);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector3::set_error(previous_last_error);
    return ;
}

vector3::vector3(const vector3 &other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector3::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector3::vector3(const vector3 &) source",
            "called with uninitialised source object");
        vector3::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        vector3::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(other);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector3::set_error(previous_last_error);
    return ;
}

vector3::vector3(vector3 &&other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector3::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector3::vector3(vector3 &&) source",
            "called with uninitialised source object");
        vector3::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        vector3::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(static_cast<vector3 &&>(other));
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector3::set_error(previous_last_error);
    return ;
}

vector3::~vector3() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = vector3::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    vector3::set_error(previous_last_error);
    return ;
}

void vector4::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void vector4::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t vector4::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("vector4::initialize",
            "called while object is already initialised");
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_w_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector4::initialize(double x_component, double y_component, double z_component, double w_component) noexcept
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector4::set_error(static_cast<uint32_t>(initialization_error)));
    this->_x_component = x_component;
    this->_y_component = y_component;
    this->_z_component = z_component;
    this->_w_component = w_component;
    return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector4::initialize(const vector4 &other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector4::initialize(const vector4 &) source",
            "called with uninitialised source object");
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector4::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_w_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector4::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector4::set_error(static_cast<uint32_t>(initialization_error)));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (vector4::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    this->_z_component = other._z_component;
    this->_w_component = other._w_component;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t vector4::move(vector4 &other) noexcept
{
    const vector4 *lower;
    const vector4 *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector4::move source",
            "called with uninitialised source object");
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector4::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_w_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector4::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (vector4::set_error(static_cast<uint32_t>(initialize_error)));
    }
    lock_error = vector4::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_w_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector4::set_error(static_cast<uint32_t>(lock_error)));
    }
    this->_x_component = other._x_component;
    this->_y_component = other._y_component;
    this->_z_component = other._z_component;
    this->_w_component = other._w_component;
    other._x_component = 0.0;
    other._y_component = 0.0;
    other._z_component = 0.0;
    other._w_component = 0.0;
    vector4::unlock_pair(lower, upper);
    return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t vector4::initialize(vector4 &&other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector4::initialize(vector4 &&) source",
            "called with uninitialised source object");
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (vector4::set_error(static_cast<uint32_t>(destroy_error)));
        }
        this->_x_component = 0.0;
        this->_y_component = 0.0;
        this->_z_component = 0.0;
        this->_w_component = 0.0;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (vector4::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (vector4::set_error(static_cast<uint32_t>(initialization_error)));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (vector4::set_error(static_cast<uint32_t>(move_error)));
    }
    return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t vector4::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    disable_error = this->disable_thread_safety();
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_w_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (vector4::set_error(static_cast<uint32_t>(disable_error)));
    return (vector4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

vector4::vector4() noexcept
{
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_w_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

vector4::vector4(double x_component, double y_component, double z_component, double w_component) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector4::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_w_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    initialization_error = this->initialize(x_component, y_component, z_component, w_component);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector4::set_error(previous_last_error);
    return ;
}

vector4::vector4(const vector4 &other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector4::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_w_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector4::vector4(const vector4 &) source",
            "called with uninitialised source object");
        vector4::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        vector4::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(other);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector4::set_error(previous_last_error);
    return ;
}

vector4::vector4(vector4 &&other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = vector4::_last_error;
    this->_x_component = 0.0;
    this->_y_component = 0.0;
    this->_z_component = 0.0;
    this->_w_component = 0.0;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("vector4::vector4(vector4 &&) source",
            "called with uninitialised source object");
        vector4::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        vector4::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(static_cast<vector4 &&>(other));
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    vector4::set_error(previous_last_error);
    return ;
}

vector4::~vector4() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = vector4::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    vector4::set_error(previous_last_error);
    return ;
}

void matrix2::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void matrix2::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t matrix2::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("matrix2::initialize",
            "called while object is already initialised");
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    matrix2_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix2::initialize(double m00, double m01, double m10, double m11) noexcept
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix2::set_error(static_cast<uint32_t>(initialization_error)));
    this->_m[0][0] = m00;
    this->_m[0][1] = m01;
    this->_m[1][0] = m10;
    this->_m[1][1] = m11;
    return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix2::initialize(const matrix2 &other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t lock_error;
    int32_t row;
    int32_t column;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix2::initialize(const matrix2 &) source",
            "called with uninitialised source object");
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix2::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix2_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix2::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix2::set_error(static_cast<uint32_t>(initialization_error)));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (matrix2::set_error(static_cast<uint32_t>(lock_error)));
    }
    row = 0;
    while (row < 2)
    {
        column = 0;
        while (column < 2)
        {
            this->_m[row][column] = other._m[row][column];
            column++;
        }
        row++;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t matrix2::move(matrix2 &other) noexcept
{
    const matrix2 *lower;
    const matrix2 *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;
    int32_t row;
    int32_t column;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix2::move source",
            "called with uninitialised source object");
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix2::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix2_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix2::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (matrix2::set_error(static_cast<uint32_t>(initialize_error)));
    }
    lock_error = matrix2::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        matrix2_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix2::set_error(static_cast<uint32_t>(lock_error)));
    }
    row = 0;
    while (row < 2)
    {
        column = 0;
        while (column < 2)
        {
            this->_m[row][column] = other._m[row][column];
            if (row == column)
                other._m[row][column] = 1.0;
            else
                other._m[row][column] = 0.0;
            column++;
        }
        row++;
    }
    matrix2::unlock_pair(lower, upper);
    return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix2::initialize(matrix2 &&other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix2::initialize(matrix2 &&) source",
            "called with uninitialised source object");
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix2::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix2_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix2::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix2::set_error(static_cast<uint32_t>(initialization_error)));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (matrix2::set_error(static_cast<uint32_t>(move_error)));
    }
    return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t matrix2::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    disable_error = this->disable_thread_safety();
    matrix2_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (matrix2::set_error(static_cast<uint32_t>(disable_error)));
    return (matrix2::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

matrix2::matrix2() noexcept
{
    matrix2_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

matrix2::matrix2(double m00, double m01, double m10, double m11) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix2::_last_error;
    matrix2_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    initialization_error = this->initialize(m00, m01, m10, m11);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix2::set_error(previous_last_error);
    return ;
}

matrix2::matrix2(const matrix2 &other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix2::_last_error;
    matrix2_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix2::matrix2(const matrix2 &) source",
            "called with uninitialised source object");
        matrix2::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        matrix2::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(other);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix2::set_error(previous_last_error);
    return ;
}

matrix2::matrix2(matrix2 &&other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix2::_last_error;
    matrix2_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix2::matrix2(matrix2 &&) source",
            "called with uninitialised source object");
        matrix2::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        matrix2::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(static_cast<matrix2 &&>(other));
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix2::set_error(previous_last_error);
    return ;
}

matrix2::~matrix2() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = matrix2::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    matrix2::set_error(previous_last_error);
    return ;
}

void matrix3::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void matrix3::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t matrix3::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("matrix3::initialize",
            "called while object is already initialised");
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    matrix3_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix3::initialize(double m00, double m01, double m02,
    double m10, double m11, double m12, double m20, double m21,
    double m22) noexcept
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix3::set_error(static_cast<uint32_t>(initialization_error)));
    this->_m[0][0] = m00;
    this->_m[0][1] = m01;
    this->_m[0][2] = m02;
    this->_m[1][0] = m10;
    this->_m[1][1] = m11;
    this->_m[1][2] = m12;
    this->_m[2][0] = m20;
    this->_m[2][1] = m21;
    this->_m[2][2] = m22;
    return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix3::initialize(const matrix3 &other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t lock_error;
    int32_t row;
    int32_t column;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix3::initialize(const matrix3 &) source",
            "called with uninitialised source object");
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix3::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix3_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix3::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix3::set_error(static_cast<uint32_t>(initialization_error)));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (matrix3::set_error(static_cast<uint32_t>(lock_error)));
    }
    row = 0;
    while (row < 3)
    {
        column = 0;
        while (column < 3)
        {
            this->_m[row][column] = other._m[row][column];
            column++;
        }
        row++;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t matrix3::move(matrix3 &other) noexcept
{
    const matrix3 *lower;
    const matrix3 *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;
    int32_t row;
    int32_t column;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix3::move source",
            "called with uninitialised source object");
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix3::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix3_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix3::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (matrix3::set_error(static_cast<uint32_t>(initialize_error)));
    }
    lock_error = matrix3::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        matrix3_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix3::set_error(static_cast<uint32_t>(lock_error)));
    }
    row = 0;
    while (row < 3)
    {
        column = 0;
        while (column < 3)
        {
            this->_m[row][column] = other._m[row][column];
            if (row == column)
                other._m[row][column] = 1.0;
            else
                other._m[row][column] = 0.0;
            column++;
        }
        row++;
    }
    matrix3::unlock_pair(lower, upper);
    return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix3::initialize(matrix3 &&other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix3::initialize(matrix3 &&) source",
            "called with uninitialised source object");
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix3::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix3_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix3::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix3::set_error(static_cast<uint32_t>(initialization_error)));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (matrix3::set_error(static_cast<uint32_t>(move_error)));
    }
    return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t matrix3::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    disable_error = this->disable_thread_safety();
    matrix3_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (matrix3::set_error(static_cast<uint32_t>(disable_error)));
    return (matrix3::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

matrix3::matrix3() noexcept
{
    matrix3_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

matrix3::matrix3(double m00, double m01, double m02,
    double m10, double m11, double m12, double m20, double m21, double m22) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix3::_last_error;
    matrix3_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    initialization_error = this->initialize(m00, m01, m02, m10, m11, m12,
            m20, m21, m22);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix3::set_error(previous_last_error);
    return ;
}

matrix3::matrix3(const matrix3 &other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix3::_last_error;
    matrix3_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix3::matrix3(const matrix3 &) source",
            "called with uninitialised source object");
        matrix3::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        matrix3::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(other);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix3::set_error(previous_last_error);
    return ;
}

matrix3::matrix3(matrix3 &&other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix3::_last_error;
    matrix3_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix3::matrix3(matrix3 &&) source",
            "called with uninitialised source object");
        matrix3::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        matrix3::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(static_cast<matrix3 &&>(other));
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix3::set_error(previous_last_error);
    return ;
}

matrix3::~matrix3() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = matrix3::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    matrix3::set_error(previous_last_error);
    return ;
}

void matrix4::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void matrix4::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t matrix4::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        this->abort_lifecycle_error("matrix4::initialize",
            "called while object is already initialised");
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    matrix4_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix4::initialize(double m00, double m01, double m02, double m03,
    double m10, double m11, double m12, double m13,
    double m20, double m21, double m22, double m23,
    double m30, double m31, double m32, double m33) noexcept
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix4::set_error(static_cast<uint32_t>(initialization_error)));
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
    return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix4::initialize(const matrix4 &other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t lock_error;
    int32_t row;
    int32_t column;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix4::initialize(const matrix4 &) source",
            "called with uninitialised source object");
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix4::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix4_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix4::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix4::set_error(static_cast<uint32_t>(initialization_error)));
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (matrix4::set_error(static_cast<uint32_t>(lock_error)));
    }
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            this->_m[row][column] = other._m[row][column];
            column++;
        }
        row++;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t matrix4::move(matrix4 &other) noexcept
{
    const matrix4 *lower;
    const matrix4 *upper;
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t lock_error;
    int32_t row;
    int32_t column;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix4::move source",
            "called with uninitialised source object");
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix4::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix4_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix4::set_error(static_cast<uint32_t>(destroy_error)));
    }
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (matrix4::set_error(static_cast<uint32_t>(initialize_error)));
    }
    lock_error = matrix4::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        matrix4_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix4::set_error(static_cast<uint32_t>(lock_error)));
    }
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            this->_m[row][column] = other._m[row][column];
            if (row == column)
                other._m[row][column] = 1.0;
            else
                other._m[row][column] = 0.0;
            column++;
        }
        row++;
    }
    matrix4::unlock_pair(lower, upper);
    return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

int32_t matrix4::initialize(matrix4 &&other) noexcept
{
    int32_t initialization_error;
    int32_t destroy_error;
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix4::initialize(matrix4 &&) source",
            "called with uninitialised source object");
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_INVALID_STATE)));
    }
    if (this == &other)
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            destroy_error = this->destroy();
            if (destroy_error != FT_ERR_SUCCESS)
                return (matrix4::set_error(static_cast<uint32_t>(destroy_error)));
        }
        matrix4_set_identity(this->_m);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (matrix4::set_error(static_cast<uint32_t>(destroy_error)));
    }
    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (matrix4::set_error(static_cast<uint32_t>(initialization_error)));
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (matrix4::set_error(static_cast<uint32_t>(move_error)));
    }
    return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

uint32_t matrix4::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
    disable_error = this->disable_thread_safety();
    matrix4_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (matrix4::set_error(static_cast<uint32_t>(disable_error)));
    return (matrix4::set_error(static_cast<uint32_t>(FT_ERR_SUCCESS)));
}

matrix4::matrix4() noexcept
{
    matrix4_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

matrix4::matrix4(double m00, double m01, double m02, double m03,
    double m10, double m11, double m12, double m13,
    double m20, double m21, double m22, double m23,
    double m30, double m31, double m32, double m33) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix4::_last_error;
    matrix4_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    initialization_error = this->initialize(m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix4::set_error(previous_last_error);
    return ;
}

matrix4::matrix4(const matrix4 &other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix4::_last_error;
    matrix4_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix4::matrix4(const matrix4 &) source",
            "called with uninitialised source object");
        matrix4::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        matrix4::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(other);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix4::set_error(previous_last_error);
    return ;
}

matrix4::matrix4(matrix4 &&other) noexcept
{
    int32_t initialization_error;
    uint32_t previous_last_error;

    previous_last_error = matrix4::_last_error;
    matrix4_set_identity(this->_m);
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        other.abort_lifecycle_error("matrix4::matrix4(matrix4 &&) source",
            "called with uninitialised source object");
        matrix4::set_error(previous_last_error);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        matrix4::set_error(previous_last_error);
        return ;
    }
    initialization_error = this->initialize(static_cast<matrix4 &&>(other));
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    matrix4::set_error(previous_last_error);
    return ;
}

matrix4::~matrix4() noexcept
{
    uint32_t previous_last_error;

    previous_last_error = matrix4::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    matrix4::set_error(previous_last_error);
    return ;
}
