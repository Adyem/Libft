#include "class_ofstream.hpp"
#include "../Template/move.hpp"
#include <fcntl.h>

static int ft_ofstream_capture_guard_error() noexcept
{
    int error_value;

    error_value = ft_global_error_stack_last_error();
    ft_global_error_stack_pop_newest();
    if (error_value != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(error_value);
    return (error_value);
}

ft_ofstream::ft_ofstream() noexcept
    : _file(), _mutex()
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_ofstream::~ft_ofstream() noexcept
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_ofstream::lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_recursive_mutex> local_guard(this->_mutex);

    int guard_error = ft_ofstream_capture_guard_error();
    if (guard_error != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_recursive_mutex>();
        return (guard_error);
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESSS);
}

int ft_ofstream::finalize_lock(ft_unique_lock<pt_recursive_mutex> &guard) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    return (ft_ofstream_capture_guard_error());
}

int ft_ofstream::open(const char *filename) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    int guard_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        guard_error = ft_ofstream::finalize_lock(guard);
        int report_error;
        if (guard_error != FT_ERR_SUCCESSS)
            report_error = guard_error;
        else
            report_error = lock_error;
        ft_global_error_stack_push(report_error);
        return (1);
    }
    if (filename == ft_nullptr)
    {
        guard_error = ft_ofstream::finalize_lock(guard);
        int report_error;
        if (guard_error != FT_ERR_SUCCESSS)
            report_error = guard_error;
        else
            report_error = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(report_error);
        return (1);
    }
    if (this->_file.open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644) != 0)
    {
        int file_error = this->_file.get_error();
        guard_error = ft_ofstream::finalize_lock(guard);
        int report_error;
        if (guard_error != FT_ERR_SUCCESSS)
            report_error = guard_error;
        else
            report_error = file_error;
        ft_global_error_stack_push(report_error);
        return (1);
    }
    guard_error = ft_ofstream::finalize_lock(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(guard_error);
        return (1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

ssize_t ft_ofstream::write(const char *string) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    int guard_error;
    ssize_t result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        guard_error = ft_ofstream::finalize_lock(guard);
        int report_error;
        if (guard_error != FT_ERR_SUCCESSS)
            report_error = guard_error;
        else
            report_error = lock_error;
        ft_global_error_stack_push(report_error);
        return (-1);
    }
    if (string == ft_nullptr)
    {
        guard_error = ft_ofstream::finalize_lock(guard);
        int report_error;
        if (guard_error != FT_ERR_SUCCESSS)
            report_error = guard_error;
        else
            report_error = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(report_error);
        return (-1);
    }
    result = this->_file.write(string);
    if (result < 0)
    {
        int file_error = this->_file.get_error();
        guard_error = ft_ofstream::finalize_lock(guard);
        int report_error;
        if (guard_error != FT_ERR_SUCCESSS)
            report_error = guard_error;
        else
            report_error = file_error;
        ft_global_error_stack_push(report_error);
        return (-1);
    }
    guard_error = ft_ofstream::finalize_lock(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(guard_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

void ft_ofstream::close() noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    int guard_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        guard_error = ft_ofstream::finalize_lock(guard);
        int report_error;
        if (guard_error != FT_ERR_SUCCESSS)
            report_error = guard_error;
        else
            report_error = lock_error;
        ft_global_error_stack_push(report_error);
        return ;
    }
    int previous_fd = this->_file.get_fd();
    this->_file.close();
    int current_fd = this->_file.get_fd();
    int file_error = this->_file.get_error();
    guard_error = ft_ofstream::finalize_lock(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(guard_error);
        return ;
    }
    if (previous_fd >= 0 && current_fd == previous_fd && file_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(file_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
