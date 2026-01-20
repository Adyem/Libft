#include "yaml.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

thread_local ft_operation_error_stack yaml_value::_operation_errors = {{}, {}, 0};

void yaml_value::record_operation_error_unlocked(int error_code) noexcept
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(yaml_value::_operation_errors,
            error_code, operation_id);
    return ;
}

yaml_value::thread_guard::thread_guard(const yaml_value *value) noexcept
    : _value(value), _lock_acquired(false), _status(0)
{
    if (!this->_value)
        return ;
    this->_status = this->_value->lock(&this->_lock_acquired);
    return ;
}

yaml_value::thread_guard::~thread_guard() noexcept
{
    if (!this->_value)
        return ;
    int unlock_error;

    unlock_error = this->_value->unlock(this->_lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
        this->_value->set_error(unlock_error);
    return ;
}

int yaml_value::thread_guard::get_status() const noexcept
{
    return (this->_status);
}

bool yaml_value::thread_guard::lock_acquired() const noexcept
{
    return (this->_lock_acquired);
}

yaml_value::yaml_value() noexcept
{
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    this->set_error(FT_ERR_SUCCESSS);
    this->_type = YAML_SCALAR;
    this->_scalar = "";
    if (this->_scalar.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_scalar.get_error());
        return ;
    }
    int prepare_error;

    prepare_error = this->prepare_thread_safety();
    if (prepare_error != FT_ERR_SUCCESSS)
    {
        this->set_error(prepare_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

yaml_value::~yaml_value() noexcept
{
    {
        thread_guard guard(this);

        if (guard.get_status() == 0)
        {
            if (this->_type == YAML_LIST)
            {
                size_t list_index;
                size_t list_size;

                list_index = 0;
                list_size = this->_list.size();
                if (this->_list.get_error() != FT_ERR_SUCCESSS)
                    this->set_error(this->_list.get_error());
                else
                {
                    while (list_index < list_size)
                    {
                        yaml_value *child;

                        child = this->_list[list_index];
                        if (this->_list.get_error() != FT_ERR_SUCCESSS)
                        {
                            this->set_error(this->_list.get_error());
                            break ;
                        }
                        delete child;
                        list_index += 1;
                    }
                }
            }
            else if (this->_type == YAML_MAP)
            {
                size_t key_index;
                size_t key_count;

                key_index = 0;
                key_count = this->_map_keys.size();
                if (this->_map_keys.get_error() != FT_ERR_SUCCESSS)
                    this->set_error(this->_map_keys.get_error());
                else
                {
                    while (key_index < key_count)
                    {
                        const ft_string &key = this->_map_keys[key_index];
                        yaml_value *child;

                        if (this->_map_keys.get_error() != FT_ERR_SUCCESSS)
                        {
                            this->set_error(this->_map_keys.get_error());
                            break ;
                        }
                        child = this->_map.at(key);
                        if (this->_map.get_error() != FT_ERR_SUCCESSS)
                        {
                            this->set_error(this->_map.get_error());
                            break ;
                        }
                        delete child;
                        key_index += 1;
                    }
                }
            }
        }
        else
            this->set_error(guard.get_status());
    }
    this->teardown_thread_safety();
    return ;
}

void yaml_value::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    yaml_value::record_operation_error_unlocked(error_code);
    return ;
}

int yaml_value::get_error() const noexcept
{
    int error_code;

    error_code = this->_error_code;
    return (error_code);
}

const char *yaml_value::get_error_str() const noexcept
{
    int error_code;

    error_code = this->_error_code;
    return (ft_strerror(error_code));
}

void yaml_value::set_type(yaml_type type) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = type;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

yaml_type yaml_value::get_type() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<yaml_value *>(this)->set_error(guard.get_status());
        return (this->_type);
    }
    const_cast<yaml_value *>(this)->set_error(FT_ERR_SUCCESSS);
    return (this->_type);
}

void yaml_value::set_scalar(const ft_string &value) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = YAML_SCALAR;
    this->_scalar = value;
    if (this->_scalar.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_scalar.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

const ft_string &yaml_value::get_scalar() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<yaml_value *>(this)->set_error(guard.get_status());
        return (this->_scalar);
    }
    const_cast<yaml_value *>(this)->set_error(FT_ERR_SUCCESSS);
    return (this->_scalar);
}

void yaml_value::add_list_item(yaml_value *item) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = YAML_LIST;
    this->_list.push_back(item);
    if (this->_list.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_list.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

const ft_vector<yaml_value*> &yaml_value::get_list() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<yaml_value *>(this)->set_error(guard.get_status());
        return (this->_list);
    }
    const_cast<yaml_value *>(this)->set_error(FT_ERR_SUCCESSS);
    return (this->_list);
}

void yaml_value::add_map_item(const ft_string &key, yaml_value *value) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = YAML_MAP;
    this->_map.insert(key, value);
    if (this->_map.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_map.get_error());
        return ;
    }
    this->_map_keys.push_back(key);
    if (this->_map_keys.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_map_keys.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

const ft_map<ft_string, yaml_value*> &yaml_value::get_map() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<yaml_value *>(this)->set_error(guard.get_status());
        return (this->_map);
    }
    const_cast<yaml_value *>(this)->set_error(FT_ERR_SUCCESSS);
    return (this->_map);
}

const ft_vector<ft_string> &yaml_value::get_map_keys() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<yaml_value *>(this)->set_error(guard.get_status());
        return (this->_map_keys);
    }
    const_cast<yaml_value *>(this)->set_error(FT_ERR_SUCCESSS);
    return (this->_map_keys);
}

bool yaml_value::is_thread_safe_enabled() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        const_cast<yaml_value *>(this)->set_error(guard.get_status());
        return (false);
    }
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        const_cast<yaml_value *>(this)->set_error(FT_ERR_SUCCESSS);
        return (false);
    }
    const_cast<yaml_value *>(this)->set_error(FT_ERR_SUCCESSS);
    return (true);
}

int yaml_value::prepare_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    void     *memory_pointer;

    if (this->_thread_safe_enabled && this->_mutex)
    {
        return (FT_ERR_SUCCESSS);
    }
    memory_pointer = cma_malloc(sizeof(pt_mutex));
    if (!memory_pointer)
    {
        return (FT_ERR_NO_MEMORY);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory_pointer);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    return (FT_ERR_SUCCESSS);
}

void yaml_value::teardown_thread_safety() noexcept
{
    if (!this->_mutex)
    {
        this->_thread_safe_enabled = false;
        return ;
    }
    this->_mutex->~pt_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    return ;
}

int yaml_value::lock(bool *lock_acquired) const noexcept
{
    int mutex_error;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        return (FT_ERR_SUCCESSS);
    }
    this->_mutex->lock(THREAD_ID);
    mutex_error = this->_mutex->get_error();
    if (mutex_error == FT_ERR_SUCCESSS)
    {
        if (lock_acquired)
            *lock_acquired = true;
        return (FT_ERR_SUCCESSS);
    }
    if (mutex_error == FT_ERR_MUTEX_ALREADY_LOCKED)
    {
        return (FT_ERR_SUCCESSS);
    }
    return (mutex_error);
}

int yaml_value::unlock(bool lock_acquired) const noexcept
{
    if (!lock_acquired || !this->_thread_safe_enabled || !this->_mutex)
    {
        return (FT_ERR_SUCCESSS);
    }
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        return (mutex_error);
    }
    return (FT_ERR_SUCCESSS);
}

pt_mutex *yaml_value::mutex_handle() const noexcept
{
    return (this->_mutex);
}
