#include "thread.hpp"
#include <cerrno>

void *ft_thread::start_routine(void *data)
{
    start_data *start;
    start = static_cast<start_data *>(data);
    if (start == ft_nullptr)
        return (ft_nullptr);
    start->function();
    ft_thread::release_start_data(start, false);
    return (ft_nullptr);
}

void ft_thread::release_start_data(start_data *data, bool owner_release)
{
    int previous_count;
    bool owner_finalized;

    if (data == ft_nullptr)
        return ;
    if (owner_release)
        data->owner_finalized.store(true);
    previous_count = data->reference_count.fetch_sub(1);
    if (previous_count <= 0)
    {
        return ;
    }
    if (previous_count == 1)
    {
        owner_finalized = data->owner_finalized.load();
        if (!owner_finalized)
        {
            data->reference_count.fetch_add(1);
            return ;
        }
        delete data;
    }
    return ;
}

void ft_thread::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

ft_thread::ft_thread()
    : _thread(), _joinable(false), _error_code(ER_SUCCESS), _start_data(ft_nullptr)
{
    return ;
}

ft_thread::~ft_thread()
{
    if (this->_joinable)
        this->detach();
    else if (this->_start_data != ft_nullptr)
    {
        start_data *data;

        data = this->_start_data;
        this->_start_data = ft_nullptr;
        ft_thread::release_start_data(data, true);
    }
    return ;
}

ft_thread::ft_thread(ft_thread &&other)
    : _thread(other._thread), _joinable(other._joinable),
      _error_code(other._error_code), _start_data(other._start_data)
{
    other._joinable = false;
    other._error_code = ER_SUCCESS;
    other._start_data = ft_nullptr;
    return ;
}

ft_thread &ft_thread::operator=(ft_thread &&other)
{
    if (this != &other)
    {
        if (this->_joinable)
            this->detach();
        else if (this->_start_data != ft_nullptr)
        {
            start_data *data;

            data = this->_start_data;
            this->_start_data = ft_nullptr;
            ft_thread::release_start_data(data, true);
        }
        this->_thread = other._thread;
        this->_joinable = other._joinable;
        this->_error_code = other._error_code;
        this->_start_data = other._start_data;
        other._joinable = false;
        other._error_code = ER_SUCCESS;
        other._start_data = ft_nullptr;
    }
    return (*this);
}

bool ft_thread::joinable() const
{
    return (this->_joinable);
}

void ft_thread::join()
{
    if (!this->_joinable)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    if (pt_thread_join(this->_thread, ft_nullptr) != 0)
    {
        int join_error;

        join_error = ft_errno;
        this->set_error(join_error);
        if (join_error == ESRCH || join_error == EINVAL)
        {
            start_data *data;

            data = this->_start_data;
            this->_start_data = ft_nullptr;
            this->_joinable = false;
            ft_thread::release_start_data(data, true);
        }
        return ;
    }
    start_data *data;

    data = this->_start_data;
    this->_start_data = ft_nullptr;
    this->_joinable = false;
    ft_thread::release_start_data(data, true);
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_thread::detach()
{
    if (!this->_joinable)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    if (pt_thread_detach(this->_thread) != 0)
    {
        int detach_error;

        detach_error = ft_errno;
        this->set_error(detach_error);
        if (detach_error == ESRCH || detach_error == EINVAL)
        {
            start_data *data;

            data = this->_start_data;
            this->_start_data = ft_nullptr;
            this->_joinable = false;
            ft_thread::release_start_data(data, true);
        }
        return ;
    }
    start_data *data;

    data = this->_start_data;
    this->_start_data = ft_nullptr;
    this->_joinable = false;
    ft_thread::release_start_data(data, true);
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_thread::get_error() const
{
    return (this->_error_code);
}

const char *ft_thread::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
