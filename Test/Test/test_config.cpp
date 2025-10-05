#include "../../Config/config.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdio>
#include <cstring>

static void cleanup_file(const char *filename)
{
    if (!filename)
        return ;
    std::remove(filename);
    return ;
}

FT_TEST(test_cnfg_parse_null_filename_sets_errno, "cnfg_parse rejects null filename")
{
    ft_errno = ER_SUCCESS;
    cnfg_config *config = cnfg_parse(ft_nullptr);
    FT_ASSERT(config == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_config_load_file_null_filename_sets_errno, "config_load_file rejects null filename")
{
    ft_errno = ER_SUCCESS;
    cnfg_config *config = config_load_file(ft_nullptr);
    FT_ASSERT(config == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cnfg_parse_success_sets_errno_success, "cnfg_parse loads ini files and clears errno")
{
    const char *filename = "test_config.ini";
    FILE *file = std::fopen(filename, "w");
    if (!file)
        return (0);
    std::fprintf(file, "[section]\nkey=value\n");
    std::fclose(file);
    ft_errno = FT_EINVAL;
    cnfg_config *config = cnfg_parse(filename);
    FT_ASSERT(config != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(config->entry_count == 1);
    FT_ASSERT(config->entries[0].section != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].section, "section") == 0);
    FT_ASSERT(config->entries[0].key != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].key, "key") == 0);
    FT_ASSERT(config->entries[0].value != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].value, "value") == 0);
    if (config)
        cnfg_free(config);
    cleanup_file(filename);
    return (1);
}

FT_TEST(test_cnfg_parse_missing_value_handles_entries, "cnfg_parse accepts missing keys and values")
{
    const char *filename = "test_config_missing.ini";
    FILE *file = std::fopen(filename, "w");
    if (!file)
        return (0);
    std::fprintf(file, "key_without_value=\n=value_without_key\n");
    std::fclose(file);
    ft_errno = FT_EALLOC;
    cnfg_config *config = cnfg_parse(filename);
    FT_ASSERT(config != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(config->entry_count == 2);
    FT_ASSERT(config->entries[0].key != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].key, "key_without_value") == 0);
    FT_ASSERT(config->entries[0].value == ft_nullptr);
    FT_ASSERT(config->entries[1].key == ft_nullptr);
    FT_ASSERT(config->entries[1].value != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[1].value, "value_without_key") == 0);
    if (config)
        cnfg_free(config);
    cleanup_file(filename);
    return (1);
}

