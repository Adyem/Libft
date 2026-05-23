#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"

FT_TEST(test_file_path_equal_compares_normalized_paths)
{
    FT_ASSERT_EQ(FT_TRUE, file_path_equal("alpha/./beta/../gamma", "alpha/gamma"));
    FT_ASSERT_EQ(FT_FALSE, file_path_equal("alpha/gamma", "alpha/delta"));
    FT_ASSERT_EQ(FT_FALSE, file_path_equal(ft_nullptr, "alpha"));
    return (1);
}
