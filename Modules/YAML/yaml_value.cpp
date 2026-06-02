#include "yaml.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/map.hpp"
#include "../Template/pair.hpp"
#include "../Template/vector.hpp"

yaml_value::yaml_value() noexcept
{
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    this->_mutex = ft_nullptr;
    this->_type = YAML_SCALAR;
    return ;
}

yaml_value::~yaml_value() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t yaml_value::initialize() noexcept
{
    int32_t scalar_error;
    int32_t list_error;
    int32_t map_error;
    int32_t map_keys_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state,
            "yaml_value::initialize", "initialize called while already initialised");
    scalar_error = this->_scalar.initialize();
    if (scalar_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (scalar_error);
    }
    list_error = this->_list.initialize();
    if (list_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (list_error);
    }
    this->_type = YAML_SCALAR;
    this->_scalar = "";
    map_error = this->_map.initialize();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (map_error);
    }
    map_keys_error = this->_map_keys.initialize();
    if (map_keys_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (map_keys_error);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t yaml_value::initialize(const yaml_value &other) noexcept
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "yaml_value::initialize(const yaml_value &) source",
            "source is not initialised");
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_type = other._type;
    this->_scalar = other._scalar;
    return (FT_ERR_SUCCESS);
}

int32_t yaml_value::initialize(yaml_value &&other) noexcept
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "yaml_value::initialize(yaml_value &&) source",
            "source is not initialised");
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_type = other._type;
    this->_scalar = other._scalar;
    other._type = YAML_SCALAR;
    other._scalar = "";
    return (FT_ERR_SUCCESS);
}

int32_t yaml_value::destroy() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error == FT_ERR_SUCCESS)
    {
        if (this->_type == YAML_LIST)
        {
            ft_size_t list_index;
            ft_size_t list_size;

            list_index = 0;
            list_size = this->_list.size();
            while (list_index < list_size)
            {
                yaml_value *child_value;

                child_value = this->_list[list_index];
                delete child_value;
                list_index += 1;
            }
        }
        else if (this->_type == YAML_MAP)
        {
            ft_size_t key_index;
            ft_size_t key_count;

            key_index = 0;
            key_count = this->_map_keys.size();
            while (key_index < key_count)
            {
                const ft_string &key = this->_map_keys[key_index];
                yaml_value *child_value;

                child_value = this->_map.at(key);
                delete child_value;
                key_index += 1;
            }
        }
    }
    this->unlock(lock_acquired);
    this->_list.clear();
    this->_map_keys.clear();
    (void)this->_map.destroy();
    (void)this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

uint32_t yaml_value::move(yaml_value &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(static_cast<yaml_value &&>(other))));
}

void yaml_value::set_type(yaml_type type) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::set_type");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = type;
    this->unlock(lock_acquired);
    return ;
}

yaml_type yaml_value::get_type() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    yaml_type value_type;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::get_type");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_type);
    value_type = this->_type;
    this->unlock(lock_acquired);
    return (value_type);
}

void yaml_value::set_scalar(const ft_string &value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::set_scalar");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = YAML_SCALAR;
    this->_scalar = value;
    this->unlock(lock_acquired);
    return ;
}

const ft_string &yaml_value::get_scalar() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const ft_string *scalar_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::get_scalar");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_scalar);
    scalar_pointer = &this->_scalar;
    this->unlock(lock_acquired);
    return (*scalar_pointer);
}

void yaml_value::add_list_item(yaml_value *item) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::add_list_item");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = YAML_LIST;
    this->_list.push_back(item);
    this->unlock(lock_acquired);
    return ;
}

const ft_vector<yaml_value*> &yaml_value::get_list() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const ft_vector<yaml_value*> *list_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::get_list");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_list);
    list_pointer = &this->_list;
    this->unlock(lock_acquired);
    return (*list_pointer);
}

void yaml_value::add_map_item(const ft_string &key, yaml_value *value) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::add_map_item");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = YAML_MAP;
    this->_map.insert(key, value);
    this->_map_keys.push_back(key);
    this->unlock(lock_acquired);
    return ;
}

const ft_map<ft_string, yaml_value*> &yaml_value::get_map() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const ft_map<ft_string, yaml_value*> *map_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::get_map");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_map);
    map_pointer = &this->_map;
    this->unlock(lock_acquired);
    return (*map_pointer);
}

const ft_vector<ft_string> &yaml_value::get_map_keys() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const ft_vector<ft_string> *map_keys_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "yaml_value::get_map_keys");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_map_keys);
    map_keys_pointer = &this->_map_keys;
    this->unlock(lock_acquired);
    return (*map_keys_pointer);
}

int32_t yaml_value::enable_thread_safety() noexcept
{
    void *memory_pointer;
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "yaml_value::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    memory_pointer = cma_malloc(sizeof(pt_recursive_mutex));
    if (memory_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_pointer = new(memory_pointer) pt_recursive_mutex();
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        mutex_pointer->~pt_recursive_mutex();
        cma_free(memory_pointer);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t yaml_value::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "yaml_value::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    this->_mutex->~pt_recursive_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool yaml_value::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t yaml_value::lock(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void yaml_value::unlock(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}
