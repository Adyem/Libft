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

FT_TEST(test_file_path_relative_rejects_different_drive_roots)
{
    FT_ASSERT(file_path_relative("C:/project/src", "D:/assets/file.png")
        == ft_nullptr);
    return (1);
}

FT_TEST(test_file_path_relative_rejects_absolute_relative_mix)
{
    FT_ASSERT(file_path_relative("/workspace/src", "assets/file.png")
        == ft_nullptr);
    return (1);
}

FT_TEST(test_file_path_relative_rejects_different_unc_roots)
{
    FT_ASSERT(file_path_relative("//server_a/share/src",
            "//server_b/share/assets/file.png") == ft_nullptr);
    return (1);
}

FT_TEST(test_file_path_relative_compares_complete_components)
{
    ft_string *relative_path;

    relative_path = file_path_relative("/home/user/game",
            "/home/user/games/assets");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == "../games/assets");
    relative_path->destroy();
    delete relative_path;
    relative_path = file_path_relative("/foo/bar", "/foo/barn/file");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == "../barn/file");
    relative_path->destroy();
    delete relative_path;
    return (1);
}

FT_TEST(test_file_path_relative_windows_drive_root_is_case_insensitive)
{
    ft_string *relative_path;

    relative_path = file_path_relative("C:/Project/Src", "c:/project/assets");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == "../assets");
    relative_path->destroy();
    delete relative_path;
    return (1);
}

FT_TEST(test_file_path_relative_windows_unc_root_is_case_insensitive)
{
    ft_string *relative_path;

    relative_path = file_path_relative("//Server/Share/Src",
            "//server/share/assets/file.txt");
    FT_ASSERT(relative_path != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *relative_path == "../assets/file.txt");
    relative_path->destroy();
    delete relative_path;
    return (1);
}
