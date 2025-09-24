#include "../../File/file_utils.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

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
