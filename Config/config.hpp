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

/*
** cnfg_parse reads a configuration file and fills a cnfg_config
** structure. For each key, the parser first checks the environment
** using ft_getenv. If an environment variable with the same key is
** found, its value overrides the one from the file.
*/
cnfg_config   *cnfg_parse(const char *filename);
void        cnfg_free(cnfg_config *config);
char       *cnfg_parse_flags(int argument_count, char **argument_values);
char      **cnfg_parse_long_flags(int argument_count, char **argument_values);
cnfg_config   *config_load_env();
cnfg_config   *config_load_file(const char *filename);

#endif
