#include "../test_internal.hpp"
#include "../../Config/config.hpp"
#include "../../Advanced/advanced.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdio>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static void cleanup_file(const char *filename)
{
    if (!filename)
        return ;
    std::remove(filename);
    return ;
}

static config_data *create_test_config(size_t entry_count)
{
    config_data *config;

    config = config_data_create();
    if (!config)
        return (ft_nullptr);
    if (entry_count)
    {
        config->entries = static_cast<config_entry*>(adv_calloc(entry_count, sizeof(config_entry)));
        if (!config->entries)
        {
            config_data_free(config);
            return (ft_nullptr);
        }
    }
    config->entry_count = entry_count;
    return (config);
}

FT_TEST(test_config_parse_null_filename_sets_errno, "config_parse rejects null filename")
{
    config_data *config = config_parse(ft_nullptr);
    FT_ASSERT(config == ft_nullptr);
    return (1);
}

FT_TEST(test_config_load_file_null_filename_sets_errno, "config_load_file rejects null filename")
{
    config_data *config = config_load_file(ft_nullptr);
    FT_ASSERT(config == ft_nullptr);
    return (1);
}

FT_TEST(test_config_parse_success_sets_errno_success, "config_parse loads ini files and clears errno")
{
    const char *filename = "test_config.ini";
    config_data *source = create_test_config(1);
    if (!source)
        return (0);
    source->entries[0].section = adv_strdup("section");
    if (!source->entries[0].section)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[0].key = adv_strdup("key");
    if (!source->entries[0].key)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[0].value = adv_strdup("value");
    if (!source->entries[0].value)
    {
        config_data_free(source);
        return (0);
    }
    if (config_write_file(source, filename) != 0)
    {
        config_data_free(source);
        cleanup_file(filename);
        return (0);
    }
    config_data_free(source);
    config_data *config = config_parse(filename);
    FT_ASSERT(config != ft_nullptr);
    FT_ASSERT(config->entry_count == 1);
    FT_ASSERT(config->entries[0].section != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].section, "section") == 0);
    FT_ASSERT(config->entries[0].key != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].key, "key") == 0);
    FT_ASSERT(config->entries[0].value != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].value, "value") == 0);
    if (config)
        config_data_free(config);
    cleanup_file(filename);
    return (1);
}

FT_TEST(test_config_parse_missing_value_handles_entries, "config_parse accepts missing keys and values")
{
    const char *filename = "test_config_missing.ini";
    config_data *source = create_test_config(2);
    if (!source)
        return (0);
    source->entries[0].key = adv_strdup("key_without_value");
    if (!source->entries[0].key)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[1].value = adv_strdup("value_without_key");
    if (!source->entries[1].value)
    {
        config_data_free(source);
        return (0);
    }
    if (config_write_file(source, filename) != 0)
    {
        config_data_free(source);
        cleanup_file(filename);
        return (0);
    }
    config_data_free(source);
    config_data *config = config_parse(filename);
    FT_ASSERT(config != ft_nullptr);
    FT_ASSERT(config->entry_count == 2);
    FT_ASSERT(config->entries[0].key != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[0].key, "key_without_value") == 0);
    FT_ASSERT(config->entries[0].value == ft_nullptr);
    FT_ASSERT(config->entries[1].key == ft_nullptr);
    FT_ASSERT(config->entries[1].value != ft_nullptr);
    FT_ASSERT(std::strcmp(config->entries[1].value, "value_without_key") == 0);
    if (config)
        config_data_free(config);
    cleanup_file(filename);
    return (1);
}

