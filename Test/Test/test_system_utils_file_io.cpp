#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

static void create_system_io_test_file(void)
{
    FILE *file_handle;

    file_handle = std::fopen("test_cmp_system_io.txt", "w");
    if (file_handle != ft_nullptr)
    {
        std::fputs("data", file_handle);
        std::fclose(file_handle);
    }
    return ;
}

static int  g_su_write_hook_calls = 0;

static ssize_t   su_write_zero_progress_hook(int file_descriptor, const void *buffer, size_t count)
{
    (void)file_descriptor;
    (void)buffer;
    if (count == 0)
        return (0);
    g_su_write_hook_calls += 1;
    if (g_su_write_hook_calls == 1)
        return (0);
    return (static_cast<ssize_t>(count));
}

FT_TEST(test_cmp_open_failure_sets_errno, "cmp_open failure reports ft_errno")
{
    ft_errno = ER_SUCCESS;
    errno = 0;
    std::remove("missing_cmp_file.txt");
    FT_ASSERT_EQ(-1, cmp_open("missing_cmp_file.txt"));
    FT_ASSERT_EQ(ENOENT + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_cmp_read_invalid_fd_sets_ft_einval, "cmp_read invalid descriptor")
{
    char buffer[4];

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, cmp_read(-1, buffer, sizeof(buffer)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cmp_write_invalid_fd_sets_ft_einval, "cmp_write invalid descriptor")
{
    char buffer[4] = {0, 0, 0, 0};

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, cmp_write(-1, buffer, sizeof(buffer)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cmp_close_invalid_fd_sets_ft_einval, "cmp_close invalid descriptor")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, cmp_close(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cmp_read_translates_errno, "cmp_read propagates errno failures")
{
    int file_descriptor;
    char buffer[4];

    create_system_io_test_file();
    ft_errno = ER_SUCCESS;
    file_descriptor = cmp_open("test_cmp_system_io.txt", O_WRONLY);
    FT_ASSERT(file_descriptor >= 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    errno = 0;
    FT_ASSERT_EQ(-1, cmp_read(file_descriptor, buffer, sizeof(buffer)));
    FT_ASSERT_EQ(EBADF + ERRNO_OFFSET, ft_errno);
    FT_ASSERT_EQ(0, cmp_close(file_descriptor));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cmp_write_translates_errno, "cmp_write propagates errno failures")
{
    int file_descriptor;
    const char buffer[4] = {'t', 'e', 's', 't'};

    create_system_io_test_file();
    ft_errno = ER_SUCCESS;
    file_descriptor = cmp_open("test_cmp_system_io.txt", O_RDONLY);
    FT_ASSERT(file_descriptor >= 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    errno = 0;
    FT_ASSERT_EQ(-1, cmp_write(file_descriptor, buffer, sizeof(buffer)));
    FT_ASSERT_EQ(EBADF + ERRNO_OFFSET, ft_errno);
    FT_ASSERT_EQ(0, cmp_close(file_descriptor));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_su_write_reports_zero_progress, "su_write reports stalled writes as FT_ERR_IO")
{
    const char  buffer[4] = {'d', 'a', 't', 'a'};

    g_su_write_hook_calls = 0;
    su_set_write_syscall_hook(su_write_zero_progress_hook);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_write(42, buffer, sizeof(buffer)));
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    FT_ASSERT_EQ(1, g_su_write_hook_calls);
    su_reset_write_syscall_hook();
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_cmp_close_translates_errno, "cmp_close propagates errno failures")
{
    int file_descriptor;

    create_system_io_test_file();
    ft_errno = ER_SUCCESS;
    file_descriptor = cmp_open("test_cmp_system_io.txt", O_RDONLY);
    FT_ASSERT(file_descriptor >= 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, cmp_close(file_descriptor));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    errno = 0;
    FT_ASSERT_EQ(-1, cmp_close(file_descriptor));
    FT_ASSERT_EQ(EBADF + ERRNO_OFFSET, ft_errno);
    return (1);
}

