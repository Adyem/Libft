#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../CPP_class/class_nullptr.hpp"
#include <cstddef>

class pt_mutex;

struct cnfg_entry
{
    pt_mutex *mutex;
    char    *section;
    char    *key;
    char    *value;
};

struct cnfg_config
{
    cnfg_entry  *entries;
    size_t               entry_count;
    pt_mutex             *mutex;
};

cnfg_config   *cnfg_config_create();
int         cnfg_config_prepare_thread_safety(cnfg_config *config);
void        cnfg_config_teardown_thread_safety(cnfg_config *config);

cnfg_config   *cnfg_parse(const char *filename);
void        cnfg_free(cnfg_config *config);
cnfg_config   *config_load_env();
cnfg_config   *config_load_file(const char *filename);
int         config_write_file(const cnfg_config *config, const char *filename);
cnfg_config   *config_merge(const cnfg_config *base_config, const cnfg_config *override_config);

int         cnfg_entry_prepare_thread_safety(cnfg_entry *entry);
void        cnfg_entry_teardown_thread_safety(cnfg_entry *entry);
int         cnfg_entry_lock(cnfg_entry *entry, bool *lock_acquired);
void        cnfg_entry_unlock(cnfg_entry *entry, bool lock_acquired);

#endif
