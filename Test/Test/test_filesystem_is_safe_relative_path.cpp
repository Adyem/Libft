#include "../test_internal.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_filesystem_is_safe_relative_path_rejects_escape)
{
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_safe_relative_path("assets/image.png"));
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_safe_relative_path(
        "assets/../image.png"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_safe_relative_path("../secret.txt"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_safe_relative_path(
        "assets/../../secret.txt"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_safe_relative_path("/tmp/secret.txt"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_safe_relative_path(ft_nullptr));
    return (1);
}
