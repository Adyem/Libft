#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../CPP_class/nullptr.hpp"
#include <cstddef>

struct ft_config_entry
{
    char    *section;
    char    *key;
    char    *value;
};

struct ft_config
{
    ft_config_entry *entries;
    size_t           entry_count;
};

ft_config   *ft_config_parse(const char *filename);
void        ft_config_free(ft_config *config);

#endif
