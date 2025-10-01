#include "../../Storage/kv_store.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../File/file_utils.hpp"
#include "../../File/open_dir.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include <cstdio>
#include <cerrno>
#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <unistd.h>
#endif

static void remove_directory_if_present(const char *directory_path)
{
    int directory_exists;

    directory_exists = cmp_directory_exists(directory_path);
    if (directory_exists != 1)
        return ;
#if defined(_WIN32) || defined(_WIN64)
    RemoveDirectoryA(directory_path);
#else
    rmdir(directory_path);
#endif
    return ;
}

static void cleanup_paths(const char *directory_path, const char *file_path)
{
    file_delete(file_path);
    remove_directory_if_present(directory_path);
    ft_errno = ER_SUCCESS;
    return ;
}

static void create_kv_store_file(const char *file_path)
{
    FILE *file_pointer;

    file_pointer = ft_fopen(file_path, "w");
    if (file_pointer == ft_nullptr)
        return ;
    std::fputs("{\n  \"kv_store\": {\n    \"__placeholder__\": \"\"\n  }\n}\n", file_pointer);
    ft_fclose(file_pointer);
    return ;
}

FT_TEST(test_kv_store_flush_propagates_json_writer_errno, "kv_store flush propagates json writer errno")
{
    const char *directory_path;
    const char *file_path;
    int flush_result;
    int expected_error;

    directory_path = "kv_store_flush_failure_directory";
    file_path = "kv_store_flush_failure_directory/kv_store.json";
    cleanup_paths(directory_path, file_path);
    FT_ASSERT_EQ(0, file_create_directory(directory_path, 0700));
    create_kv_store_file(file_path);
    kv_store store(file_path);
    FT_ASSERT_EQ(ER_SUCCESS, store.get_error());
    FT_ASSERT_EQ(0, store.kv_delete("__placeholder__"));
    FT_ASSERT_EQ(ER_SUCCESS, store.get_error());
    FT_ASSERT_EQ(0, store.kv_set("key", "value"));
    FT_ASSERT_EQ(ER_SUCCESS, store.get_error());
    FT_ASSERT_EQ(0, file_delete(file_path));
    remove_directory_if_present(directory_path);
    ft_errno = ER_SUCCESS;
    flush_result = store.kv_flush();
    FT_ASSERT_EQ(-1, flush_result);
#if defined(_WIN32) || defined(_WIN64)
    expected_error = ERROR_PATH_NOT_FOUND + ERRNO_OFFSET;
#else
    expected_error = ENOENT + ERRNO_OFFSET;
#endif
    FT_ASSERT_EQ(expected_error, ft_errno);
    FT_ASSERT_EQ(expected_error, store.get_error());
    cleanup_paths(directory_path, file_path);
    return (1);
}

