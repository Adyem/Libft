#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../File/open_dir.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <cstdio>
#if defined(_WIN32) || defined(_WIN64)
# include <io.h>
#else
# include <unistd.h>
#endif

static void create_test_file(void)
{
    FILE *file = ft_fopen("test_file_io.txt", "w");
    if (file != ft_nullptr)
    {
        std::fputs("Line1\nLine2\n", file);
        ft_fclose(file);
    }
    return ;
}

static void force_file_descriptor_failure(FILE *file_handle)
{
    int file_descriptor;

    if (file_handle == ft_nullptr)
        return ;
#if defined(_WIN32) || defined(_WIN64)
    file_descriptor = _fileno(file_handle);
    if (file_descriptor >= 0)
        _close(file_descriptor);
#else
    file_descriptor = fileno(file_handle);
    if (file_descriptor >= 0)
        close(file_descriptor);
#endif
    return ;
}

FT_TEST(test_fopen_valid, "ft_fopen and ft_fclose basic")
{
    FILE *file;

    create_test_file();
    ft_errno = FILE_INVALID_FD;
    file = ft_fopen("test_file_io.txt", "r");
    FT_ASSERT(file != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FILE_INVALID_FD;
    FT_ASSERT_EQ(FT_SUCCESS, ft_fclose(file));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_fopen_invalid, "ft_fopen invalid path")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_fopen("missing_file.txt", "r"));
    FT_ASSERT_EQ(ENOENT + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_fopen_null, "ft_fopen with null")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_fopen(ft_nullptr, "r"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_fopen("test_file_io.txt", ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_fclose_null, "ft_fclose with null")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(EOF, ft_fclose(ft_nullptr));
    FT_ASSERT_EQ(FILE_INVALID_FD, ft_errno);
    return (1);
}

FT_TEST(test_fclose_failure_sets_errno, "ft_fclose propagates fclose failures")
{
    FILE *file;

    create_test_file();
    file = ft_fopen("test_file_io.txt", "r");
    if (file == ft_nullptr)
        return (0);
    force_file_descriptor_failure(file);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(EOF, ft_fclose(file));
    FT_ASSERT_EQ(EBADF + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_fgets_basic, "ft_fgets basic")
{
    char buffer[16];
    FILE *file;

    create_test_file();
    ft_errno = FILE_INVALID_FD;
    file = ft_fopen("test_file_io.txt", "r");
    if (file == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FILE_INVALID_FD;
    FT_ASSERT_EQ(buffer, ft_fgets(buffer, 16, file));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Line1\n"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FILE_INVALID_FD;
    FT_ASSERT_EQ(buffer, ft_fgets(buffer, 16, file));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Line2\n"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FILE_INVALID_FD;
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(buffer, 16, file));
    FT_ASSERT_EQ(FILE_END_OF_FILE, ft_errno);
    FT_ASSERT_EQ(FT_SUCCESS, ft_fclose(file));
    return (1);
}

FT_TEST(test_fgets_edge_cases, "ft_fgets edge cases")
{
    char buffer[8];
    FILE *file;

    create_test_file();
    ft_errno = FILE_INVALID_FD;
    file = ft_fopen("test_file_io.txt", "r");
    if (file == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FILE_INVALID_FD;
    FT_ASSERT_EQ(buffer, ft_fgets(buffer, 5, file));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Line"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(ft_nullptr, 5, file));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(buffer, 0, file));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(buffer, 5, ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(FT_SUCCESS, ft_fclose(file));
    return (1);
}

FT_TEST(test_fgets_stream_error_sets_errno, "ft_fgets reports stream errors")
{
    char buffer[8];
    FILE *file;

    create_test_file();
    file = ft_fopen("test_file_io.txt", "r");
    if (file == ft_nullptr)
        return (0);
    force_file_descriptor_failure(file);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(buffer, sizeof(buffer), file));
    FT_ASSERT_EQ(EBADF + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_file_readdir_returns_entries, "file_readdir yields directory entries")
{
    file_dir *directory_stream;
    file_dirent *directory_entry;

    directory_stream = file_opendir(".");
    FT_ASSERT(directory_stream != ft_nullptr);
    directory_entry = file_readdir(directory_stream);
    FT_ASSERT(directory_entry != ft_nullptr);
    FT_ASSERT_EQ(0, file_closedir(directory_stream));
    return (1);
}
