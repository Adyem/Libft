#include "thread.hpp"
#include <cerrno>

ft_thread::start_payload::start_payload()
    : function()
{
    return ;
}

ft_thread::start_payload::~start_payload()
{
    return ;
}

void *ft_thread::start_routine(void *data)
{
    std::shared_ptr<start_payload> *payload_capsule;
    std::shared_ptr<start_payload> payload;

    payload_capsule = static_cast<std::shared_ptr<start_payload> *>(data);
    if (payload_capsule == ft_nullptr)
        return (ft_nullptr);
    payload = *payload_capsule;
    delete payload_capsule;
    if (!payload)
        return (ft_nullptr);
    payload->function();
    return (ft_nullptr);
}

void ft_thread::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

ft_thread::ft_thread()
    : _thread(), _joinable(false), _error_code(ER_SUCCESS), _start_payload()
{
    return ;
}

ft_thread::~ft_thread()
{
    if (this->_joinable)
        this->detach();
    else
        this->_start_payload.reset();
    return ;
}

ft_thread::ft_thread(ft_thread &&other)
    : _thread(other._thread), _joinable(other._joinable),
      _error_code(other._error_code), _start_payload(std::move(other._start_payload))
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
            this->detach();
        this->_thread = other._thread;
        this->_joinable = other._joinable;
        this->_error_code = other._error_code;
        this->_start_payload = std::move(other._start_payload);
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
        this->_start_payload.reset();
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
            this->_joinable = false;
            this->_start_payload.reset();
        }
        return ;
    }
    this->_joinable = false;
    this->_start_payload.reset();
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_thread::detach()
{
    if (!this->_joinable)
    {
        this->_start_payload.reset();
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
            this->_joinable = false;
            this->_start_payload.reset();
        }
        return ;
    }
    this->_joinable = false;
    this->_start_payload.reset();
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
