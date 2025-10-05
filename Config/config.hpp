#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../CPP_class/class_nullptr.hpp"
#include <cstddef>

struct cnfg_entry
{
    char    *section;
    char    *key;
    char    *value;
};

struct cnfg_config
{
    cnfg_entry *entries;
    size_t           entry_count;
};


cnfg_config   *cnfg_parse(const char *filename);
void        cnfg_free(cnfg_config *config);
char       *cnfg_parse_flags(int argument_count, char **argument_values);
char      **cnfg_parse_long_flags(int argument_count, char **argument_values);
cnfg_config   *config_load_env();
cnfg_config   *config_load_file(const char *filename);
int         config_write_file(const cnfg_config *config, const char *filename);
cnfg_config   *config_merge(const cnfg_config *base_config, const cnfg_config *override_config);

#endif
