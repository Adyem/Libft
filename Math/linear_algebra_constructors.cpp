#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

static void linear_algebra_print_lifecycle_error(const char *class_name,
    const char *method_name, const char *reason)
{
    const char *resolved_class_name;
    const char *resolved_method_name;
    const char *resolved_reason;

    resolved_class_name = class_name;
    resolved_method_name = method_name;
    resolved_reason = reason;
    if (resolved_class_name == ft_nullptr)
        resolved_class_name = "unknown";
    if (resolved_method_name == ft_nullptr)
        resolved_method_name = "unknown";
    if (resolved_reason == ft_nullptr)
        resolved_reason = "unknown";
    pf_printf_fd(2, "%s lifecycle error: %s: %s\n",
        resolved_class_name, resolved_method_name, resolved_reason);
    su_abort();
    return ;
}

static void matrix2_set_identity(double matrix_values[2][2])
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
    int row;
    int column;

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
    int row;
    int column;

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
    linear_algebra_print_lifecycle_error("vector2", method_name, reason);
    return ;
}

void vector2::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == vector2::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int vector2::initialize() noexcept
{
    if (this->_initialized_state == vector2::_state_initialized)
    {
        this->abort_lifecycle_error("vector2::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_x = 0.0;
    this->_y = 0.0;
    this->_initialized_state = vector2::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int vector2::initialize(double x, double y) noexcept
{
    int initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    this->_x = x;
    this->_y = y;
    return (FT_ERR_SUCCESS);
}

int vector2::initialize(const vector2 &other) noexcept
{
    int initialization_error;
    int lock_error;
    int unlock_error;

    if (other._initialized_state != vector2::_state_initialized)
    {
        if (other._initialized_state == vector2::_state_uninitialized)
            other.abort_lifecycle_error("vector2::initialize(const vector2 &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector2::initialize(const vector2 &) source",
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
    this->_x = other._x;
    this->_y = other._y;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int vector2::move(vector2 &other) noexcept
{
    const vector2 *lower;
    const vector2 *upper;
    int initialize_error;
    int lock_error;

    if (other._initialized_state != vector2::_state_initialized)
    {
        if (other._initialized_state == vector2::_state_uninitialized)
            other.abort_lifecycle_error("vector2::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector2::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != vector2::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = vector2::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_x = other._x;
    this->_y = other._y;
    other._x = 0.0;
    other._y = 0.0;
    vector2::unlock_pair(lower, upper);
    return (FT_ERR_SUCCESS);
}

int vector2::initialize(vector2 &&other) noexcept
{
    int initialization_error;
    int move_error;

    if (other._initialized_state != vector2::_state_initialized)
    {
        if (other._initialized_state == vector2::_state_uninitialized)
            other.abort_lifecycle_error("vector2::initialize(vector2 &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector2::initialize(vector2 &&) source",
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

int vector2::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != vector2::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_x = 0.0;
    this->_y = 0.0;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = vector2::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

vector2::vector2()
{
    this->_x = 0.0;
    this->_y = 0.0;
    this->_initialized_state = vector2::_state_uninitialized;
    return ;
}

vector2::vector2(double x, double y)
{
    int initialization_error;

    this->_x = 0.0;
    this->_y = 0.0;
    this->_initialized_state = vector2::_state_uninitialized;
    initialization_error = this->initialize(x, y);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialized_state == vector2::_state_uninitialized)
        this->_initialized_state = vector2::_state_destroyed;
    return ;
}

vector2::~vector2()
{
    if (this->_initialized_state == vector2::_state_initialized)
        (void)this->destroy();
    return ;
}

void vector3::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    linear_algebra_print_lifecycle_error("vector3", method_name, reason);
    return ;
}

void vector3::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == vector3::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int vector3::initialize() noexcept
{
    if (this->_initialized_state == vector3::_state_initialized)
    {
        this->abort_lifecycle_error("vector3::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_initialized_state = vector3::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int vector3::initialize(double x, double y, double z) noexcept
{
    int initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    this->_x = x;
    this->_y = y;
    this->_z = z;
    return (FT_ERR_SUCCESS);
}

int vector3::initialize(const vector3 &other) noexcept
{
    int initialization_error;
    int lock_error;
    int unlock_error;

    if (other._initialized_state != vector3::_state_initialized)
    {
        if (other._initialized_state == vector3::_state_uninitialized)
            other.abort_lifecycle_error("vector3::initialize(const vector3 &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector3::initialize(const vector3 &) source",
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

int vector3::move(vector3 &other) noexcept
{
    const vector3 *lower;
    const vector3 *upper;
    int initialize_error;
    int lock_error;

    if (other._initialized_state != vector3::_state_initialized)
    {
        if (other._initialized_state == vector3::_state_uninitialized)
            other.abort_lifecycle_error("vector3::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector3::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != vector3::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = vector3::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    vector3::unlock_pair(lower, upper);
    return (FT_ERR_SUCCESS);
}

int vector3::initialize(vector3 &&other) noexcept
{
    int initialization_error;
    int move_error;

    if (other._initialized_state != vector3::_state_initialized)
    {
        if (other._initialized_state == vector3::_state_uninitialized)
            other.abort_lifecycle_error("vector3::initialize(vector3 &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector3::initialize(vector3 &&) source",
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

int vector3::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != vector3::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = vector3::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

vector3::vector3()
{
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_initialized_state = vector3::_state_uninitialized;
    return ;
}

vector3::vector3(double x, double y, double z)
{
    int initialization_error;

    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_initialized_state = vector3::_state_uninitialized;
    initialization_error = this->initialize(x, y, z);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialized_state == vector3::_state_uninitialized)
        this->_initialized_state = vector3::_state_destroyed;
    return ;
}

vector3::~vector3()
{
    if (this->_initialized_state == vector3::_state_initialized)
        (void)this->destroy();
    return ;
}

void vector4::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    linear_algebra_print_lifecycle_error("vector4", method_name, reason);
    return ;
}

void vector4::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == vector4::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int vector4::initialize() noexcept
{
    if (this->_initialized_state == vector4::_state_initialized)
    {
        this->abort_lifecycle_error("vector4::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_w = 0.0;
    this->_initialized_state = vector4::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int vector4::initialize(double x, double y, double z, double w) noexcept
{
    int initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->_w = w;
    return (FT_ERR_SUCCESS);
}

int vector4::initialize(const vector4 &other) noexcept
{
    int initialization_error;
    int lock_error;
    int unlock_error;

    if (other._initialized_state != vector4::_state_initialized)
    {
        if (other._initialized_state == vector4::_state_uninitialized)
            other.abort_lifecycle_error("vector4::initialize(const vector4 &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector4::initialize(const vector4 &) source",
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
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_w = other._w;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int vector4::move(vector4 &other) noexcept
{
    const vector4 *lower;
    const vector4 *upper;
    int initialize_error;
    int lock_error;

    if (other._initialized_state != vector4::_state_initialized)
    {
        if (other._initialized_state == vector4::_state_uninitialized)
            other.abort_lifecycle_error("vector4::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector4::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != vector4::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = vector4::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_w = other._w;
    other._x = 0.0;
    other._y = 0.0;
    other._z = 0.0;
    other._w = 0.0;
    vector4::unlock_pair(lower, upper);
    return (FT_ERR_SUCCESS);
}

int vector4::initialize(vector4 &&other) noexcept
{
    int initialization_error;
    int move_error;

    if (other._initialized_state != vector4::_state_initialized)
    {
        if (other._initialized_state == vector4::_state_uninitialized)
            other.abort_lifecycle_error("vector4::initialize(vector4 &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("vector4::initialize(vector4 &&) source",
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

int vector4::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != vector4::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_w = 0.0;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = vector4::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

vector4::vector4()
{
    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_w = 0.0;
    this->_initialized_state = vector4::_state_uninitialized;
    return ;
}

vector4::vector4(double x, double y, double z, double w)
{
    int initialization_error;

    this->_x = 0.0;
    this->_y = 0.0;
    this->_z = 0.0;
    this->_w = 0.0;
    this->_initialized_state = vector4::_state_uninitialized;
    initialization_error = this->initialize(x, y, z, w);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialized_state == vector4::_state_uninitialized)
        this->_initialized_state = vector4::_state_destroyed;
    return ;
}

vector4::~vector4()
{
    if (this->_initialized_state == vector4::_state_initialized)
        (void)this->destroy();
    return ;
}

void matrix2::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    linear_algebra_print_lifecycle_error("matrix2", method_name, reason);
    return ;
}

void matrix2::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == matrix2::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int matrix2::initialize() noexcept
{
    if (this->_initialized_state == matrix2::_state_initialized)
    {
        this->abort_lifecycle_error("matrix2::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    matrix2_set_identity(this->_m);
    this->_initialized_state = matrix2::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int matrix2::initialize(double m00, double m01, double m10, double m11) noexcept
{
    int initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    this->_m[0][0] = m00;
    this->_m[0][1] = m01;
    this->_m[1][0] = m10;
    this->_m[1][1] = m11;
    return (FT_ERR_SUCCESS);
}

int matrix2::initialize(const matrix2 &other) noexcept
{
    int initialization_error;
    int lock_error;
    int unlock_error;
    int row;
    int column;

    if (other._initialized_state != matrix2::_state_initialized)
    {
        if (other._initialized_state == matrix2::_state_uninitialized)
            other.abort_lifecycle_error("matrix2::initialize(const matrix2 &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix2::initialize(const matrix2 &) source",
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
    unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int matrix2::move(matrix2 &other) noexcept
{
    const matrix2 *lower;
    const matrix2 *upper;
    int initialize_error;
    int lock_error;
    int row;
    int column;

    if (other._initialized_state != matrix2::_state_initialized)
    {
        if (other._initialized_state == matrix2::_state_uninitialized)
            other.abort_lifecycle_error("matrix2::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix2::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != matrix2::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = matrix2::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
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
    return (FT_ERR_SUCCESS);
}

int matrix2::initialize(matrix2 &&other) noexcept
{
    int initialization_error;
    int move_error;

    if (other._initialized_state != matrix2::_state_initialized)
    {
        if (other._initialized_state == matrix2::_state_uninitialized)
            other.abort_lifecycle_error("matrix2::initialize(matrix2 &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix2::initialize(matrix2 &&) source",
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

int matrix2::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != matrix2::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    matrix2_set_identity(this->_m);
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = matrix2::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

matrix2::matrix2()
{
    matrix2_set_identity(this->_m);
    this->_initialized_state = matrix2::_state_uninitialized;
    return ;
}

matrix2::matrix2(double m00, double m01, double m10, double m11)
{
    int initialization_error;

    matrix2_set_identity(this->_m);
    this->_initialized_state = matrix2::_state_uninitialized;
    initialization_error = this->initialize(m00, m01, m10, m11);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialized_state == matrix2::_state_uninitialized)
        this->_initialized_state = matrix2::_state_destroyed;
    return ;
}

matrix2::~matrix2()
{
    if (this->_initialized_state == matrix2::_state_initialized)
        (void)this->destroy();
    return ;
}

void matrix3::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    linear_algebra_print_lifecycle_error("matrix3", method_name, reason);
    return ;
}

void matrix3::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == matrix3::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int matrix3::initialize() noexcept
{
    if (this->_initialized_state == matrix3::_state_initialized)
    {
        this->abort_lifecycle_error("matrix3::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    matrix3_set_identity(this->_m);
    this->_initialized_state = matrix3::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int matrix3::initialize(double m00, double m01, double m02,
    double m10, double m11, double m12, double m20, double m21,
    double m22) noexcept
{
    int initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    this->_m[0][0] = m00;
    this->_m[0][1] = m01;
    this->_m[0][2] = m02;
    this->_m[1][0] = m10;
    this->_m[1][1] = m11;
    this->_m[1][2] = m12;
    this->_m[2][0] = m20;
    this->_m[2][1] = m21;
    this->_m[2][2] = m22;
    return (FT_ERR_SUCCESS);
}

int matrix3::initialize(const matrix3 &other) noexcept
{
    int initialization_error;
    int lock_error;
    int unlock_error;
    int row;
    int column;

    if (other._initialized_state != matrix3::_state_initialized)
    {
        if (other._initialized_state == matrix3::_state_uninitialized)
            other.abort_lifecycle_error("matrix3::initialize(const matrix3 &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix3::initialize(const matrix3 &) source",
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
    unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int matrix3::move(matrix3 &other) noexcept
{
    const matrix3 *lower;
    const matrix3 *upper;
    int initialize_error;
    int lock_error;
    int row;
    int column;

    if (other._initialized_state != matrix3::_state_initialized)
    {
        if (other._initialized_state == matrix3::_state_uninitialized)
            other.abort_lifecycle_error("matrix3::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix3::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != matrix3::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = matrix3::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
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
    return (FT_ERR_SUCCESS);
}

int matrix3::initialize(matrix3 &&other) noexcept
{
    int initialization_error;
    int move_error;

    if (other._initialized_state != matrix3::_state_initialized)
    {
        if (other._initialized_state == matrix3::_state_uninitialized)
            other.abort_lifecycle_error("matrix3::initialize(matrix3 &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix3::initialize(matrix3 &&) source",
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

int matrix3::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != matrix3::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    matrix3_set_identity(this->_m);
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = matrix3::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

matrix3::matrix3()
{
    matrix3_set_identity(this->_m);
    this->_initialized_state = matrix3::_state_uninitialized;
    return ;
}

matrix3::matrix3(double m00, double m01, double m02,
    double m10, double m11, double m12, double m20, double m21, double m22)
{
    int initialization_error;

    matrix3_set_identity(this->_m);
    this->_initialized_state = matrix3::_state_uninitialized;
    initialization_error = this->initialize(m00, m01, m02, m10, m11, m12,
            m20, m21, m22);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialized_state == matrix3::_state_uninitialized)
        this->_initialized_state = matrix3::_state_destroyed;
    return ;
}

matrix3::~matrix3()
{
    if (this->_initialized_state == matrix3::_state_initialized)
        (void)this->destroy();
    return ;
}

void matrix4::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    linear_algebra_print_lifecycle_error("matrix4", method_name, reason);
    return ;
}

void matrix4::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == matrix4::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int matrix4::initialize() noexcept
{
    if (this->_initialized_state == matrix4::_state_initialized)
    {
        this->abort_lifecycle_error("matrix4::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    matrix4_set_identity(this->_m);
    this->_initialized_state = matrix4::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int matrix4::initialize(double m00, double m01, double m02, double m03,
    double m10, double m11, double m12, double m13,
    double m20, double m21, double m22, double m23,
    double m30, double m31, double m32, double m33) noexcept
{
    int initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
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
    return (FT_ERR_SUCCESS);
}

int matrix4::initialize(const matrix4 &other) noexcept
{
    int initialization_error;
    int lock_error;
    int unlock_error;
    int row;
    int column;

    if (other._initialized_state != matrix4::_state_initialized)
    {
        if (other._initialized_state == matrix4::_state_uninitialized)
            other.abort_lifecycle_error("matrix4::initialize(const matrix4 &) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix4::initialize(const matrix4 &) source",
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
    unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int matrix4::move(matrix4 &other) noexcept
{
    const matrix4 *lower;
    const matrix4 *upper;
    int initialize_error;
    int lock_error;
    int row;
    int column;

    if (other._initialized_state != matrix4::_state_initialized)
    {
        if (other._initialized_state == matrix4::_state_uninitialized)
            other.abort_lifecycle_error("matrix4::move source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix4::move source",
                "called with destroyed source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state != matrix4::_state_initialized)
    {
        initialize_error = this->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    lock_error = matrix4::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
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
    return (FT_ERR_SUCCESS);
}

int matrix4::initialize(matrix4 &&other) noexcept
{
    int initialization_error;
    int move_error;

    if (other._initialized_state != matrix4::_state_initialized)
    {
        if (other._initialized_state == matrix4::_state_uninitialized)
            other.abort_lifecycle_error("matrix4::initialize(matrix4 &&) source",
                "called with uninitialized source object");
        else
            other.abort_lifecycle_error("matrix4::initialize(matrix4 &&) source",
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

int matrix4::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != matrix4::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    matrix4_set_identity(this->_m);
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = matrix4::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

matrix4::matrix4()
{
    matrix4_set_identity(this->_m);
    this->_initialized_state = matrix4::_state_uninitialized;
    return ;
}

matrix4::matrix4(double m00, double m01, double m02, double m03,
    double m10, double m11, double m12, double m13,
    double m20, double m21, double m22, double m23,
    double m30, double m31, double m32, double m33)
{
    int initialization_error;

    matrix4_set_identity(this->_m);
    this->_initialized_state = matrix4::_state_uninitialized;
    initialization_error = this->initialize(m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33);
    if (initialization_error != FT_ERR_SUCCESS
        && this->_initialized_state == matrix4::_state_uninitialized)
        this->_initialized_state = matrix4::_state_destroyed;
    return ;
}

matrix4::~matrix4()
{
    if (this->_initialized_state == matrix4::_state_initialized)
        (void)this->destroy();
    return ;
}
