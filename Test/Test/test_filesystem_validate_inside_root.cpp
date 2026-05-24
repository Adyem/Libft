#include "../test_internal.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_filesystem_validate_inside_root_reports_invalid_path)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, filesystem_validate_inside_root("root",
        "root/child/file.txt"));
    FT_ASSERT_EQ(FT_ERR_INVALID_PATH, filesystem_validate_inside_root("root",
        "rooted/file.txt"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_inside_root("root", "rooted/file.txt"));
    return (1);
}
