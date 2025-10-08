#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <cstdio>
#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#endif

FT_TEST(test_cmp_dir_open_null_path, "cmp_dir_open null path sets ft_errno")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, cmp_dir_open(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cmp_dir_open_allocation_failure, "cmp_dir_open allocation failure propagates ft_errno")
{
    file_dir *directory_stream;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    directory_stream = cmp_dir_open(".");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, directory_stream);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_cmp_dir_open_os_error, "cmp_dir_open propagates operating system errors")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, cmp_dir_open("cmp_dir_open_os_error_missing"));
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(static_cast<int>(ERROR_FILE_NOT_FOUND) + ERRNO_OFFSET, ft_errno);
#else
    FT_ASSERT_EQ(ENOENT + ERRNO_OFFSET, ft_errno);
#endif
    return (1);
}

FT_TEST(test_cmp_dir_read_null_stream, "cmp_dir_read null stream sets ft_errno")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, cmp_dir_read(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cmp_dir_read_success_resets_errno, "cmp_dir_read success clears ft_errno")
{
    file_dir *directory_stream;
    file_dirent *directory_entry;

    directory_stream = cmp_dir_open(".");
    if (directory_stream == ft_nullptr)
        return (0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    directory_entry = cmp_dir_read(directory_stream);
    FT_ASSERT(directory_entry != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cmp_dir_close(directory_stream);
    return (1);
}

FT_TEST(test_cmp_directory_exists_null_path, "cmp_directory_exists null path sets ft_errno")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, cmp_directory_exists(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cmp_directory_exists_existing_directory, "cmp_directory_exists finds real directory")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, cmp_directory_exists("."));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cmp_directory_exists_missing_directory, "cmp_directory_exists missing directory propagates error")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, cmp_directory_exists("cmp_directory_exists_missing_directory"));
#if defined(_WIN32) || defined(_WIN64)
    FT_ASSERT_EQ(static_cast<int>(ERROR_FILE_NOT_FOUND) + ERRNO_OFFSET, ft_errno);
#else
    FT_ASSERT_EQ(ENOENT + ERRNO_OFFSET, ft_errno);
#endif
    return (1);
}

FT_TEST(test_cmp_directory_exists_file_path, "cmp_directory_exists returns 0 for files without error")
{
    const char *file_path = "cmp_directory_exists_file_path.txt";
    std::FILE *file_handle;

    file_handle = std::fopen(file_path, "w");
    if (file_handle == ft_nullptr)
        return (0);
    std::fclose(file_handle);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, cmp_directory_exists(file_path));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    std::remove(file_path);
    return (1);
}
