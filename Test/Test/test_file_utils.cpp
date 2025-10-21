#include "../../File/file_utils.hpp"
#include "../../File/open_dir.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../Compatebility/compatebility_system_test_hooks.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cstddef>
#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <unistd.h>
#endif

static void remove_directory_if_present(const char *directory_path)
{
    if (cmp_directory_exists(directory_path) != 1)
        return ;
#if defined(_WIN32) || defined(_WIN64)
    RemoveDirectoryA(directory_path);
#else
    rmdir(directory_path);
#endif
    return ;
}

static void create_cross_device_test_file(const char *path)
{
    FILE *file_pointer;

    file_pointer = ft_fopen(path, "w");
    if (file_pointer != ft_nullptr)
    {
        std::fputs("cross-device move payload", file_pointer);
        ft_fclose(file_pointer);
    }
    return ;
}

static void build_path_from_template(char *destination, const char *path_template, char separator)
{
    size_t index;

    index = 0;
    while (path_template[index] != '\0')
    {
        if (path_template[index] == '/')
            destination[index] = separator;
        else
            destination[index] = path_template[index];
        ++index;
    }
    destination[index] = '\0';
    return ;
}

static void build_native_path(char *destination, const char *path_template)
{
    build_path_from_template(destination, path_template, cmp_path_separator());
    return ;
}

static void build_alternate_separator_path(char *destination, const char *path_template)
{
    char path_separator;
    char alternate_separator;

    path_separator = cmp_path_separator();
    if (path_separator == '/')
        alternate_separator = '\\';
    else
        alternate_separator = '/';
    build_path_from_template(destination, path_template, alternate_separator);
    return ;
}

FT_TEST(test_file_path_join_prefers_absolute_right, "file_path_join returns absolute right operand")
{
    ft_string result = file_path_join("/etc", "/var/log");

    FT_ASSERT_EQ(ER_SUCCESS, result.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), "/var/log"));
    return (1);
}

FT_TEST(test_file_path_join_keeps_drive_letter, "file_path_join keeps Windows drive absolute path")
{
    ft_string result = file_path_join("/left", "C:/temp");

    FT_ASSERT_EQ(ER_SUCCESS, result.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), "C:/temp"));
    return (1);
}

FT_TEST(test_file_path_normalize_collapses_duplicates, "file_path_normalize collapses duplicate separators")
{
    char path_buffer[64];
    char expected_buffer[64];
    ft_string normalized;

    build_alternate_separator_path(path_buffer, "/folder///sub///file.txt");
    normalized = file_path_normalize(path_buffer);
    FT_ASSERT_EQ(ER_SUCCESS, normalized.get_error());
    build_native_path(expected_buffer, "/folder/sub/file.txt");
    FT_ASSERT_EQ(0, ft_strcmp(normalized.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_path_normalize_preserves_trailing_separator, "file_path_normalize preserves trailing separators")
{
    char path_buffer[64];
    char expected_buffer[64];
    ft_string normalized;

    build_alternate_separator_path(path_buffer, "/folder/subdir///");
    normalized = file_path_normalize(path_buffer);
    FT_ASSERT_EQ(ER_SUCCESS, normalized.get_error());
    build_native_path(expected_buffer, "/folder/subdir/");
    FT_ASSERT_EQ(0, ft_strcmp(normalized.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_path_normalize_handles_null_input, "file_path_normalize gracefully handles null input")
{
    ft_string normalized;

    normalized = file_path_normalize(ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, normalized.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(normalized.c_str(), ""));
    return (1);
}

FT_TEST(test_file_path_join_appends_missing_separator, "file_path_join inserts missing separator between components")
{
    char left_buffer[64];
    char right_buffer[64];
    char expected_buffer[128];
    ft_string result;

    build_alternate_separator_path(left_buffer, "/var//log");
    build_alternate_separator_path(right_buffer, "nginx///access.log");
    result = file_path_join(left_buffer, right_buffer);
    FT_ASSERT_EQ(ER_SUCCESS, result.get_error());
    build_native_path(expected_buffer, "/var/log/nginx/access.log");
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_path_join_with_empty_left, "file_path_join handles empty left operand")
{
    char right_buffer[64];
    char expected_buffer[64];
    ft_string result;

    build_alternate_separator_path(right_buffer, "folder//file.txt");
    result = file_path_join("", right_buffer);
    FT_ASSERT_EQ(ER_SUCCESS, result.get_error());
    build_native_path(expected_buffer, "folder/file.txt");
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_move_cross_device_fallback, "file_move falls back to copy and delete when forced cross-device")
{
    const char *source_path = "test_file_move_cross_device_source.txt";
    const char *destination_path = "test_file_move_cross_device_destination.txt";
    FILE *destination_file;
    char destination_buffer[64];

    file_delete(source_path);
    file_delete(destination_path);
    create_cross_device_test_file(source_path);
    cmp_set_force_cross_device_move(1);
    FT_ASSERT_EQ(0, file_move(source_path, destination_path));
    cmp_set_force_cross_device_move(0);
    FT_ASSERT_EQ(1, file_exists(destination_path));
    FT_ASSERT_EQ(0, file_exists(source_path));
    destination_file = ft_fopen(destination_path, "r");
    FT_ASSERT(destination_file != ft_nullptr);
    if (destination_file == ft_nullptr)
        return (0);
    FT_ASSERT(std::fgets(destination_buffer, 64, destination_file) != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(destination_buffer, "cross-device move payload"));
    FT_ASSERT_EQ(FT_SUCCESS, ft_fclose(destination_file));
    FT_ASSERT_EQ(0, file_delete(destination_path));
    return (1);
}

FT_TEST(test_file_dir_exists_matches_file_exists_semantics, "file_dir_exists returns 1 when directory exists")
{
    const char *directory_path = "test_file_dir_exists_matches_file_exists_semantics";

    remove_directory_if_present(directory_path);
    FT_ASSERT_EQ(0, file_dir_exists(directory_path));
    FT_ASSERT_EQ(0, file_create_directory(directory_path, 0777));
    FT_ASSERT_EQ(1, file_dir_exists(directory_path));
    remove_directory_if_present(directory_path);
    FT_ASSERT_EQ(0, file_dir_exists(directory_path));
    return (1);
}

FT_TEST(test_file_readdir_handles_null_stream, "file_readdir sets FT_ERR_INVALID_ARGUMENT when stream is null")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, file_readdir(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_file_readdir_clears_error_on_success, "file_readdir sets ER_SUCCESS after reading entry")
{
    file_dir *directory_stream;
    file_dirent *directory_entry;

    directory_stream = file_opendir(".");
    FT_ASSERT(directory_stream != ft_nullptr);
    if (directory_stream == ft_nullptr)
        return (0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    directory_entry = file_readdir(directory_stream);
    FT_ASSERT(directory_entry != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, file_closedir(directory_stream));
    return (1);
}

FT_TEST(test_file_closedir_handles_null_stream, "file_closedir sets FT_ERR_INVALID_ARGUMENT when stream is null")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, file_closedir(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_file_closedir_clears_error_on_success, "file_closedir sets ER_SUCCESS after close")
{
    file_dir *directory_stream;

    directory_stream = file_opendir(".");
    FT_ASSERT(directory_stream != ft_nullptr);
    if (directory_stream == ft_nullptr)
        return (0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, file_closedir(directory_stream));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
