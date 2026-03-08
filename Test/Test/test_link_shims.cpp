#include "../test_internal.hpp"
#include "../../Networking/http2_client.hpp"
#include "../../PThread/condition.hpp"
#include "../../PThread/mutex.hpp"

#if NETWORKING_HAS_OPENSSL
http2_header_field::http2_header_field(const http2_header_field &other) noexcept
    : _initialised_state(_state_uninitialised), _name(), _value(), _mutex(ft_nullptr)
{
    ft_string name_value;
    ft_string value_value;

    (void)this->initialize();
    if (other.copy_name(name_value) && other.copy_value(value_value))
        (void)this->assign(name_value, value_value);
    return ;
}

http2_header_field::http2_header_field(http2_header_field &&other) noexcept
    : _initialised_state(_state_uninitialised), _name(), _value(), _mutex(ft_nullptr)
{
    ft_string name_value;
    ft_string value_value;

    (void)this->initialize();
    if (other.copy_name(name_value) && other.copy_value(value_value))
    {
        (void)this->assign(name_value, value_value);
        other.clear();
    }
    return ;
}

http2_header_field &http2_header_field::operator=(const http2_header_field &other) noexcept
{
    ft_string name_value;
    ft_string value_value;

    if (this == &other)
        return (*this);
    if (this->_initialised_state != _state_initialised)
        (void)this->initialize();
    if (other.copy_name(name_value) && other.copy_value(value_value))
        (void)this->assign(name_value, value_value);
    return (*this);
}

http2_header_field &http2_header_field::operator=(http2_header_field &&other) noexcept
{
    ft_string name_value;
    ft_string value_value;

    if (this == &other)
        return (*this);
    if (this->_initialised_state != _state_initialised)
        (void)this->initialize();
    if (other.copy_name(name_value) && other.copy_value(value_value))
    {
        (void)this->assign(name_value, value_value);
        other.clear();
    }
    return (*this);
}
#endif

int pt_condition_variable::wait(pt_mutex &mutex)
{
    struct timespec wait_duration;

    wait_duration.tv_sec = 3600;
    wait_duration.tv_nsec = 0;
    return (this->wait_for(mutex, wait_duration));
}
