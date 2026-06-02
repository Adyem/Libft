#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Errno/errno.hpp"
#include <sys/types.h>
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

FT_TEST(test_file_get_permissions_reports_existing_file)
{
    const char *file_path;
    mode_t permissions;

    file_path = "test_file_get_permissions_value.txt";
    (void)file_delete(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(file_path, "x", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_get_permissions(file_path, &permissions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete(file_path));
    return (1);
}
