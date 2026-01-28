#include "flag_parser.hpp"
#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/lock_error_helpers.hpp"

static int cnfg_config_lock_if_enabled(cnfg_config *config, ft_unique_lock<pt_mutex> &mutex_guard)
{
    if (!config || !config->thread_safe_enabled || !config->mutex)
        return (FT_ERR_SUCCESSS);
    mutex_guard = ft_unique_lock<pt_mutex>(*config->mutex);
    return (ft_unique_lock_pop_last_error(mutex_guard));
}

static void cnfg_config_unlock_guard(ft_unique_lock<pt_mutex> &mutex_guard)
{
    if (!mutex_guard.owns_lock())
        return ;
    mutex_guard.unlock();
    return ;
}

cnfg_flag_parser::cnfg_flag_parser()
{
    this->_short_flags = ft_nullptr;
    this->_long_flags = ft_nullptr;
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

cnfg_flag_parser::cnfg_flag_parser(int argument_count, char **argument_values)
{
    this->_short_flags = ft_nullptr;
    this->_long_flags = ft_nullptr;
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    this->set_error(FT_ERR_SUCCESSS);
    this->parse(argument_count, argument_values);
    return ;
}

cnfg_flag_parser::~cnfg_flag_parser()
{
    this->free_flags();
    return ;
}

void cnfg_flag_parser::free_flags_locked()
{
    if (this->_short_flags)
    {
        cma_free(this->_short_flags);
        this->_short_flags = ft_nullptr;
    }
    if (this->_long_flags)
    {
        size_t long_flag_index;

        long_flag_index = 0;
        while (this->_long_flags[long_flag_index])
        {
            cma_free(this->_long_flags[long_flag_index]);
            ++long_flag_index;
        }
        cma_free(this->_long_flags);
        this->_long_flags = ft_nullptr;
    }
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    return ;
}

void cnfg_flag_parser::free_flags()
{
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return ;
        }
    }
    this->free_flags_locked();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

bool cnfg_flag_parser::parse(int argument_count, char **argument_values)
{
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (false);
        }
    }
    this->free_flags_locked();
    this->set_error(FT_ERR_SUCCESSS);
    this->_short_flags = cnfg_parse_flags(argument_count, argument_values);
    if (!this->_short_flags)
    {
        int short_flag_error = ft_global_error_stack_last_error();
        ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(short_flag_error);
        this->set_error(short_flag_error);
        this->free_flags_locked();
        return (false);
    }
    this->_long_flags = cnfg_parse_long_flags(argument_count, argument_values);
    if (!this->_long_flags)
    {
        int long_flag_error = ft_global_error_stack_last_error();
        ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(long_flag_error);
        this->set_error(long_flag_error);
        this->free_flags_locked();
        return (false);
    }
    size_t short_flag_index;

    short_flag_index = 0;
    if (this->_short_flags)
    {
        while (this->_short_flags[short_flag_index])
            ++short_flag_index;
    }
    this->_short_flag_count = short_flag_index;
    size_t long_flag_index;

    long_flag_index = 0;
    if (this->_long_flags)
    {
        while (this->_long_flags[long_flag_index])
            ++long_flag_index;
    }
    this->_long_flag_count = long_flag_index;
    this->set_error(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool cnfg_flag_parser::has_short_flag(char flag)
{
    bool has_flag;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (false);
        }
    }
    if (!this->_short_flags)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (false);
    }
    has_flag = ft_strchr(this->_short_flags, flag) != ft_nullptr;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (has_flag);
}

bool cnfg_flag_parser::has_long_flag(const char *flag)
{
    size_t flag_index;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (false);
        }
    }
    if (!this->_long_flags || !flag)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (false);
    }
    flag_index = 0;
    while (this->_long_flags[flag_index])
    {
        if (ft_strcmp(this->_long_flags[flag_index], flag) == 0)
            return (true);
        ++flag_index;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (false);
}

size_t cnfg_flag_parser::get_short_flag_count()
{
    size_t short_flag_count;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (0);
        }
    }
    short_flag_count = this->_short_flag_count;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (short_flag_count);
}

size_t cnfg_flag_parser::get_long_flag_count()
{
    size_t long_flag_count;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (0);
        }
    }
    long_flag_count = this->_long_flag_count;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (long_flag_count);
}

size_t cnfg_flag_parser::get_total_flag_count()
{
    size_t total_flag_count;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (0);
        }
    }
    total_flag_count = this->_short_flag_count + this->_long_flag_count;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (total_flag_count);
}

void    cnfg_flag_parser::set_error(int error_code)
{
    this->_error_code = error_code;
    return ;
}

int     cnfg_flag_parser::get_error() const
{
    int error_code;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            const_cast<cnfg_flag_parser *>(this)->set_error(lock_error);
            return (lock_error);
        }
    }
    error_code = this->_error_code;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (error_code);
}

