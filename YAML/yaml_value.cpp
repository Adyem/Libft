#include "yaml.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

static int yaml_value_pop_string_error(const ft_string &string_value) noexcept
{
    unsigned long long operation_id;

    operation_id = string_value.last_operation_id();
    if (operation_id == 0)
        return (FT_ERR_SUCCESSS);
    return (string_value.pop_operation_error(operation_id));
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
        ft_global_error_stack_push_entry(unlock_error);
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
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    this->_type = YAML_SCALAR;
    this->_scalar = "";
    {
        int scalar_error = yaml_value_pop_string_error(this->_scalar);

        if (scalar_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push_entry(scalar_error);
            return ;
        }
    }
    int prepare_error;

    prepare_error = this->prepare_thread_safety();
    if (prepare_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push_entry(prepare_error);
        return ;
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
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
                    ft_global_error_stack_push_entry(this->_list.get_error());
                else
                {
                    while (list_index < list_size)
                    {
                        yaml_value *child;

                        child = this->_list[list_index];
                        if (this->_list.get_error() != FT_ERR_SUCCESSS)
                        {
                            ft_global_error_stack_push_entry(this->_list.get_error());
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
                    ft_global_error_stack_push_entry(this->_map_keys.get_error());
                else
                {
                    while (key_index < key_count)
                    {
                        const ft_string &key = this->_map_keys[key_index];
                        yaml_value *child;

                        if (this->_map_keys.get_error() != FT_ERR_SUCCESSS)
                        {
                            ft_global_error_stack_push_entry(this->_map_keys.get_error());
                            break ;
                        }
                        child = this->_map.at(key);
                        if (this->_map.last_operation_error() != FT_ERR_SUCCESSS)
                        {
                            ft_global_error_stack_push_entry(this->_map.last_operation_error());
                            break ;
                        }
                        delete child;
                        key_index += 1;
                    }
                }
            }
        }
        else
            ft_global_error_stack_push_entry(guard.get_status());
    }
    this->teardown_thread_safety();
    return ;
}


void yaml_value::set_type(yaml_type type) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = type;
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return ;
}

yaml_type yaml_value::get_type() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        ft_global_error_stack_push_entry(guard.get_status());
        return (this->_type);
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return (this->_type);
}

void yaml_value::set_scalar(const ft_string &value) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = YAML_SCALAR;
    this->_scalar = value;
    {
        int scalar_error = yaml_value_pop_string_error(this->_scalar);

        if (scalar_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push_entry(scalar_error);
            return ;
        }
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return ;
}

const ft_string &yaml_value::get_scalar() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        ft_global_error_stack_push_entry(guard.get_status());
        return (this->_scalar);
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push_entry(this->_list.get_error());
        return ;
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return ;
}

const ft_vector<yaml_value*> &yaml_value::get_list() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        ft_global_error_stack_push_entry(guard.get_status());
        return (this->_list);
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return (this->_list);
}

void yaml_value::add_map_item(const ft_string &key, yaml_value *value) noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return ;
    this->_type = YAML_MAP;
    this->_map.insert(key, value);
    if (this->_map.last_operation_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push_entry(this->_map.last_operation_error());
        return ;
    }
    this->_map_keys.push_back(key);
    if (this->_map_keys.get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push_entry(this->_map_keys.get_error());
        return ;
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return ;
}

const ft_map<ft_string, yaml_value*> &yaml_value::get_map() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        ft_global_error_stack_push_entry(guard.get_status());
        return (this->_map);
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return (this->_map);
}

const ft_vector<ft_string> &yaml_value::get_map_keys() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        ft_global_error_stack_push_entry(guard.get_status());
        return (this->_map_keys);
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
    return (this->_map_keys);
}

bool yaml_value::is_thread_safe_enabled() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
    {
        ft_global_error_stack_push_entry(guard.get_status());
        return (false);
    }
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
        return (false);
    }
    ft_global_error_stack_push_entry(FT_ERR_SUCCESSS);
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
    {
        int mutex_error = ft_global_error_stack_last_error();

        if (mutex_error != FT_ERR_SUCCESSS)
        {
            mutex_pointer->~pt_mutex();
            cma_free(memory_pointer);
            return (mutex_error);
        }
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
    {
        mutex_error = ft_global_error_stack_last_error();
    }
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
    {
        int mutex_error = ft_global_error_stack_last_error();

        if (mutex_error != FT_ERR_SUCCESSS)
            return (mutex_error);
    }
    return (FT_ERR_SUCCESSS);
}

pt_mutex *yaml_value::mutex_handle() const noexcept
{
    return (this->_mutex);
}
