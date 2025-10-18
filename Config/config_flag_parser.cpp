#include "flag_parser.hpp"
#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/unique_lock.hpp"

cnfg_flag_parser::cnfg_flag_parser()
{
    this->_short_flags = ft_nullptr;
    this->_long_flags = ft_nullptr;
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

cnfg_flag_parser::cnfg_flag_parser(int argument_count, char **argument_values)
{
    this->_short_flags = ft_nullptr;
    this->_long_flags = ft_nullptr;
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    this->set_error(ER_SUCCESS);
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
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return ;
    }
    this->free_flags_locked();
    ft_errno = entry_errno;
    return ;
}

bool cnfg_flag_parser::parse(int argument_count, char **argument_values)
{
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (false);
    }
    this->free_flags_locked();
    this->set_error(ER_SUCCESS);
    this->_short_flags = cnfg_parse_flags(argument_count, argument_values);
    if (ft_errno != ER_SUCCESS)
    {
        this->set_error(ft_errno);
        this->free_flags_locked();
        return (false);
    }
    this->_long_flags = cnfg_parse_long_flags(argument_count, argument_values);
    if (ft_errno != ER_SUCCESS)
    {
        this->set_error(ft_errno);
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
    this->set_error(ER_SUCCESS);
    return (true);
}

bool cnfg_flag_parser::has_short_flag(char flag)
{
    int entry_errno;
    bool has_flag;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (false);
    }
    if (!this->_short_flags)
    {
        ft_errno = entry_errno;
        return (false);
    }
    has_flag = ft_strchr(this->_short_flags, flag) != ft_nullptr;
    ft_errno = entry_errno;
    return (has_flag);
}

bool cnfg_flag_parser::has_long_flag(const char *flag)
{
    int entry_errno;
    size_t flag_index;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (false);
    }
    if (!this->_long_flags || !flag)
    {
        ft_errno = entry_errno;
        return (false);
    }
    flag_index = 0;
    while (this->_long_flags[flag_index])
    {
        if (ft_strcmp(this->_long_flags[flag_index], flag) == 0)
        {
            ft_errno = entry_errno;
            return (true);
        }
        ++flag_index;
    }
    ft_errno = entry_errno;
    return (false);
}

size_t cnfg_flag_parser::get_short_flag_count()
{
    int entry_errno;
    size_t short_flag_count;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    short_flag_count = this->_short_flag_count;
    ft_errno = entry_errno;
    return (short_flag_count);
}

size_t cnfg_flag_parser::get_long_flag_count()
{
    int entry_errno;
    size_t long_flag_count;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    long_flag_count = this->_long_flag_count;
    ft_errno = entry_errno;
    return (long_flag_count);
}

size_t cnfg_flag_parser::get_total_flag_count()
{
    int entry_errno;
    size_t total_flag_count;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    total_flag_count = this->_short_flag_count + this->_long_flag_count;
    ft_errno = entry_errno;
    return (total_flag_count);
}

void    cnfg_flag_parser::set_error(int error_code)
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     cnfg_flag_parser::get_error() const
{
    int entry_errno;
    int error_code;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<cnfg_flag_parser *>(this)->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    error_code = this->_error_code;
    ft_errno = entry_errno;
    return (error_code);
}

const char  *cnfg_flag_parser::get_error_str() const
{
    int entry_errno;
    int error_code;
    const char *error_string;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<cnfg_flag_parser *>(this)->set_error(mutex_guard.get_error());
        return (ft_strerror(mutex_guard.get_error()));
    }
    error_code = this->_error_code;
    error_string = ft_strerror(error_code);
    ft_errno = entry_errno;
    return (error_string);
}

