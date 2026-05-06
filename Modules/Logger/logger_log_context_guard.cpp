#include "logger.hpp"
#include "logger_internal.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno_internal.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Template/move.hpp"

ft_log_context_guard::ft_log_context_guard() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _pushed_count(0), _active(FT_FALSE), _error_code(FT_ERR_SUCCESS)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_log_context_guard::ft_log_context_guard(
    const ft_log_context_guard &other) noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _pushed_count(0), _active(FT_FALSE), _error_code(FT_ERR_SUCCESS)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_log_context_guard copy constructor",
            "source object is uninitialised");
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

ft_log_context_guard::~ft_log_context_guard() noexcept
{
    (void)this->destroy();
    return ;
}

ft_log_context_guard::ft_log_context_guard(ft_log_context_guard &&other) noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _pushed_count(other._pushed_count), _active(other._active), _error_code(other._error_code)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_log_context_guard move constructor",
            "source object is uninitialised");
    this->_initialised_state = other._initialised_state;
    other._pushed_count = 0;
    other._active = FT_FALSE;
    other._error_code = FT_ERR_SUCCESS;
    other.set_error(FT_ERR_SUCCESS);
    this->set_error(this->_error_code);
    return ;
}

int32_t ft_log_context_guard::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state,
            "ft_log_context_guard::initialize",
            "already initialised");
    this->_pushed_count = 0;
    this->_active = FT_FALSE;
    this->_error_code = FT_ERR_SUCCESS;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_context_guard::initialize(
    const ft_log_context_guard &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_log_context_guard::initialize(copy)",
            "source object is uninitialised");
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this->_pushed_count = other._pushed_count;
    this->_active = other._active;
    this->_error_code = other._error_code;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_context_guard::initialize(
    ft_log_context_guard &&other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->initialize(static_cast<const ft_log_context_guard &>(other)) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    other._pushed_count = 0;
    other._active = FT_FALSE;
    other._error_code = FT_ERR_SUCCESS;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_context_guard::initialize(
    const s_log_field *fields, ft_size_t field_count) noexcept
{
    ft_size_t pushed_count;
    int32_t operation_result;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_NOT_INITIALISED);
        return (FT_ERR_NOT_INITIALISED);
    }
    operation_result = logger_context_push(fields, field_count, &pushed_count);
    if (operation_result != FT_ERR_SUCCESS)
    {
        this->_active = FT_FALSE;
        this->_pushed_count = 0;
        this->set_error(operation_result);
        return (operation_result);
    }
    this->_pushed_count = pushed_count;
    if (pushed_count != 0)
        this->_active = FT_TRUE;
    else
        this->_active = FT_FALSE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_log_context_guard::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    if (this->_active)
        logger_context_pop(this->_pushed_count);
    this->_pushed_count = 0;
    this->_active = FT_FALSE;
    this->_error_code = FT_ERR_SUCCESS;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

uint32_t ft_log_context_guard::move(ft_log_context_guard &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(ft_move(other))));
}

void ft_log_context_guard::set_error(int32_t error_code_value) const
{
    this->_error_code = error_code_value;
    return ;
}

void ft_log_context_guard::release() noexcept
{
    if (!this->_active)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    logger_context_pop(this->_pushed_count);
    this->_active = FT_FALSE;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_bool ft_log_context_guard::is_active() const noexcept
{
    ft_bool guard_active;

    guard_active = this->_active;
    const_cast<ft_log_context_guard *>(this)->set_error(FT_ERR_SUCCESS);
    return (guard_active);
}

int32_t ft_log_context_guard::get_error() const
{
    return (this->_error_code);
}

const char *ft_log_context_guard::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