FT_TEST(test_config_write_ini_round_trip, "config_write_file supports ini round trips")
{
    const char *filename = "config_round_trip.ini";
    config_data *source = create_test_config(3);
    if (!source)
        return (0);
    source->entries[0].section = adv_strdup("alpha");
    if (!source->entries[0].section)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[0].key = adv_strdup("first");
    if (!source->entries[0].key)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[0].value = adv_strdup("one");
    if (!source->entries[0].value)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[1].section = adv_strdup("alpha");
    if (!source->entries[1].section)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[1].key = adv_strdup("second");
    if (!source->entries[1].key)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[2].key = adv_strdup("global");
    if (!source->entries[2].key)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[2].value = adv_strdup("value");
    if (!source->entries[2].value)
    {
        config_data_free(source);
        return (0);
    }
    if (config_write_file(source, filename) != 0)
    {
        config_data_free(source);
        cleanup_file(filename);
        return (0);
    }
    config_data_free(source);
    config_data *parsed = config_parse(filename);
    FT_ASSERT(parsed != ft_nullptr);
    FT_ASSERT(parsed->entry_count == 3);
    FT_ASSERT(parsed->entries[0].section != ft_nullptr);
    FT_ASSERT(std::strcmp(parsed->entries[0].section, "alpha") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[0].key, "first") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[0].value, "one") == 0);
    FT_ASSERT(parsed->entries[1].section != ft_nullptr);
    FT_ASSERT(std::strcmp(parsed->entries[1].section, "alpha") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[1].key, "second") == 0);
    FT_ASSERT(parsed->entries[1].value == ft_nullptr);
    FT_ASSERT(parsed->entries[2].section == ft_nullptr);
    FT_ASSERT(std::strcmp(parsed->entries[2].key, "global") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[2].value, "value") == 0);
    config_data_free(parsed);
    cleanup_file(filename);
    return (1);
}

FT_TEST(test_config_data_create_initializes_thread_safety, "config_data_create prepares mutex protection")
{
    config_data *config;

    config = config_data_create();
    if (!config)
        return (0);
    FT_ASSERT(config->mutex != ft_nullptr);
    config_data_free(config);
    return (1);
}

FT_TEST(test_config_data_prepare_thread_safety_reinitializes_mutex, "config_data_prepare_thread_safety recreates mutex after teardown")
{
    config_data *config;

    config = config_data_create();
    if (!config)
        return (0);
    config_data_teardown_thread_safety(config);
    FT_ASSERT(config->mutex == ft_nullptr);
    FT_ASSERT_EQ(0, config_data_prepare_thread_safety(config));
    FT_ASSERT(config->mutex != ft_nullptr);
    config_data_free(config);
    return (1);
}

FT_TEST(test_config_entry_prepare_thread_safety_initializes_mutex, "config_entry_prepare_thread_safety allocates mutex protection")
{
    config_entry entry;
    bool lock_acquired;

    entry.mutex = ft_nullptr;
    entry.section = ft_nullptr;
    entry.key = ft_nullptr;
    entry.value = ft_nullptr;
    FT_ASSERT_EQ(0, config_entry_prepare_thread_safety(&entry));
    FT_ASSERT(entry.mutex != ft_nullptr);
    lock_acquired = false;
    FT_ASSERT_EQ(0, config_entry_lock(&entry, &lock_acquired));
    FT_ASSERT(lock_acquired == true);
    config_entry_unlock(&entry, lock_acquired);
    config_entry_teardown_thread_safety(&entry);
    FT_ASSERT(entry.mutex == ft_nullptr);
    return (1);
}

FT_TEST(test_config_entry_lock_handles_disabled_thread_safety, "config_entry_lock succeeds when mutex setup is skipped")
{
    config_entry entry;
    bool lock_acquired;

    entry.mutex = ft_nullptr;
    entry.section = ft_nullptr;
    entry.key = ft_nullptr;
    entry.value = ft_nullptr;
    lock_acquired = true;
    FT_ASSERT_EQ(0, config_entry_lock(&entry, &lock_acquired));
    FT_ASSERT(lock_acquired == false);
    config_entry_unlock(&entry, lock_acquired);
    return (1);
}

FT_TEST(test_config_write_json_round_trip, "config_write_file supports json round trips")
{
    const char *filename = "config_round_trip.json";
    config_data *source = create_test_config(2);
    if (!source)
        return (0);
    source->entries[0].section = adv_strdup("alpha");
    if (!source->entries[0].section)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[0].key = adv_strdup("first");
    if (!source->entries[0].key)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[0].value = adv_strdup("one");
    if (!source->entries[0].value)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[1].section = adv_strdup("beta");
    if (!source->entries[1].section)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[1].key = adv_strdup("second");
    if (!source->entries[1].key)
    {
        config_data_free(source);
        return (0);
    }
    source->entries[1].value = adv_strdup("two");
    if (!source->entries[1].value)
    {
        config_data_free(source);
        return (0);
    }
    if (config_write_file(source, filename) != 0)
    {
        config_data_free(source);
        cleanup_file(filename);
        return (0);
    }
    config_data_free(source);
    config_data *parsed = config_load_file(filename);
    FT_ASSERT(parsed != ft_nullptr);
    FT_ASSERT(parsed->entry_count == 2);
    FT_ASSERT(parsed->entries[0].section != ft_nullptr);
    FT_ASSERT(std::strcmp(parsed->entries[0].section, "alpha") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[0].key, "first") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[0].value, "one") == 0);
    FT_ASSERT(parsed->entries[1].section != ft_nullptr);
    FT_ASSERT(std::strcmp(parsed->entries[1].section, "beta") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[1].key, "second") == 0);
    FT_ASSERT(std::strcmp(parsed->entries[1].value, "two") == 0);
    config_data_free(parsed);
    cleanup_file(filename);
    return (1);
}

