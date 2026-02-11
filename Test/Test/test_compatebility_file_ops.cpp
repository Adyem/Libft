#include "../../Compatebility/compatebility_internal.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_system_test_hooks.hpp"
#include <cerrno>
#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
# include <io.h>
# include <windows.h>
# include <direct.h>
#else
# include <sys/stat.h>
# include <unistd.h>
#endif

static void remove_file_if_present(const char *path)
{
    if (path == ft_nullptr)
        return ;
#if defined(_WIN32) || defined(_WIN64)
    _unlink(path);
#else
    ::unlink(path);
#endif
    return ;
}

static void remove_directory_if_present(const char *path)
{
    if (path == ft_nullptr)
        return ;
#if defined(_WIN32) || defined(_WIN64)
    _rmdir(path);
#else
    ::rmdir(path);
#endif
    return ;
}

static void write_test_file(const char *path)
{
    FILE *file_pointer;

    if (path == ft_nullptr)
        return ;
    file_pointer = std::fopen(path, "w");
    if (file_pointer != ft_nullptr)
    {
        std::fputs("payload", file_pointer);
        std::fclose(file_pointer);
    }
    return ;
}

FT_TEST(test_cmp_file_exists_null_pointer_sets_errno, "cmp_file_exists reports FT_ERR_INVALID_ARGUMENT for null path")
{
    int error_code = FT_ERR_SUCCESSS;
    int exists_value = 1;

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, cmp_file_exists(ft_nullptr,
        &exists_value, &error_code));
    FT_ASSERT_EQ(0, exists_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_cmp_file_exists_missing_path_sets_errno, "cmp_file_exists reports FT_ERR_IO for missing file")
{
    const char *missing_path = "cmp_file_exists_missing_path.txt";
    int error_code = FT_ERR_SUCCESSS;
    int exists_value = 1;

    remove_file_if_present(missing_path);
    FT_ASSERT_EQ(FT_ERR_IO, cmp_file_exists(missing_path, &exists_value,
        &error_code));
    FT_ASSERT_EQ(0, exists_value);
    FT_ASSERT_EQ(FT_ERR_IO, error_code);
    return (1);
}

FT_TEST(test_cmp_file_exists_success_clears_errno, "cmp_file_exists clears errno on success")
{
    const char *path = "cmp_file_exists_success.txt";
    int error_code = FT_ERR_INVALID_ARGUMENT;
    int exists_value = 0;

    remove_file_if_present(path);
    write_test_file(path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, cmp_file_exists(path, &exists_value,
        &error_code));
    FT_ASSERT_EQ(1, exists_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, error_code);
    remove_file_if_present(path);
    return (1);
}

FT_TEST(test_cmp_file_delete_null_pointer_sets_errno, "cmp_file_delete reports FT_ERR_INVALID_ARGUMENT for null path")
{
    int error_code = FT_ERR_SUCCESSS;

    FT_ASSERT_EQ(-1, cmp_file_delete(ft_nullptr, &error_code));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_cmp_file_delete_permission_error_sets_errno, "cmp_file_delete reports EACCES for protected directory")
{
    const char *directory_path = "cmp_file_delete_permission_dir";
    const char *file_path = "cmp_file_delete_permission_dir/file.txt";
    int error_code = FT_ERR_SUCCESSS;

    remove_file_if_present(file_path);
    remove_directory_if_present(directory_path);
    FT_ASSERT_EQ(0,
#if defined(_WIN32) || defined(_WIN64)
        _mkdir(directory_path)
#else
        ::mkdir(directory_path, 0700)
#endif
    );
    write_test_file(file_path);
    FT_ASSERT_EQ(-1, cmp_file_delete(directory_path, &error_code));
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, error_code);
    cmp_file_delete(file_path, &error_code);
    remove_directory_if_present(directory_path);
    return (1);
}

FT_TEST(test_cmp_file_delete_success_clears_errno, "cmp_file_delete clears errno on success")
{
    const char *path = "cmp_file_delete_success.txt";
    int error_code = FT_ERR_INVALID_ARGUMENT;

    remove_file_if_present(path);
    write_test_file(path);
    FT_ASSERT_EQ(0, cmp_file_delete(path, &error_code));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, error_code);
    return (1);
}

