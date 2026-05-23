#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Errno/errno.hpp"

static ft_bool test_file_list_directory_contains(ft_vector<ft_string> &entries,
    const char *needle)
{
    ft_size_t index;

    index = 0;
    while (index < entries.size())
    {
        if (entries[index] == needle)
            return (FT_TRUE);
        ++index;
    }
    return (FT_FALSE);
}

FT_TEST(test_file_list_directory_filters_entries)
{
    ft_vector<ft_string> entries;

    (void)file_delete_recursive("test_file_list_directory_root");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_create_directories("test_file_list_directory_root/child", 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all("test_file_list_directory_root/value.txt", "x", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all("test_file_list_directory_root/.hidden", "x", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_list_directory("test_file_list_directory_root",
            entries, FILE_LIST_INCLUDE_FILES));
    FT_ASSERT_EQ(FT_TRUE, test_file_list_directory_contains(entries,
            "test_file_list_directory_root/value.txt"));
    FT_ASSERT_EQ(FT_FALSE, test_file_list_directory_contains(entries,
            "test_file_list_directory_root/child"));
    FT_ASSERT_EQ(FT_FALSE, test_file_list_directory_contains(entries,
            "test_file_list_directory_root/.hidden"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete_recursive("test_file_list_directory_root"));
    return (1);
}

FT_TEST(test_file_list_directory_reports_not_directory_for_missing_path)
{
    ft_vector<ft_string> entries;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_DIRECTORY, file_list_directory("test_file_list_directory_missing",
            entries, FILE_LIST_INCLUDE_ALL));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.destroy());
    return (1);
}
