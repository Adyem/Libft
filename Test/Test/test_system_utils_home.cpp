#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
static void capture_environment_value(const char *name, std::string &storage, int &was_present)
{
    char    *current_value;

    current_value = ft_getenv(name);
    if (current_value != ft_nullptr)
    {
        was_present = 1;
        storage = current_value;
    }
    else
    {
        was_present = 0;
        storage.clear();
    }
    return ;
}

static void restore_environment_value(const char *name, const std::string &storage, int was_present)
{
    if (was_present != 0)
    {
        FT_ASSERT_EQ(0, cmp_setenv(name, storage.c_str(), 1));
        return ;
    }
    FT_ASSERT_EQ(0, cmp_unsetenv(name));
    return ;
}
#endif

FT_TEST(test_su_get_home_directory_windows_missing_guard_sets_errno,
        "su_get_home_directory missing HOMEPATH sets FT_ERR_INVALID_ARGUMENT")
{
#if defined(_WIN32) || defined(_WIN64)
    std::string    original_userprofile;
    std::string    original_home_drive;
    std::string    original_home_path;
    int            original_userprofile_present;
    int            original_home_drive_present;
    int            original_home_path_present;
    char           *home_directory;

    capture_environment_value("USERPROFILE", original_userprofile,
                              original_userprofile_present);
    capture_environment_value("HOMEDRIVE", original_home_drive,
                              original_home_drive_present);
    capture_environment_value("HOMEPATH", original_home_path,
                              original_home_path_present);
    FT_ASSERT_EQ(0, cmp_unsetenv("USERPROFILE"));
    FT_ASSERT_EQ(0, cmp_setenv("HOMEDRIVE", "C:", 1));
    FT_ASSERT_EQ(0, cmp_unsetenv("HOMEPATH"));
    ft_errno = ER_SUCCESS;
    home_directory = su_get_home_directory();
    FT_ASSERT_EQ(ft_nullptr, home_directory);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    restore_environment_value("USERPROFILE", original_userprofile,
                              original_userprofile_present);
    restore_environment_value("HOMEDRIVE", original_home_drive,
                              original_home_drive_present);
    restore_environment_value("HOMEPATH", original_home_path,
                              original_home_path_present);
    return (1);
#else
    return (1);
#endif
}

FT_TEST(test_su_get_home_directory_windows_concatenates_success,
        "su_get_home_directory concatenates HOMEDRIVE and HOMEPATH")
{
#if defined(_WIN32) || defined(_WIN64)
    std::string    original_userprofile;
    std::string    original_home_drive;
    std::string    original_home_path;
    int            original_userprofile_present;
    int            original_home_drive_present;
    int            original_home_path_present;
    std::string    forced_home_drive;
    std::string    forced_home_path;
    std::string    expected_home_directory;
    char           *home_directory;

    forced_home_drive = "Z:";
    forced_home_path = "\\ForcedPath";
    expected_home_directory = forced_home_drive + forced_home_path;
    capture_environment_value("USERPROFILE", original_userprofile,
                              original_userprofile_present);
    capture_environment_value("HOMEDRIVE", original_home_drive,
                              original_home_drive_present);
    capture_environment_value("HOMEPATH", original_home_path,
                              original_home_path_present);
    FT_ASSERT_EQ(0, cmp_unsetenv("USERPROFILE"));
    FT_ASSERT_EQ(0, cmp_setenv("HOMEDRIVE", forced_home_drive.c_str(), 1));
    FT_ASSERT_EQ(0, cmp_setenv("HOMEPATH", forced_home_path.c_str(), 1));
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    home_directory = su_get_home_directory();
    FT_ASSERT(home_directory != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(home_directory,
                                expected_home_directory.c_str()));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(home_directory);
    restore_environment_value("USERPROFILE", original_userprofile,
                              original_userprofile_present);
    restore_environment_value("HOMEDRIVE", original_home_drive,
                              original_home_drive_present);
    restore_environment_value("HOMEPATH", original_home_path,
                              original_home_path_present);
    return (1);
#else
    return (1);
#endif
}
