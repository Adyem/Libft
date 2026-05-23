#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_file_get_size_reports_written_file_size)
{
    const char *file_path;
    ft_size_t file_size;

    file_path = "test_file_get_size_value.txt";
    (void)file_delete(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, "status", 6));
    file_size = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_get_size(file_path, &file_size));
    FT_ASSERT_EQ(static_cast<ft_size_t>(6), file_size);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete(file_path));
    return (1);
}

FT_TEST(test_file_get_size_rejects_invalid_arguments)
{
    ft_size_t file_size;

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, file_get_size(ft_nullptr, &file_size));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, file_get_size("test_invalid.txt", ft_nullptr));
    return (1);
}
