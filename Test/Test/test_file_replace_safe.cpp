#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_file_replace_safe_writes_and_replaces_content)
{
    const char *path;
    ft_string content;

    path = "test_file_replace_safe.txt";
    (void)file_delete(path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(path, "old", 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_replace_safe(path, "new-safe", 8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_read_all(path, content));
    FT_ASSERT_EQ(FT_TRUE, content == "new-safe");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete(path));
    return (1);
}

FT_TEST(test_file_replace_safe_rejects_invalid_arguments)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, file_replace_safe(ft_nullptr, "x", 1));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, file_replace_safe("test_invalid.txt",
            ft_nullptr, 1));
    return (1);
}
