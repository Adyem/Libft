#include "class_stringbuf.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

void ft_stringbuf::abort_lifecycle_error(const char *method_name,
    const char *reason) noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_stringbuf lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_stringbuf::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_stringbuf::_state_initialized)
        return ;
    ft_stringbuf::abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int ft_stringbuf::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_stringbuf::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

ft_stringbuf::ft_stringbuf() noexcept
    : _storage(), _position(0), _mutex(ft_nullptr),
      _initialized_state(ft_stringbuf::_state_uninitialized)
{
    return ;
}

ft_stringbuf::~ft_stringbuf() noexcept
{
    if (this->_initialized_state == ft_stringbuf::_state_uninitialized)
    {
        ft_stringbuf::abort_lifecycle_error("ft_stringbuf::~ft_stringbuf",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_stringbuf::_state_initialized)
    {
        int destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return ;
    }
    return ;
}

int ft_stringbuf::initialize(const ft_string &string) noexcept
{
    int assign_error;
    int storage_initialize_error;

    if (this->_initialized_state == ft_stringbuf::_state_initialized)
    {
        ft_stringbuf::abort_lifecycle_error("ft_stringbuf::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_position = 0;
    storage_initialize_error = this->_storage.initialize();
    if (storage_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_stringbuf::_state_destroyed;
        return (storage_initialize_error);
    }
    this->_initialized_state = ft_stringbuf::_state_initialized;
    assign_error = this->_storage.assign(string.c_str(), string.size());
    if (assign_error != FT_ERR_SUCCESS)
    {
        (void)this->_storage.destroy();
        this->_initialized_state = ft_stringbuf::_state_destroyed;
        return (assign_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_stringbuf::destroy() noexcept
{
    int clear_error;
    int storage_destroy_error;
    int mutex_destroy_error;

    if (this->_initialized_state != ft_stringbuf::_state_initialized)
    {
        ft_stringbuf::abort_lifecycle_error("ft_stringbuf::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    clear_error = this->_storage.clear();
    storage_destroy_error = this->_storage.destroy();
    this->_position = 0;
    mutex_destroy_error = FT_ERR_SUCCESS;
    if (this->_mutex != ft_nullptr)
    {
        mutex_destroy_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    this->_initialized_state = ft_stringbuf::_state_destroyed;
    if (clear_error != FT_ERR_SUCCESS)
        return (clear_error);
    if (storage_destroy_error != FT_ERR_SUCCESS)
        return (storage_destroy_error);
    if (mutex_destroy_error != FT_ERR_SUCCESS)
        return (mutex_destroy_error);
    return (FT_ERR_SUCCESS);
}

ssize_t ft_stringbuf::read(char *buffer, std::size_t count) noexcept
{
    std::size_t index;
    int unlock_error;

    this->abort_if_not_initialized("ft_stringbuf::read");
    if (buffer == ft_nullptr)
        return (-1);
    if (count == 0)
        return (0);
    if (this->lock_mutex() != FT_ERR_SUCCESS)
        return (-1);
    index = 0;
    while (index < count && this->_position < this->_storage.size())
    {
        const char *character_pointer = this->_storage.at(this->_position);
        if (character_pointer == ft_nullptr)
            break ;
        buffer[index] = *character_pointer;
        index++;
        this->_position++;
    }
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (static_cast<ssize_t>(index));
}

bool ft_stringbuf::is_valid() const noexcept
{
    this->abort_if_not_initialized("ft_stringbuf::is_valid");
    return (true);
}

int ft_stringbuf::str(ft_string &value) const noexcept
{
    int lock_error;
    int assign_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_stringbuf::str");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    assign_error = value.assign(this->_storage.c_str() + this->_position,
            this->_storage.size() - this->_position);
    unlock_error = this->unlock_mutex();
    if (assign_error != FT_ERR_SUCCESS)
        return (assign_error);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int ft_stringbuf::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_stringbuf::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_stringbuf::disable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_stringbuf::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_stringbuf::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("ft_stringbuf::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_stringbuf::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
