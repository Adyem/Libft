#include "thread.hpp"

void *ft_thread::start_routine(void *data)
{
    start_data *start;
    void *mutex_address;
    void *function_address;

    start = static_cast<start_data *>(data);
    if (start == ft_nullptr)
        return (ft_nullptr);
    mutex_address = start->function.get_mutex_address_debug();
    function_address = reinterpret_cast<void *>(&start->function);
    cma_debug_log_start_data_event("start_data_thread_begin", start,
        mutex_address, function_address);
    start->function();
    cma_debug_log_start_data_event("start_data_thread_finished", start,
        mutex_address, function_address);
    ft_thread::release_start_data(start);
    return (ft_nullptr);
}

void ft_thread::release_start_data(start_data *data)
{
    int previous_count;
    void *mutex_address;
    void *function_address;

    if (data == ft_nullptr)
        return ;
    mutex_address = data->function.get_mutex_address_debug();
    function_address = reinterpret_cast<void *>(&data->function);
    previous_count = data->reference_count.fetch_sub(1);
    cma_debug_log_start_data_event("start_data_release_reference", data,
        mutex_address, function_address);
    if (previous_count <= 0)
    {
        cma_debug_log_start_data_event("start_data_release_underflow", data,
            mutex_address, function_address);
        return ;
    }
    if (previous_count == 1)
    {
        cma_debug_log_start_data_event("start_data_destroy", data,
            mutex_address, function_address);
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
        ft_thread::release_start_data(this->_start_data);
        this->_start_data = ft_nullptr;
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
            ft_thread::release_start_data(this->_start_data);
            this->_start_data = ft_nullptr;
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
        this->set_error(ft_errno);
        return ;
    }
    this->_joinable = false;
    ft_thread::release_start_data(this->_start_data);
    this->_start_data = ft_nullptr;
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
        this->set_error(ft_errno);
        return ;
    }
    this->_joinable = false;
    ft_thread::release_start_data(this->_start_data);
    this->_start_data = ft_nullptr;
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