static cnfg_config *merge_configs(cnfg_config *base_config,
                                  cnfg_config *override_config)
{
    if (!override_config)
        return (base_config);
    size_t override_index = 0;
    while (override_index < override_config->entry_count)
    {
        cnfg_entry *override_entry = &override_config->entries[override_index];
        bool replaced = false;
        size_t base_index = 0;
        if (base_config)
        {
            while (base_index < base_config->entry_count)
            {
                cnfg_entry *base_entry = &base_config->entries[base_index];
                bool same_section = (base_entry->section == ft_nullptr
                    && override_entry->section == ft_nullptr)
                    || (base_entry->section && override_entry->section
                    && ft_strcmp(base_entry->section,
                    override_entry->section) == 0);
                bool same_key = (base_entry->key == ft_nullptr
                    && override_entry->key == ft_nullptr)
                    || (base_entry->key && override_entry->key
                    && ft_strcmp(base_entry->key,
                    override_entry->key) == 0);
                if (same_section && same_key)
                {
                    cma_free(base_entry->value);
                    if (override_entry->value)
                    {
                        base_entry->value = cma_strdup(override_entry->value);
                        if (!base_entry->value)
                        {
                            ft_errno = FT_ERR_NO_MEMORY;
                            cnfg_free(base_config);
                            return (ft_nullptr);
                        }
                    }
                    else
                        base_entry->value = ft_nullptr;
                    replaced = true;
                    break;
                }
                ++base_index;
            }
        }
        if (!replaced)
        {
            if (!base_config)
            {
                base_config = static_cast<cnfg_config*>(cma_calloc(1,
                    sizeof(cnfg_config)));
                if (!base_config)
                {
                    ft_errno = FT_ERR_NO_MEMORY;
                    return (ft_nullptr);
                }
            }
            cnfg_entry *new_entries = static_cast<cnfg_entry*>(cma_realloc(
                base_config->entries, sizeof(cnfg_entry)
                * (base_config->entry_count + 1)));
            if (!new_entries)
            {
                ft_errno = FT_ERR_NO_MEMORY;
                cnfg_free(base_config);
                return (ft_nullptr);
            }
            base_config->entries = new_entries;
            cnfg_entry *new_entry = &base_config->entries[base_config->entry_count];
            if (override_entry->section)
            {
                new_entry->section = cma_strdup(override_entry->section);
                if (!new_entry->section)
                {
                    ft_errno = FT_ERR_NO_MEMORY;
                    cnfg_free(base_config);
                    return (ft_nullptr);
                }
            }
            else
                new_entry->section = ft_nullptr;
            if (override_entry->key)
            {
                new_entry->key = cma_strdup(override_entry->key);
                if (!new_entry->key)
                {
                    ft_errno = FT_ERR_NO_MEMORY;
                    cnfg_free(base_config);
                    return (ft_nullptr);
                }
            }
            else
                new_entry->key = ft_nullptr;
            if (override_entry->value)
            {
                new_entry->value = cma_strdup(override_entry->value);
                if (!new_entry->value)
                {
                    ft_errno = FT_ERR_NO_MEMORY;
                    cnfg_free(base_config);
                    return (ft_nullptr);
                }
            }
            else
                new_entry->value = ft_nullptr;
            base_config->entry_count++;
        }
        ++override_index;
    }
    ft_errno = ER_SUCCESS;
    return (base_config);
}

static cnfg_config *append_flag_entry(cnfg_config *config, const char *flag)
{
    cnfg_entry entry;
    entry.section = ft_nullptr;
    entry.key = cma_strdup(flag);
    entry.value = ft_nullptr;
    if (!entry.key)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        cnfg_free(config);
        return (ft_nullptr);
    }
    cnfg_config temp;
    temp.entries = &entry;
    temp.entry_count = 1;
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
    int previous_errno = ft_errno;
    ft_errno = ER_SUCCESS;
    char *short_flags = cnfg_parse_flags(argument_count, argument_values);
    int short_flags_errno = ft_errno;
    if (!short_flags && short_flags_errno != ER_SUCCESS)
    {
        if (result)
            cnfg_free(result);
        ft_errno = short_flags_errno;
        return (ft_nullptr);
    }
    if (short_flags_errno == ER_SUCCESS)
        ft_errno = previous_errno;
    else
        ft_errno = short_flags_errno;
    if (short_flags)
    {
        size_t flag_index = 0;
        while (short_flags[flag_index])
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
    }
    previous_errno = ft_errno;
    ft_errno = ER_SUCCESS;
    char **long_flags = cnfg_parse_long_flags(argument_count, argument_values);
    int long_flags_errno = ft_errno;
    if (!long_flags && long_flags_errno != ER_SUCCESS)
    {
        if (result)
            cnfg_free(result);
        ft_errno = long_flags_errno;
        return (ft_nullptr);
    }
    if (long_flags_errno == ER_SUCCESS)
        ft_errno = previous_errno;
    else
        ft_errno = long_flags_errno;
    if (long_flags)
    {
        size_t flag_index = 0;
        while (long_flags[flag_index])
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
    }
    if (!result)
    {
        result = static_cast<cnfg_config*>(cma_calloc(1, sizeof(cnfg_config)));
        if (!result)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
    }
    return (result);
}
