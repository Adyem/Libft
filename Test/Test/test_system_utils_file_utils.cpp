#include "../test_internal.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef LIBFT_TEST_BUILD
#endif

#if defined(_WIN32) || defined(_WIN64)
# include <direct.h>
#else
# include <unistd.h>
#endif

static void su_cleanup_path(const char *path)
{
    if (path == ft_nullptr)
        return ;
    std::remove(path);
    return ;
}

static void su_remove_directory(const char *path)
{
#if defined(_WIN32) || defined(_WIN64)
    _rmdir(path);
#else
    rmdir(path);
#endif
    return ;
}

static int su_write_text_file(const char *path, const char *contents)
{
    su_file *file_stream;
    size_t text_length;
    size_t written;

    file_stream = su_fopen(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_stream == ft_nullptr)
    {
        ft_global_error_stack_drop_last_error();
        return (-1);
    }
    ft_global_error_stack_drop_last_error();
    text_length = std::strlen(contents);
    if (text_length == 0)
    {
        su_fclose(file_stream);
        ft_global_error_stack_drop_last_error();
        return (0);
    }
    written = su_fwrite(contents, 1, text_length, file_stream);
    ft_global_error_stack_drop_last_error();
    if (written != text_length)
    {
        su_fclose(file_stream);
        ft_global_error_stack_drop_last_error();
        return (-1);
    }
    if (su_fclose(file_stream) != 0)
    {
        ft_global_error_stack_drop_last_error();
        return (-1);
    }
    ft_global_error_stack_drop_last_error();
    return (0);
}

static ft_string su_read_text_file(const char *path)
{
    su_file *file_stream;
    char buffer[128];
    ft_string result;
    int error_code;

    file_stream = su_fopen(path, O_RDONLY);
    if (file_stream == ft_nullptr)
    {
        error_code = ft_global_error_stack_drop_last_error();
        return (ft_string(error_code));
    }
    ft_global_error_stack_drop_last_error();
    result = ft_string();
    if (result.get_error())
    {
        su_fclose(file_stream);
        ft_global_error_stack_drop_last_error();
        return (result);
    }
    while (true)
    {
        size_t bytes_read = su_fread(buffer, 1, sizeof(buffer), file_stream);
        if (bytes_read == 0)
        {
            error_code = ft_global_error_stack_drop_last_error();
            if (error_code == FT_ERR_SUCCESS)
                break;
            result = ft_string(error_code);
            break;
        }
        ft_global_error_stack_drop_last_error();
        size_t index = 0;
        while (index < bytes_read)
        {
            result.append(buffer[index]);
            if (result.get_error())
                break;
            index += 1;
        }
        if (result.get_error())
            break;
    }
    su_fclose(file_stream);
    ft_global_error_stack_drop_last_error();
    return (result);
}

static void su_prepare_directory_fixture(void)
{
    int error_code = FT_ERR_SUCCESS;

    cmp_file_create_directory("su_copy_dir_source", 0755, &error_code);
    cmp_file_create_directory("su_copy_dir_source/nested", 0755, &error_code);
    su_write_text_file("su_copy_dir_source/root.txt", "root payload\n");
    su_write_text_file("su_copy_dir_source/nested/item.txt", "nested payload\n");
    return ;
}

static void su_cleanup_directory_fixture(void)
{
    su_cleanup_path("su_copy_dir_source/root.txt");
    su_cleanup_path("su_copy_dir_source/nested/item.txt");
    su_remove_directory("su_copy_dir_source/nested");
    su_remove_directory("su_copy_dir_source");
    su_cleanup_path("su_copy_dir_destination/root.txt");
    su_cleanup_path("su_copy_dir_destination/nested/item.txt");
    su_remove_directory("su_copy_dir_destination/nested");
    su_remove_directory("su_copy_dir_destination");
    return ;
}

FT_TEST(test_su_copy_file_copies_contents, "su_copy_file copies file payloads")
{
    const char *source_path = "su_copy_file_source.txt";
    const char *destination_path = "su_copy_file_destination.txt";
    ft_string contents;

    su_cleanup_path(source_path);
    su_cleanup_path(destination_path);
    FT_ASSERT_EQ(0, su_write_text_file(source_path, "example payload"));
    FT_ASSERT_EQ(0, su_copy_file(source_path, destination_path));
    contents = su_read_text_file(destination_path);
    FT_ASSERT_EQ(0, contents.get_error());
    FT_ASSERT(strcmp(contents.c_str(), "example payload") == 0);
    su_cleanup_path(source_path);
    su_cleanup_path(destination_path);
    return (1);
}

FT_TEST(test_su_copy_directory_recursive_replicates_structure, "su_copy_directory_recursive copies nested directories")
{
    ft_string contents;

    su_cleanup_directory_fixture();
    su_prepare_directory_fixture();
    su_remove_directory("su_copy_dir_destination");
    FT_ASSERT_EQ(0, su_copy_directory_recursive("su_copy_dir_source", "su_copy_dir_destination"));
    contents = su_read_text_file("su_copy_dir_destination/root.txt");
    FT_ASSERT_EQ(0, contents.get_error());
    FT_ASSERT(strcmp(contents.c_str(), "root payload\n") == 0);
    contents = su_read_text_file("su_copy_dir_destination/nested/item.txt");
    FT_ASSERT_EQ(0, contents.get_error());
    FT_ASSERT(strcmp(contents.c_str(), "nested payload\n") == 0);
    su_cleanup_directory_fixture();
    return (1);
}

FT_TEST(test_su_inspect_permissions_reports_mode_bits, "su_inspect_permissions returns stat mode")
{
    const char *path = "su_permissions_target.txt";
    mode_t permissions;

    su_cleanup_path(path);
    FT_ASSERT_EQ(0, su_write_text_file(path, "permissions"));
    FT_ASSERT_EQ(0, su_inspect_permissions(path, &permissions));
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT((permissions & _S_IFREG) != 0);
#else
    FT_ASSERT((permissions & S_IFREG) != 0);
#endif
    su_cleanup_path(path);
    return (1);
}

FT_TEST(test_su_copy_directory_handles_invalid_arguments, "su_copy_directory_recursive validates inputs")
{
    FT_ASSERT_EQ(-1, su_copy_directory_recursive(ft_nullptr, "target"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_global_error_stack_drop_last_error());
    FT_ASSERT_EQ(-1, su_copy_directory_recursive("missing", "target"));
    FT_ASSERT(ft_global_error_stack_drop_last_error() != FT_ERR_SUCCESS);
    return (1);
}

FT_TEST(test_su_inspect_permissions_rejects_null_output, "su_inspect_permissions rejects missing destination")
{
    FT_ASSERT_EQ(-1, su_inspect_permissions("any", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_global_error_stack_drop_last_error());
    return (1);
}
