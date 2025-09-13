#include "../../Config/config.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Libft/libft.hpp"
#include <cstdio>
#include <cstring>

int test_config_basic(void)
{
    const char *filename = "test_config.ini";
    FILE *file = std::fopen(filename, "w");
    if (!file)
        return (0);
    std::fprintf(file, "[section]\nkey=value\n");
    std::fclose(file);
    cnfg_config *cfg = cnfg_parse(filename);
    int ok = cfg && cfg->entry_count == 1 &&
             cfg->entries[0].section && std::strcmp(cfg->entries[0].section, "section") == 0 &&
             cfg->entries[0].key && std::strcmp(cfg->entries[0].key, "key") == 0 &&
             cfg->entries[0].value && std::strcmp(cfg->entries[0].value, "value") == 0;
    if (cfg)
        cnfg_free(cfg);
    std::remove(filename);
    return (ok);
}

int test_config_missing_value(void)
{
    const char *filename = "test_config_missing.ini";
    FILE *file = std::fopen(filename, "w");
    if (!file)
        return (0);
    std::fprintf(file, "key_without_value=\n=value_without_key\n");
    std::fclose(file);
    cnfg_config *cfg = cnfg_parse(filename);
    int ok = cfg && cfg->entry_count == 2 &&
             cfg->entries[0].key && std::strcmp(cfg->entries[0].key, "key_without_value") == 0 &&
             cfg->entries[0].value == ft_nullptr &&
             cfg->entries[1].key == ft_nullptr &&
             cfg->entries[1].value && std::strcmp(cfg->entries[1].value, "value_without_key") == 0;
    if (cfg)
        cnfg_free(cfg);
    std::remove(filename);
    return (ok);
}

int test_config_env_override(void)
{
    const char *filename = "test_config_env.ini";
    FILE *file = std::fopen(filename, "w");
    if (!file)
        return (0);
    std::fprintf(file, "key=file\n");
    std::fclose(file);
    if (ft_setenv("key", "env", 1) != 0)
    {
        std::remove(filename);
        return (0);
    }
    cnfg_config *config = cnfg_parse(filename);
    int is_ok = config && config->entry_count == 1 &&
                config->entries[0].value && std::strcmp(config->entries[0].value, "env") == 0;
    if (config)
        cnfg_free(config);
    std::remove(filename);
    return (is_ok);
}

