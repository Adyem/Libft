#include "thread.hpp"

void *ft_thread::start_routine(void *data)
{
    start_data *start;

    start = static_cast<start_data *>(data);
    start->function();
    delete start;
    return (ft_nullptr);
}

void ft_thread::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

ft_thread::ft_thread()
    : _thread(), _joinable(false), _error_code(ER_SUCCESS)
{
    return ;
}

ft_thread::~ft_thread()
{
    if (this->_joinable)
        pt_thread_detach(this->_thread);
    return ;
}

ft_thread::ft_thread(ft_thread &&other)
    : _thread(other._thread), _joinable(other._joinable),
      _error_code(other._error_code)
{
    other._joinable = false;
    other._error_code = ER_SUCCESS;
    return ;
}

ft_thread &ft_thread::operator=(ft_thread &&other)
{
    if (this != &other)
    {
        if (this->_joinable)
            pt_thread_detach(this->_thread);
        this->_thread = other._thread;
        this->_joinable = other._joinable;
        this->_error_code = other._error_code;
        other._joinable = false;
        other._error_code = ER_SUCCESS;
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
        this->_joinable = false;
        return ;
    }
    this->_joinable = false;
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
        this->_joinable = false;
        return ;
    }
    this->_joinable = false;
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