FT_TEST(test_cmp_file_move_null_pointer_sets_errno, "cmp_file_move reports FT_ERR_INVALID_ARGUMENT for null source")
{
    int error_code = FT_ERR_SUCCESSS;

    FT_ASSERT_EQ(-1,
        cmp_file_move(ft_nullptr, "cmp_file_move_null_pointer_destination.txt", &error_code));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_cmp_file_move_missing_source_sets_errno, "cmp_file_move reports FT_ERR_IO for missing source")
{
    const char *source_path = "cmp_file_move_missing_source.txt";
    const char *destination_path = "cmp_file_move_missing_destination.txt";
    int error_code = FT_ERR_SUCCESSS;

    remove_file_if_present(source_path);
    remove_file_if_present(destination_path);
    FT_ASSERT_EQ(-1, cmp_file_move(source_path, destination_path, &error_code));
    FT_ASSERT_EQ(FT_ERR_IO, error_code);
    return (1);
}

FT_TEST(test_cmp_file_move_cross_device_copy_failure_sets_errno, "cmp_file_move sets errno to the first fallback failure")
{
    const char *source_path = "cmp_file_move_cross_device_failure_source.txt";
    const char *destination_directory = "cmp_file_move_cross_device_failure_dir";
    const char *stale_destination_path = "cmp_file_move_cross_device_failure_dir/destination.txt";
    int error_code = FT_ERR_CONFIGURATION;

    remove_file_if_present(source_path);
    remove_file_if_present(stale_destination_path);
    remove_directory_if_present(destination_directory);
    write_test_file(source_path);
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(0, _mkdir(destination_directory));
#else
    FT_ASSERT_EQ(0, ::mkdir(destination_directory, 0700));
#endif
    cmp_set_force_cross_device_move(1);
    FT_ASSERT_EQ(-1, cmp_file_move(source_path, destination_directory, &error_code));
    FT_ASSERT_NE(FT_ERR_CONFIGURATION, error_code);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, error_code);
    cmp_set_force_cross_device_move(0);
    remove_file_if_present(source_path);
    remove_directory_if_present(destination_directory);
    remove_file_if_present(stale_destination_path);
    return (1);
}

FT_TEST(test_cmp_file_move_cross_device_directory_destination_sets_errno, "cmp_file_move reports FT_ERR_INVALID_OPERATION for directory destination in fallback")
{
    const char *source_path = "cmp_file_move_cross_device_directory_destination_source.txt";
    const char *destination_directory = "cmp_file_move_cross_device_directory_destination_dir";
    const char *stale_destination_path = "cmp_file_move_cross_device_directory_destination_dir/destination.txt";
    FILE *file_pointer;
    int error_code = FT_ERR_CONFIGURATION;

    remove_file_if_present(source_path);
    remove_file_if_present(stale_destination_path);
    remove_directory_if_present(destination_directory);
    write_test_file(source_path);
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(0, _mkdir(destination_directory));
#else
    FT_ASSERT_EQ(0, ::mkdir(destination_directory, 0700));
#endif
    cmp_set_force_cross_device_move(1);
    FT_ASSERT_EQ(-1, cmp_file_move(source_path, destination_directory, &error_code));
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, error_code);
    cmp_set_force_cross_device_move(0);
    file_pointer = std::fopen(source_path, "r");
    FT_ASSERT_NE(ft_nullptr, file_pointer);
    if (file_pointer != ft_nullptr)
        std::fclose(file_pointer);
    file_pointer = std::fopen(stale_destination_path, "r");
    FT_ASSERT_EQ(ft_nullptr, file_pointer);
    if (file_pointer != ft_nullptr)
        std::fclose(file_pointer);
    remove_file_if_present(source_path);
    remove_directory_if_present(destination_directory);
    remove_file_if_present(stale_destination_path);
    return (1);
}

