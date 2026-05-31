#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_file_write_all_atomic_replaces_content)
{
    const char *file_path;
    ft_string content;

    file_path = "test_file_write_all_atomic.txt";
    (void)file_delete(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, "old", 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all_atomic(file_path, "new-value", 9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_read_all(file_path, content));
    FT_ASSERT_EQ(FT_TRUE, content == "new-value");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete(file_path));
    return (1);
}

FT_TEST(test_file_write_all_atomic_cleans_up_temp_file_after_open_failure)
{
    FT_ASSERT_EQ(FT_ERR_FILE_OPEN_FAILED, file_write_all_atomic(
            "test_file_failure_missing_parent/value.txt", "x", 1));
    FT_ASSERT_EQ(FILE_TYPE_MISSING, file_get_type(
            "test_file_failure_missing_parent/value.txt.tmp"));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, file_write_all_atomic("test_invalid.txt",
            ft_nullptr, 1));
    return (1);
}
