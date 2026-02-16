#include "yaml.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

void yaml_value::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "yaml_value lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void yaml_value::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_state == yaml_value::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

yaml_value::yaml_value() noexcept
{
    this->_state = yaml_value::_state_uninitialized;
    this->_mutex = ft_nullptr;
    this->_type = YAML_SCALAR;
    this->_scalar = "";
    return ;
}

yaml_value::~yaml_value() noexcept
{
    if (this->_state == yaml_value::_state_uninitialized)
        this->abort_lifecycle_error("yaml_value::~yaml_value",
            "destructor called while object is uninitialized");
    if (this->_state == yaml_value::_state_initialized)
        (void)this->destroy();
    return ;
}

int yaml_value::initialize() noexcept
{
    int map_error;

    if (this->_state == yaml_value::_state_initialized)
    {
        this->abort_lifecycle_error("yaml_value::initialize",
            "initialize called while already initialized");
    }
    this->_type = YAML_SCALAR;
    this->_scalar = "";
    this->_list.clear();
    this->_map_keys.clear();
    map_error = this->_map.initialize();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->_state = yaml_value::_state_destroyed;
        return (map_error);
    }
    this->_state = yaml_value::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int yaml_value::destroy() noexcept
{
    bool lock_acquired;
    int lock_error;

    if (this->_state != yaml_value::_state_initialized)
    {
        this->abort_lifecycle_error("yaml_value::destroy",
            "destroy called while object is not initialized");
    }
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error == FT_ERR_SUCCESS)
    {
        if (this->_type == YAML_LIST)
        {
            size_t list_index;
            size_t list_size;

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
            size_t key_index;
            size_t key_count;

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
    (void)this->unlock(lock_acquired);
    this->_list.clear();
    this->_map_keys.clear();
    (void)this->_map.destroy();
    (void)this->disable_thread_safety();
    this->_state = yaml_value::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

void yaml_value::set_type(yaml_type type) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("yaml_value::set_type");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = type;
    (void)this->unlock(lock_acquired);
    return ;
}

yaml_type yaml_value::get_type() const noexcept
{
    bool lock_acquired;
    int lock_error;
    yaml_type value_type;

    this->abort_if_not_initialized("yaml_value::get_type");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_type);
    value_type = this->_type;
    (void)this->unlock(lock_acquired);
    return (value_type);
}

void yaml_value::set_scalar(const ft_string &value) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("yaml_value::set_scalar");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = YAML_SCALAR;
    this->_scalar = value;
    (void)this->unlock(lock_acquired);
    return ;
}

const ft_string &yaml_value::get_scalar() const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_string *scalar_pointer;

    this->abort_if_not_initialized("yaml_value::get_scalar");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_scalar);
    scalar_pointer = &this->_scalar;
    (void)this->unlock(lock_acquired);
    return (*scalar_pointer);
}

void yaml_value::add_list_item(yaml_value *item) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("yaml_value::add_list_item");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = YAML_LIST;
    this->_list.push_back(item);
    (void)this->unlock(lock_acquired);
    return ;
}

const ft_vector<yaml_value*> &yaml_value::get_list() const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_vector<yaml_value*> *list_pointer;

    this->abort_if_not_initialized("yaml_value::get_list");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_list);
    list_pointer = &this->_list;
    (void)this->unlock(lock_acquired);
    return (*list_pointer);
}

void yaml_value::add_map_item(const ft_string &key, yaml_value *value) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("yaml_value::add_map_item");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_type = YAML_MAP;
    this->_map.insert(key, value);
    this->_map_keys.push_back(key);
    (void)this->unlock(lock_acquired);
    return ;
}

const ft_map<ft_string, yaml_value*> &yaml_value::get_map() const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_map<ft_string, yaml_value*> *map_pointer;

    this->abort_if_not_initialized("yaml_value::get_map");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_map);
    map_pointer = &this->_map;
    (void)this->unlock(lock_acquired);
    return (*map_pointer);
}

const ft_vector<ft_string> &yaml_value::get_map_keys() const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_vector<ft_string> *map_keys_pointer;

    this->abort_if_not_initialized("yaml_value::get_map_keys");
    lock_acquired = false;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_map_keys);
    map_keys_pointer = &this->_map_keys;
    (void)this->unlock(lock_acquired);
    return (*map_keys_pointer);
}

int yaml_value::enable_thread_safety() noexcept
{
    void *memory_pointer;
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("yaml_value::enable_thread_safety");
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

int yaml_value::disable_thread_safety() noexcept
{
    int destroy_error;

    this->abort_if_not_initialized("yaml_value::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    this->_mutex->~pt_recursive_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool yaml_value::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("yaml_value::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int yaml_value::lock(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->is_thread_safe() == false)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int yaml_value::unlock(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->is_thread_safe() == false)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *yaml_value::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
