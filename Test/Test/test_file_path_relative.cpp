#include "../test_internal.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_file_path_relative_returns_sibling_path)
{
    ft_string *relative_path;

    relative_path = file_path_relative("/workspace/src", "/workspace/tests/case.txt");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == "../tests/case.txt");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, relative_path->destroy());
    delete relative_path;
    relative_path = file_path_relative("/workspace/src", "/workspace/src/assets");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == "assets");
    relative_path->destroy();
    delete relative_path;
    relative_path = file_path_relative("/workspace/src", "/workspace/src");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == ".");
    relative_path->destroy();
    delete relative_path;
    return (1);
}

FT_TEST(test_file_path_relative_handles_multiple_ancestor_segments)
{
    ft_string *relative_path;

    relative_path = file_path_relative("/workspace/src/deep",
            "/workspace/tests/case.txt");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == "../../tests/case.txt");
    relative_path->destroy();
    delete relative_path;
    return (1);
}

FT_TEST(test_file_path_relative_rejects_null_inputs)
{
    FT_ASSERT(file_path_relative(ft_nullptr, "/workspace/file") == ft_nullptr);
    FT_ASSERT(file_path_relative("/workspace", ft_nullptr) == ft_nullptr);
    return (1);
}
