#include "../../File/file_utils.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cstdio>

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
