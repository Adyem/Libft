#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

FT_TEST(test_file_copy_rejects_directory_source_as_file)
{
    int32_t error_code;

    (void)file_delete_recursive("test_file_copy_directory_as_file");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_create_directories(
            "test_file_copy_directory_as_file/source", 0700));
    error_code = file_copy("test_file_copy_directory_as_file/source",
            "test_file_copy_directory_as_file/copied_directory_as_file");
    FT_ASSERT(error_code != FT_ERR_SUCCESS);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete_recursive("test_file_copy_directory_as_file"));
    return (1);
}
