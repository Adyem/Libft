#include "class_ofstream.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Template/move.hpp"
#include <fcntl.h>
#include "class_nullptr.hpp"

int ft_ofstream::lock_mutex(void) const noexcept
{
    return (pt_recursive_mutex_lock_if_valid(this->_mutex));
}

int ft_ofstream::unlock_mutex(void) const noexcept
{
    return (pt_recursive_mutex_unlock_if_valid(this->_mutex));
}

int ft_ofstream::prepare_thread_safety(void) noexcept
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

void ft_ofstream::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_ofstream::enable_thread_safety(void) noexcept
{
    return (this->prepare_thread_safety());
}

void ft_ofstream::disable_thread_safety(void) noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_ofstream::is_thread_safe_enabled(void) const noexcept
{
    return (this->_mutex != ft_nullptr);
}

ft_ofstream::ft_ofstream() noexcept
    : _file()
    , _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_ofstream::~ft_ofstream() noexcept
{
    this->disable_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_ofstream::open(const char *filename) noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (1);
    }
    int operation_error = FT_ERR_SUCCESSS;

    if (filename == ft_nullptr)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
    }
    else if (this->_file.open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644) != 0)
    {
        operation_error = this->_file.get_error();
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_global_error_stack_push(operation_error);
    return (operation_error != FT_ERR_SUCCESSS ? 1 : 0);
}

ssize_t ft_ofstream::write(const char *string) noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    int operation_error = FT_ERR_SUCCESSS;
    ssize_t result = -1;

    if (string == ft_nullptr)
    {
        operation_error = FT_ERR_INVALID_ARGUMENT;
    }
    else
    {
        result = this->_file.write(string);
        if (result < 0)
            operation_error = this->_file.get_error();
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_global_error_stack_push(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (-1);
    return (result);
}

void ft_ofstream::close() noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    int previous_fd = this->_file.get_fd();
    this->_file.close();
    int current_fd = this->_file.get_fd();
    int file_error = this->_file.get_error();
    int operation_error = FT_ERR_SUCCESSS;

    if (previous_fd >= 0 && current_fd == previous_fd && file_error != FT_ERR_SUCCESSS)
        operation_error = file_error;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    ft_global_error_stack_push(operation_error);
    return ;
}