FT_TEST(test_cmp_file_copy_null_pointer_sets_errno, "cmp_file_copy reports FT_ERR_INVALID_ARGUMENT for null source")
{
    int error_code = FT_ERR_SUCCESSS;

    FT_ASSERT_EQ(-1, cmp_file_copy(ft_nullptr, "cmp_file_copy_null_pointer_destination.txt", &error_code));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_cmp_file_copy_permission_error_sets_errno, "cmp_file_copy reports permission errors")
{
    const char *source_path = "cmp_file_copy_permission_source.txt";
    const char *destination_directory = "cmp_file_copy_permission_dir";
    const char *stale_destination_path = "cmp_file_copy_permission_dir/destination.txt";
    int error_code = FT_ERR_SUCCESSS;

    remove_file_if_present(source_path);
    remove_file_if_present(stale_destination_path);
    remove_directory_if_present(destination_directory);
    write_test_file(source_path);
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(0, _mkdir(destination_directory));
#else
    FT_ASSERT_EQ(0, ::mkdir(destination_directory, 0700));
#endif
    FT_ASSERT_EQ(-1, cmp_file_copy(source_path, destination_directory, &error_code));
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, error_code);
    remove_file_if_present(source_path);
    remove_directory_if_present(destination_directory);
    remove_file_if_present(stale_destination_path);
    return (1);
}

FT_TEST(test_cmp_file_copy_success_clears_errno, "cmp_file_copy clears errno on success")
{
    const char *source_path = "cmp_file_copy_success_source.txt";
    const char *destination_path = "cmp_file_copy_success_destination.txt";
    int error_code = FT_ERR_INVALID_ARGUMENT;

    remove_file_if_present(source_path);
    remove_file_if_present(destination_path);
    write_test_file(source_path);
    FT_ASSERT_EQ(0, cmp_file_copy(source_path, destination_path, &error_code));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, error_code);
    cmp_file_delete(source_path, &error_code);
    cmp_file_delete(destination_path, &error_code);
    return (1);
}

FT_TEST(test_cmp_file_create_directory_null_pointer_sets_errno, "cmp_file_create_directory reports FT_ERR_INVALID_ARGUMENT for null path")
{
    int error_code = FT_ERR_SUCCESSS;

    FT_ASSERT_EQ(-1, cmp_file_create_directory(ft_nullptr, 0700, &error_code));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_cmp_file_create_directory_permission_error_sets_errno, "cmp_file_create_directory reports permission errors")
{
    const char *parent_file = "cmp_file_create_directory_permission_parent_file.txt";
    const char *child_directory = "cmp_file_create_directory_permission_parent_file.txt/child";
    int error_code = FT_ERR_SUCCESSS;

    remove_directory_if_present(child_directory);
    remove_file_if_present(parent_file);
    write_test_file(parent_file);
    FT_ASSERT_EQ(-1, cmp_file_create_directory(child_directory, 0700, &error_code));
    FT_ASSERT_EQ(FT_ERR_IO, error_code);
    remove_directory_if_present(child_directory);
    remove_file_if_present(parent_file);
    return (1);
}

FT_TEST(test_cmp_file_create_directory_existing_path_sets_errno, "cmp_file_create_directory reports EEXIST")
{
    const char *directory_path = "cmp_file_create_directory_existing";
    int error_code = FT_ERR_SUCCESSS;

    remove_directory_if_present(directory_path);
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(0, _mkdir(directory_path));
#else
    FT_ASSERT_EQ(0, ::mkdir(directory_path, 0700));
#endif
    FT_ASSERT_EQ(-1, cmp_file_create_directory(directory_path, 0700, &error_code));
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, error_code);
    remove_directory_if_present(directory_path);
    return (1);
}

FT_TEST(test_cmp_file_create_directory_success_clears_errno, "cmp_file_create_directory clears errno on success")
{
    const char *directory_path = "cmp_file_create_directory_success";
    int error_code = FT_ERR_INVALID_ARGUMENT;

    remove_directory_if_present(directory_path);
    FT_ASSERT_EQ(0, cmp_file_create_directory(directory_path, 0700, &error_code));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, error_code);
    remove_directory_if_present(directory_path);
    return (1);
}
