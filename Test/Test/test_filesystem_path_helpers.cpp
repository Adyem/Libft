#include "../test_internal.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_filesystem_split_path_returns_directory_and_basename)
{
    ft_string directory;
    ft_string basename;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, directory.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, basename.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, filesystem_split_path(
        "/tmp/example.txt", &directory, &basename));
    FT_ASSERT_EQ(0, ft_strcmp(directory.c_str(), "/tmp"));
    FT_ASSERT_EQ(0, ft_strcmp(basename.c_str(), "example.txt"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, directory.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, basename.destroy());
    return (1);
}

FT_TEST(test_filesystem_hidden_and_rooted_checks)
{
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_hidden(".env"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_hidden("visible.txt"));
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_rooted("/tmp"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_rooted("relative/path"));
    return (1);
}

FT_TEST(test_filesystem_reserved_name_checks_windows_device_names)
{
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_reserved_name("CON"));
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_reserved_name("com1.txt"));
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_reserved_name("LPT9"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_reserved_name("notes.txt"));
    FT_ASSERT_EQ(FT_FALSE, filesystem_is_reserved_name(ft_nullptr));
    return (1);
}

