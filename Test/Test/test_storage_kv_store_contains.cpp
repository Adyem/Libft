#include "../test_internal.hpp"
#include "../../Modules/Storage/kv_store.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_storage_kv_store_contains_distinguishes_empty_and_missing)
{
    const char *file_path;
    const char *initial_contents;
    kv_store store;

    file_path = "test_storage_contains.json";
    initial_contents = "{\"kv_store\":{}}\n";
    file_delete(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, initial_contents,
            ft_strlen_size_t(initial_contents)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, store.initialize(file_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, store.kv_set("empty", ""));
    FT_ASSERT_EQ(FT_TRUE, store.kv_contains("empty"));
    FT_ASSERT_EQ(FT_FALSE, store.kv_contains("missing"));
    store.destroy();
    file_delete(file_path);
    return (1);
}

FT_TEST(test_storage_kv_store_contains_remains_usable_with_thread_safety)
{
    const char *file_path;
    const char *initial_contents;
    kv_store store;

    file_path = "test_storage_contains_thread_safe.json";
    initial_contents = "{\"kv_store\":{}}\n";
    file_delete(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, initial_contents,
            ft_strlen_size_t(initial_contents)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, store.initialize(file_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, store.enable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, store.kv_contains("missing"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, store.kv_set("present", "value"));
    FT_ASSERT_EQ(FT_TRUE, store.kv_contains("present"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, store.disable_thread_safety());
    store.destroy();
    file_delete(file_path);
    return (1);
}

FT_TEST(test_storage_kv_store_contains_rejects_null_key)
{
    const char *file_path;
    const char *initial_contents;
    kv_store store;

    file_path = "test_storage_contains_null.json";
    initial_contents = "{\"kv_store\":{}}\n";
    file_delete(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, initial_contents,
            ft_strlen_size_t(initial_contents)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, store.initialize(file_path));
    FT_ASSERT_EQ(FT_FALSE, store.kv_contains(ft_nullptr));
    store.destroy();
    file_delete(file_path);
    return (1);
}
