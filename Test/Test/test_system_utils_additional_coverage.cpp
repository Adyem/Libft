#include "../test_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdio>
#include <fcntl.h>

#ifndef LIBFT_TEST_BUILD
#endif

static void cleanup_system_utils_additional_file(const char *path)
{
    if (path == ft_nullptr)
        return ;
    std::remove(path);
    return ;
}

static int create_system_utils_additional_file(const char *path)
{
    FILE *file_stream;

    file_stream = std::fopen(path, "w");
    if (file_stream == ft_nullptr)
        return (-1);
    std::fputs("system_utils\n", file_stream);
    std::fclose(file_stream);
    return (0);
}

FT_TEST(test_su_open_null_path_returns_negative, "su_open rejects null path")
{
    FT_ASSERT_EQ(-1, su_open(ft_nullptr));
    return (1);
}

FT_TEST(test_su_open_with_flags_null_path_returns_negative, "su_open(flags) rejects null path")
{
    FT_ASSERT_EQ(-1, su_open(ft_nullptr, O_RDONLY));
    return (1);
}

FT_TEST(test_su_open_with_mode_null_path_returns_negative, "su_open(mode) rejects null path")
{
    FT_ASSERT_EQ(-1, su_open(ft_nullptr, O_RDONLY, 0644));
    return (1);
}

FT_TEST(test_su_close_invalid_descriptor_returns_negative, "su_close rejects invalid descriptor")
{
    FT_ASSERT_EQ(-1, su_close(-1));
    return (1);
}

FT_TEST(test_su_write_zero_count_returns_zero, "su_write returns zero for empty writes")
{
    const char *buffer_data;

    buffer_data = "abc";
    FT_ASSERT_EQ(0, su_write(-1, buffer_data, 0));
    return (1);
}

FT_TEST(test_su_fopen_null_path_returns_null, "su_fopen rejects null path")
{
    FT_ASSERT_EQ(ft_nullptr, su_fopen(ft_nullptr));
    return (1);
}

FT_TEST(test_su_fopen_flags_null_path_returns_null, "su_fopen(flags) rejects null path")
{
    FT_ASSERT_EQ(ft_nullptr, su_fopen(ft_nullptr, O_RDONLY));
    return (1);
}

FT_TEST(test_su_fopen_mode_null_path_returns_null, "su_fopen(mode) rejects null path")
{
    FT_ASSERT_EQ(ft_nullptr, su_fopen(ft_nullptr, O_RDONLY, 0644));
    return (1);
}

FT_TEST(test_su_fclose_null_stream_returns_negative, "su_fclose rejects null stream")
{
    FT_ASSERT_EQ(-1, su_fclose(ft_nullptr));
    return (1);
}

FT_TEST(test_su_fread_null_buffer_returns_zero, "su_fread rejects null buffer")
{
    FT_ASSERT_EQ(0, su_fread(ft_nullptr, 1, 1, ft_nullptr));
    return (1);
}

FT_TEST(test_su_fread_null_stream_returns_zero, "su_fread rejects null stream")
{
    char buffer_data[4];

    FT_ASSERT_EQ(0, su_fread(buffer_data, 1, 1, ft_nullptr));
    return (1);
}

FT_TEST(test_su_fwrite_null_buffer_returns_zero, "su_fwrite rejects null buffer")
{
    FT_ASSERT_EQ(0, su_fwrite(ft_nullptr, 1, 1, ft_nullptr));
    return (1);
}

FT_TEST(test_su_fwrite_null_stream_returns_zero, "su_fwrite rejects null stream")
{
    char buffer_data[4];

    buffer_data[0] = 'a';
    buffer_data[1] = 'b';
    buffer_data[2] = 'c';
    buffer_data[3] = '\0';
    FT_ASSERT_EQ(0, su_fwrite(buffer_data, 1, 3, ft_nullptr));
    return (1);
}

FT_TEST(test_su_fseek_null_stream_returns_negative, "su_fseek rejects null stream")
{
    FT_ASSERT_EQ(-1, su_fseek(ft_nullptr, 0, SEEK_SET));
    return (1);
}

FT_TEST(test_su_ftell_null_stream_returns_negative, "su_ftell rejects null stream")
{
    FT_ASSERT_EQ(-1L, su_ftell(ft_nullptr));
    return (1);
}

FT_TEST(test_su_copy_file_null_source_returns_negative, "su_copy_file rejects null source")
{
    FT_ASSERT_EQ(-1, su_copy_file(ft_nullptr, "target.txt"));
    return (1);
}

FT_TEST(test_su_copy_file_null_destination_returns_negative, "su_copy_file rejects null destination")
{
    FT_ASSERT_EQ(-1, su_copy_file("source.txt", ft_nullptr));
    return (1);
}

FT_TEST(test_su_copy_directory_null_source_returns_negative, "su_copy_directory_recursive rejects null source")
{
    FT_ASSERT_EQ(-1, su_copy_directory_recursive(ft_nullptr, "destination"));
    return (1);
}

FT_TEST(test_su_copy_directory_null_destination_returns_negative, "su_copy_directory_recursive rejects null destination")
{
    FT_ASSERT_EQ(-1, su_copy_directory_recursive("source", ft_nullptr));
    return (1);
}

FT_TEST(test_su_inspect_permissions_null_path_returns_negative, "su_inspect_permissions rejects null path")
{
    mode_t permissions;

    permissions = 0;
    FT_ASSERT_EQ(-1, su_inspect_permissions(ft_nullptr, &permissions));
    return (1);
}

FT_TEST(test_su_inspect_permissions_null_output_returns_negative, "su_inspect_permissions rejects null output")
{
    FT_ASSERT_EQ(-1, su_inspect_permissions("any_path", ft_nullptr));
    return (1);
}

FT_TEST(test_su_chmod_null_path_returns_negative, "su_chmod rejects null path")
{
    FT_ASSERT_EQ(-1, su_chmod(ft_nullptr, 6, 4, 4));
    return (1);
}

FT_TEST(test_su_chmod_invalid_owner_permissions_returns_negative, "su_chmod rejects invalid owner permissions")
{
    FT_ASSERT_EQ(-1, su_chmod("any_path", -1, 4, 4));
    return (1);
}

FT_TEST(test_su_chmod_invalid_group_permissions_returns_negative, "su_chmod rejects invalid group permissions")
{
    FT_ASSERT_EQ(-1, su_chmod("any_path", 6, 9, 4));
    return (1);
}

FT_TEST(test_su_chmod_invalid_other_permissions_returns_negative, "su_chmod rejects invalid other permissions")
{
    FT_ASSERT_EQ(-1, su_chmod("any_path", 6, 4, 10));
    return (1);
}

FT_TEST(test_su_get_home_directory_returns_value, "su_get_home_directory returns a non-null pointer")
{
    FT_ASSERT(su_get_home_directory() != ft_nullptr);
    return (1);
}

FT_TEST(test_su_chmod_valid_file_returns_zero, "su_chmod applies permissions for a regular file")
{
    const char *path;

    path = "system_utils_additional_permissions.txt";
    cleanup_system_utils_additional_file(path);
    FT_ASSERT_EQ(0, create_system_utils_additional_file(path));
    FT_ASSERT_EQ(0, su_chmod(path, 6, 4, 4));
    cleanup_system_utils_additional_file(path);
    return (1);
}
