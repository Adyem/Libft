#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../CPP_class/class_nullptr.hpp"
#include <cstddef>

class pt_mutex;

struct config_entry
{
    pt_mutex *mutex;
    char    *section;
    char    *key;
    char    *value;
};

struct config_data
{
    config_entry  *entries;
    size_t               entry_count;
    pt_mutex             *mutex;
};

config_data   *config_data_create();
int         config_data_prepare_thread_safety(config_data *config);
void        config_data_teardown_thread_safety(config_data *config);
config_data   *config_parse(const char *filename);
void        config_data_free(config_data *config);
config_data   *config_load_env();
config_data   *config_load_file(const char *filename);
int         config_write_file(const config_data *config, const char *filename);
config_data   *config_merge(const config_data *base_config, const config_data *override_config);

int         config_entry_prepare_thread_safety(config_entry *entry);
void        config_entry_teardown_thread_safety(config_entry *entry);
int         config_entry_lock(config_entry *entry, bool *lock_acquired);
void        config_entry_unlock(config_entry *entry, bool lock_acquired);

#endif
