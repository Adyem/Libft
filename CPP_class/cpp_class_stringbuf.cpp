#include "class_stringbuf.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread_internal.hpp"

int ft_stringbuf::lock_mutex() const noexcept
{
    return (pt_recursive_mutex_lock_if_valid(this->_mutex));
}

int ft_stringbuf::unlock_mutex() const noexcept
{
    return (pt_recursive_mutex_unlock_if_valid(this->_mutex));
}

int ft_stringbuf::prepare_thread_safety() noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_stringbuf::teardown_thread_safety() noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

int ft_stringbuf::enable_thread_safety() noexcept
{
    return (this->prepare_thread_safety());
}

void ft_stringbuf::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_stringbuf::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

static void ft_stringbuf_push_string_error() noexcept
{
    int string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESS)
        ft_global_error_stack_push(string_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

ft_stringbuf::ft_stringbuf(const ft_string &string) noexcept
    : _storage(string)
    , _position(0)
    , _mutex(ft_nullptr)
{
    ft_stringbuf_push_string_error();
    return ;
}

ft_stringbuf::ft_stringbuf(const ft_stringbuf &other) noexcept
    : _storage(other._storage)
    , _position(other._position)
    , _mutex(ft_nullptr)
{
    ft_stringbuf_push_string_error();
    return ;
}

ft_stringbuf &ft_stringbuf::operator=(const ft_stringbuf &other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    this->_storage = other._storage;
    this->_position = other._position;
    ft_stringbuf_push_string_error();
    return (*this);
}

ft_stringbuf::ft_stringbuf(ft_stringbuf &&other) noexcept
    : _storage(ft_move(other._storage))
    , _position(other._position)
    , _mutex(ft_nullptr)
{
    other._position = 0;
    ft_stringbuf_push_string_error();
    return ;
}

ft_stringbuf &ft_stringbuf::operator=(ft_stringbuf &&other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    this->_storage = ft_move(other._storage);
    this->_position = other._position;
    other._position = 0;
    ft_stringbuf_push_string_error();
    return (*this);
}

ft_stringbuf::~ft_stringbuf() noexcept
{
    this->teardown_thread_safety();
    return ;
}

std::size_t ft_stringbuf::read(char *buffer, std::size_t count)
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    std::size_t index = 0;
    int operation_error = FT_ERR_SUCCESS;
    if (buffer == ft_nullptr)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
    }
    else
    {
        while (index < count && this->_position < this->_storage.size())
        {
            const char *current = this->_storage.at(this->_position);
            if (current == ft_nullptr)
            {
                operation_error = FT_ERR_INVALID_ARGUMENT;
                break ;
            }
            buffer[index] = *current;
            index++;
            this->_position++;
        }
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return (0);
    }
    ft_global_error_stack_push(operation_error);
    return (index);
}

bool ft_stringbuf::is_valid() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    int last_error = ft_global_error_stack_peek_last_error();
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return (false);
    }
    ft_global_error_stack_push(last_error);
    return (last_error == FT_ERR_SUCCESS);
}

ft_string ft_stringbuf::str() const
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_string(lock_error));
    }
    const char *start = this->_storage.c_str();
    ft_string result(start + this->_position);
    int string_error = ft_string::last_operation_error();
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return (ft_string(unlock_error));
    }
    if (string_error != FT_ERR_SUCCESS)
        ft_global_error_stack_push(string_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_stringbuf::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