FT_TEST(test_config_merge_prefers_override_entries, "config_merge replaces duplicate keys with override values")
{
    config_data *base = create_test_config(2);
    if (!base)
        return (0);
    base->entries[0].section = adv_strdup("shared");
    if (!base->entries[0].section)
    {
        config_data_free(base);
        return (0);
    }
    base->entries[0].key = adv_strdup("key");
    if (!base->entries[0].key)
    {
        config_data_free(base);
        return (0);
    }
    base->entries[0].value = adv_strdup("base");
    if (!base->entries[0].value)
    {
        config_data_free(base);
        return (0);
    }
    base->entries[1].section = adv_strdup("base_only");
    if (!base->entries[1].section)
    {
        config_data_free(base);
        return (0);
    }
    base->entries[1].key = adv_strdup("flag");
    if (!base->entries[1].key)
    {
        config_data_free(base);
        return (0);
    }
    base->entries[1].value = adv_strdup("true");
    if (!base->entries[1].value)
    {
        config_data_free(base);
        return (0);
    }
    config_data *override_config = create_test_config(2);
    if (!override_config)
    {
        config_data_free(base);
        return (0);
    }
    override_config->entries[0].section = adv_strdup("shared");
    if (!override_config->entries[0].section)
    {
        config_data_free(base);
        config_data_free(override_config);
        return (0);
    }
    override_config->entries[0].key = adv_strdup("key");
    if (!override_config->entries[0].key)
    {
        config_data_free(base);
        config_data_free(override_config);
        return (0);
    }
    override_config->entries[0].value = adv_strdup("override");
    if (!override_config->entries[0].value)
    {
        config_data_free(base);
        config_data_free(override_config);
        return (0);
    }
    override_config->entries[1].section = adv_strdup("override_only");
    if (!override_config->entries[1].section)
    {
        config_data_free(base);
        config_data_free(override_config);
        return (0);
    }
    override_config->entries[1].key = adv_strdup("flag");
    if (!override_config->entries[1].key)
    {
        config_data_free(base);
        config_data_free(override_config);
        return (0);
    }
    override_config->entries[1].value = adv_strdup("false");
    if (!override_config->entries[1].value)
    {
        config_data_free(base);
        config_data_free(override_config);
        return (0);
    }
    config_data *merged = config_merge(base, override_config);
    FT_ASSERT(merged != ft_nullptr);
    FT_ASSERT(merged->entry_count == 3);
    FT_ASSERT(std::strcmp(base->entries[0].value, "base") == 0);
    size_t index = 0;
    bool found_override = false;
    bool found_base_only = false;
    bool found_new_entry = false;
    while (index < merged->entry_count)
    {
        config_entry *entry = &merged->entries[index];
        if (entry->section && ft_strcmp(entry->section, "shared") == 0)
        {
            FT_ASSERT(std::strcmp(entry->key, "key") == 0);
            FT_ASSERT(std::strcmp(entry->value, "override") == 0);
            found_override = true;
        }
        else if (entry->section && ft_strcmp(entry->section, "base_only") == 0)
        {
            FT_ASSERT(std::strcmp(entry->key, "flag") == 0);
            FT_ASSERT(std::strcmp(entry->value, "true") == 0);
            found_base_only = true;
        }
        else if (entry->section && ft_strcmp(entry->section, "override_only") == 0)
        {
            FT_ASSERT(std::strcmp(entry->key, "flag") == 0);
            FT_ASSERT(std::strcmp(entry->value, "false") == 0);
            found_new_entry = true;
        }
        ++index;
    }
    FT_ASSERT(found_override);
    FT_ASSERT(found_base_only);
    FT_ASSERT(found_new_entry);
    config_data_free(base);
    config_data_free(override_config);
    config_data_free(merged);
    return (1);
}
