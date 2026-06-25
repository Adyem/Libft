#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

static ft_bool test_file_list_directory_recursive_contains(ft_vector<ft_string> &entries,
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

FT_TEST(test_file_list_directory_recursive_includes_children_and_hidden)
{
    ft_vector<ft_string> entries;

    (void)file_delete_recursive("test_file_list_recursive_root");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_create_directories("test_file_list_recursive_root/child", 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all("test_file_list_recursive_root/child/value.txt", "x", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all("test_file_list_recursive_root/.hidden", "x", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_list_directory_recursive("test_file_list_recursive_root",
            entries, FILE_LIST_INCLUDE_ALL));
    FT_ASSERT_EQ(FT_TRUE, test_file_list_directory_recursive_contains(entries,
            "test_file_list_recursive_root/child"));
    FT_ASSERT_EQ(FT_TRUE, test_file_list_directory_recursive_contains(entries,
            "test_file_list_recursive_root/child/value.txt"));
    FT_ASSERT_EQ(FT_TRUE, test_file_list_directory_recursive_contains(entries,
            "test_file_list_recursive_root/.hidden"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, entries.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete_recursive("test_file_list_recursive_root"));
    return (1);
}
