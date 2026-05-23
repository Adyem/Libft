#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_file_read_all_round_trips_written_content)
{
    const char *file_path;
    ft_string content;

    file_path = "test_file_read_all.txt";
    (void)file_delete(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, "hello file", 10));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_read_all(file_path, content));
    FT_ASSERT_EQ(FT_TRUE, content == "hello file");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete(file_path));
    return (1);
}

FT_TEST(test_file_read_all_rejects_null_path)
{
    ft_string content;

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, file_read_all(ft_nullptr, content));
    return (1);
}
