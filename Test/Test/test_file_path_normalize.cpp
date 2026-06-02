#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

FT_TEST(test_file_path_normalize_resolves_dot_and_dot_dot)
{
    ft_string *normalized;

    normalized = file_path_normalize("alpha/./beta/../gamma.txt");
    FT_ASSERT(normalized != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, normalized->get_error());
    FT_ASSERT_EQ(FT_TRUE, *normalized == "alpha/gamma.txt");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, normalized->destroy());
    delete normalized;
    normalized = file_path_normalize(".");
    FT_ASSERT(normalized != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *normalized == ".");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, normalized->destroy());
    delete normalized;
    normalized = file_path_normalize("..");
    FT_ASSERT(normalized != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *normalized == "..");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, normalized->destroy());
    delete normalized;
    return (1);
}

FT_TEST(test_file_path_normalize_root_drive_and_unc_edges)
{
    ft_string *normalized;

    normalized = file_path_normalize("/");
    FT_ASSERT(normalized != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, *normalized == "/");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, normalized->destroy());
    delete normalized;
    normalized = file_path_normalize("C:/folder/../file.txt");
    FT_ASSERT(normalized != ft_nullptr);
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(FT_TRUE, *normalized == "C:\\file.txt");
#else
    FT_ASSERT_EQ(FT_TRUE, *normalized == "C:/file.txt");
#endif
    FT_ASSERT_EQ(FT_ERR_SUCCESS, normalized->destroy());
    delete normalized;
    normalized = file_path_normalize("//server//share/file.txt");
    FT_ASSERT(normalized != ft_nullptr);
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(FT_TRUE, *normalized == "\\\\server\\share\\file.txt");
#else
    FT_ASSERT_EQ(FT_TRUE, *normalized == "//server/share/file.txt");
#endif
    FT_ASSERT_EQ(FT_ERR_SUCCESS, normalized->destroy());
    delete normalized;
    return (1);
}