const char  *cnfg_flag_parser::get_error_str() const
{
    int error_code;
    const char *error_string;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    {
        int lock_error = ft_unique_lock_pop_last_error(mutex_guard);

        if (lock_error != FT_ERR_SUCCESSS)
        {
            const_cast<cnfg_flag_parser *>(this)->set_error(lock_error);
            return (ft_strerror(lock_error));
        }
    }
    error_code = this->_error_code;
    error_string = ft_strerror(error_code);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (error_string);
}

static cnfg_config *merge_configs(cnfg_config *base_config,
                                  cnfg_config *override_config)
{
    ft_unique_lock<pt_mutex> base_guard;
    ft_unique_lock<pt_mutex> override_guard;
    int lock_error;

    if (!override_config)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (base_config);
    }
    lock_error = cnfg_config_lock_if_enabled(override_config, override_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    if (base_config)
    {
        lock_error = cnfg_config_lock_if_enabled(base_config, base_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            cnfg_config_unlock_guard(override_guard);
            ft_global_error_stack_push(lock_error);
            return (ft_nullptr);
        }
    }
    size_t override_index = 0;
    while (override_index < override_config->entry_count)
    {
        cnfg_entry *override_entry = &override_config->entries[override_index];
        const char *override_section;
        const char *override_key;
        const char *override_value;
        bool override_locked;
        bool replaced = false;

        if (cnfg_entry_lock(override_entry, &override_locked) != 0)
        {
            cnfg_config_unlock_guard(base_guard);
            cnfg_config_unlock_guard(override_guard);
            return (ft_nullptr);
        }
        override_section = override_entry->section;
        override_key = override_entry->key;
        override_value = override_entry->value;
        cnfg_entry_unlock(override_entry, override_locked);
        size_t base_index = 0;
        if (base_config)
        {
            while (base_index < base_config->entry_count)
            {
                cnfg_entry *base_entry = &base_config->entries[base_index];
                bool base_locked;
                bool same_section;
                bool same_key;
                if (cnfg_entry_lock(base_entry, &base_locked) != 0)
                {
                    cnfg_config_unlock_guard(base_guard);
                    cnfg_config_unlock_guard(override_guard);
                    return (ft_nullptr);
                }
                same_section = (base_entry->section == ft_nullptr
                    && override_section == ft_nullptr)
                    || (base_entry->section && override_section
                    && ft_strcmp(base_entry->section,
                    override_section) == 0);
                same_key = (base_entry->key == ft_nullptr
                    && override_key == ft_nullptr)
                    || (base_entry->key && override_key
                    && ft_strcmp(base_entry->key,
                    override_key) == 0);
                if (same_section && same_key)
                {
                    cma_free(base_entry->value);
                    base_entry->value = ft_nullptr;
                    if (override_value)
                    {
                        base_entry->value = cma_strdup(override_value);
                        if (!base_entry->value)
                        {
                            cnfg_entry_unlock(base_entry, base_locked);
                            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                            cnfg_config_unlock_guard(base_guard);
                            cnfg_config_unlock_guard(override_guard);
                            cnfg_free(base_config);
                            return (ft_nullptr);
                        }
                    }
                    cnfg_entry_unlock(base_entry, base_locked);
                    replaced = true;
                    break;
                }
                cnfg_entry_unlock(base_entry, base_locked);
                ++base_index;
            }
        }
        if (!replaced)
        {
            if (!base_config)
            {
                base_config = cnfg_config_create();
                if (!base_config)
                {
                    cnfg_config_unlock_guard(base_guard);
                    cnfg_config_unlock_guard(override_guard);
                    return (ft_nullptr);
                }
                lock_error = cnfg_config_lock_if_enabled(base_config, base_guard);
                if (lock_error != FT_ERR_SUCCESSS)
                {
                    cnfg_config_unlock_guard(override_guard);
                    cnfg_free(base_config);
                    ft_global_error_stack_push(lock_error);
                    return (ft_nullptr);
                }
            }
            cnfg_entry *new_entries = static_cast<cnfg_entry*>(cma_realloc(
                base_config->entries, sizeof(cnfg_entry)
                * (base_config->entry_count + 1)));
            if (!new_entries)
            {
                ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                cnfg_config_unlock_guard(base_guard);
                cnfg_config_unlock_guard(override_guard);
                cnfg_free(base_config);
                return (ft_nullptr);
            }
            base_config->entries = new_entries;
            cnfg_entry *new_entry = &base_config->entries[base_config->entry_count];
            new_entry->mutex = ft_nullptr;
            new_entry->thread_safe_enabled = false;
            if (override_section)
            {
                new_entry->section = cma_strdup(override_section);
                if (!new_entry->section)
                {
                    ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                    cnfg_config_unlock_guard(base_guard);
                    cnfg_config_unlock_guard(override_guard);
                    cnfg_free(base_config);
                    return (ft_nullptr);
                }
            }
            else
                new_entry->section = ft_nullptr;
            if (override_key)
            {
                new_entry->key = cma_strdup(override_key);
                if (!new_entry->key)
                {
                    ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                    cnfg_config_unlock_guard(base_guard);
                    cnfg_config_unlock_guard(override_guard);
                    cnfg_free(base_config);
                    return (ft_nullptr);
                }
            }
            else
                new_entry->key = ft_nullptr;
            if (override_value)
            {
                new_entry->value = cma_strdup(override_value);
                if (!new_entry->value)
                {
                    ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                    cnfg_config_unlock_guard(base_guard);
                    cnfg_config_unlock_guard(override_guard);
                    cnfg_free(base_config);
                    return (ft_nullptr);
                }
            }
            else
                new_entry->value = ft_nullptr;
            if (cnfg_entry_prepare_thread_safety(new_entry) != 0)
            {
                cma_free(new_entry->section);
                cma_free(new_entry->key);
                cma_free(new_entry->value);
                new_entry->section = ft_nullptr;
                new_entry->key = ft_nullptr;
                new_entry->value = ft_nullptr;
                cnfg_config_unlock_guard(base_guard);
                cnfg_config_unlock_guard(override_guard);
                cnfg_free(base_config);
                return (ft_nullptr);
            }
            base_config->entry_count++;
        }
        ++override_index;
    }
    cnfg_config_unlock_guard(base_guard);
    cnfg_config_unlock_guard(override_guard);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (base_config);
}

