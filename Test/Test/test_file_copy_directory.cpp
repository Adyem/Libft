#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_file_copy_directory_copies_nested_files)
{
    const char *source_path;
    const char *destination_path;
    ft_string *source_child_directory;
    ft_string *source_child_file;
    ft_string *destination_child_file;
    ft_string content;

    source_path = "test_file_copy_directory_source";
    destination_path = "test_file_copy_directory_destination";
    (void)file_delete_recursive(source_path);
    (void)file_delete_recursive(destination_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_create_directories("test_file_copy_directory_source/child", 0700));
    source_child_directory = file_path_join(source_path, "child");
    FT_ASSERT(source_child_directory != ft_nullptr);
    source_child_file = file_path_join(source_child_directory->c_str(), "value.txt");
    FT_ASSERT(source_child_file != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(source_child_file->c_str(), "copy", 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_copy_directory(source_path, destination_path));
    destination_child_file = file_path_join(destination_path, "child/value.txt");
    FT_ASSERT(destination_child_file != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_read_all(destination_child_file->c_str(), content));
    FT_ASSERT_EQ(FT_TRUE, content == "copy");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_child_file->destroy());
    delete source_child_file;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_child_directory->destroy());
    delete source_child_directory;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_child_file->destroy());
    delete destination_child_file;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete_recursive(source_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete_recursive(destination_path));
    return (1);
}

FT_TEST(test_file_copy_directory_rejects_missing_source_and_existing_file_destination)
{
    (void)file_delete_recursive("test_file_copy_directory_failure_paths");
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, file_copy_directory(
            "test_file_copy_directory_failure_paths", "test_file_copy_directory_destination"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_create_directories(
            "test_file_copy_directory_failure_paths/source", 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(
            "test_file_copy_directory_failure_paths/destination_file", "x", 1));
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, file_copy_directory(
            "test_file_copy_directory_failure_paths/source",
            "test_file_copy_directory_failure_paths/destination_file"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete_recursive(
            "test_file_copy_directory_failure_paths"));
    return (1);
}
