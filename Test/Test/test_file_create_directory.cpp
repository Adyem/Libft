#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/open_dir.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_file_create_directory_reports_missing_parent_failure)
{
    int32_t error_code;

    error_code = file_create_directory("test_file_failure_missing_parent/child", 0700);
    FT_ASSERT(error_code != FT_ERR_SUCCESS);
    return (1);
}
