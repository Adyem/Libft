#include "ft_region_definition.hpp"
#include "../Template/move.hpp"

int ft_region_definition::lock_pair(const ft_region_definition &first, const ft_region_definition &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_region_definition *ordered_first;
    const ft_region_definition *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESS;
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_region_definition *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ERR_SUCCESS;
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_region_definition::ft_region_definition() noexcept
    : _region_id(0), _name(), _description(), _recommended_level(0),
    _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_region_definition::ft_region_definition(int region_id, const ft_string &name,
        const ft_string &description, int recommended_level) noexcept
    : _region_id(region_id), _name(name), _description(description),
    _recommended_level(recommended_level), _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_region_definition::~ft_region_definition() noexcept
{
    return ;
}

ft_region_definition::ft_region_definition(const ft_region_definition &other) noexcept
    : _region_id(0), _name(), _description(), _recommended_level(0),
    _error_code(FT_ERR_SUCCESS)
{
    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_region_id = other._region_id;
    this->_name = other._name;
    this->_description = other._description;
    this->_recommended_level = other._recommended_level;
    this->_error_code = other._error_code;
    return ;
}

ft_region_definition &ft_region_definition::operator=(const ft_region_definition &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    ft_errno = FT_ERR_SUCCESS;
    lock_error = ft_region_definition::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_region_id = other._region_id;
    this->_name = other._name;
    this->_description = other._description;
    this->_recommended_level = other._recommended_level;
    this->_error_code = other._error_code;
    return (*this);
}

ft_region_definition::ft_region_definition(ft_region_definition &&other) noexcept
    : _region_id(0), _name(), _description(), _recommended_level(0), _error_code(FT_ERR_SUCCESS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_region_id = other._region_id;
    this->_name = other._name;
    this->_description = other._description;
    this->_recommended_level = other._recommended_level;
    this->_error_code = other._error_code;
    other._region_id = 0;
    other._name.clear();
    other._description.clear();
    other._recommended_level = 0;
    other._error_code = FT_ERR_SUCCESS;
    return ;
}

ft_region_definition &ft_region_definition::operator=(ft_region_definition &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_region_definition::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_region_id = other._region_id;
    this->_name = other._name;
    this->_description = other._description;
    this->_recommended_level = other._recommended_level;
    this->_error_code = other._error_code;
    other._region_id = 0;
    other._name.clear();
    other._description.clear();
    other._recommended_level = 0;
    other._error_code = FT_ERR_SUCCESS;
    return (*this);
}

int ft_region_definition::get_region_id() const noexcept
{
    int region_id;

    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    region_id = this->_region_id;
    return (region_id);
}

void ft_region_definition::set_region_id(int region_id) noexcept
{
    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_region_id = region_id;
    this->_error_code = FT_ERR_SUCCESS;
    return ;
}

const ft_string &ft_region_definition::get_name() const noexcept
{
    const ft_string *name;

    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (this->_name);
    }
    name = &this->_name;
    return (*name);
}

void ft_region_definition::set_name(const ft_string &name) noexcept
{
    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_name = name;
    this->_error_code = FT_ERR_SUCCESS;
    return ;
}

const ft_string &ft_region_definition::get_description() const noexcept
{
    const ft_string *description;

    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (this->_description);
    }
    description = &this->_description;
    return (*description);
}

void ft_region_definition::set_description(const ft_string &description) noexcept
{
    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_description = description;
    this->_error_code = FT_ERR_SUCCESS;
    return ;
}

int ft_region_definition::get_recommended_level() const noexcept
{
    int recommended_level;

    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    recommended_level = this->_recommended_level;
    return (recommended_level);
}

void ft_region_definition::set_recommended_level(int recommended_level) noexcept
{
    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_recommended_level = recommended_level;
    this->_error_code = FT_ERR_SUCCESS;
    return ;
}

int ft_region_definition::get_error() const noexcept
{
    int error_code;

    ft_errno = FT_ERR_SUCCESS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_region_definition *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    return (error_code);
}

const char *ft_region_definition::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_region_definition::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