static cnfg_config *append_flag_entry(cnfg_config *config, const char *flag)
{
    cnfg_entry entry;
    entry.mutex = ft_nullptr;
    entry.thread_safe_enabled = false;
    entry.section = ft_nullptr;
    entry.key = cma_strdup(flag);
    entry.value = ft_nullptr;
    if (!entry.key)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        cnfg_free(config);
        return (ft_nullptr);
    }
    cnfg_config temp;
    temp.entries = &entry;
    temp.entry_count = 1;
    temp.mutex = ft_nullptr;
    temp.thread_safe_enabled = false;
    config = merge_configs(config, &temp);
    cma_free(entry.key);
    return (config);
}

cnfg_config *config_merge_sources(int argument_count,
                                  char **argument_values,
                                  const char *filename)
{
    cnfg_config *result = config_load_file(filename);
    cnfg_config *env_config = config_load_env();
    result = merge_configs(result, env_config);
    if (env_config && result != env_config)
        cnfg_free(env_config);
    ft_size_t short_flags_depth = ft_global_error_stack_depth();
    char *short_flags = cnfg_parse_flags(argument_count, argument_values);
    int short_flags_errno = FT_ERR_SUCCESSS;
    ft_size_t short_flags_new_depth = ft_global_error_stack_depth();
    if (short_flags_new_depth != short_flags_depth)
    {
        short_flags_errno = ft_global_error_stack_last_error();
        ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(short_flags_errno);
    }
    if (!short_flags && short_flags_errno != FT_ERR_SUCCESSS)
    {
        if (result)
            cnfg_free(result);
        return (ft_nullptr);
    }
    size_t flag_index = 0;
    while (short_flags && short_flags[flag_index])
    {
        char key[2];
        key[0] = short_flags[flag_index];
        key[1] = '\0';
        result = append_flag_entry(result, key);
        if (!result)
        {
            cma_free(short_flags);
            return (ft_nullptr);
        }
        ++flag_index;
    }
    cma_free(short_flags);
    ft_size_t long_flags_depth = ft_global_error_stack_depth();
    char **long_flags = cnfg_parse_long_flags(argument_count, argument_values);
    int long_flags_errno = FT_ERR_SUCCESSS;
    ft_size_t long_flags_new_depth = ft_global_error_stack_depth();
    if (long_flags_new_depth != long_flags_depth)
    {
        long_flags_errno = ft_global_error_stack_last_error();
        ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(long_flags_errno);
    }
    if (!long_flags && long_flags_errno != FT_ERR_SUCCESSS)
    {
        if (result)
            cnfg_free(result);
        return (ft_nullptr);
    }
    flag_index = 0;
    while (long_flags && long_flags[flag_index])
    {
        result = append_flag_entry(result, long_flags[flag_index]);
        cma_free(long_flags[flag_index]);
        if (!result)
        {
            size_t cleanup_index = flag_index + 1;
            while (long_flags[cleanup_index])
            {
                cma_free(long_flags[cleanup_index]);
                ++cleanup_index;
            }
            cma_free(long_flags);
            return (ft_nullptr);
        }
        ++flag_index;
    }
    cma_free(long_flags);
    if (!result)
    {
        result = cnfg_config_create();
        if (!result)
            return (ft_nullptr);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
