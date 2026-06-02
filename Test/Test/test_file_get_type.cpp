#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include <sys/types.h>
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

FT_TEST(test_file_get_type_reports_regular_directory_and_missing)
{
    const char *directory_path;
    const char *file_path;

    directory_path = "test_file_status_directory";
    file_path = "test_file_status_directory/value.txt";
    (void)file_delete_recursive(directory_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_create_directory(directory_path, 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, "status", 6));
    FT_ASSERT_EQ(FILE_TYPE_DIRECTORY, file_get_type(directory_path));
    FT_ASSERT_EQ(FILE_TYPE_REGULAR, file_get_type(file_path));
    FT_ASSERT_EQ(FILE_TYPE_MISSING, file_get_type("test_file_status_missing.txt"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete_recursive(directory_path));
    return (1);
}
