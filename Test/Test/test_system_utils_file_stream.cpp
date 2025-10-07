#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <limits>

extern void su_force_file_stream_allocation_failure(bool should_fail);

static void create_test_file_stream_file(void)
{
    FILE *file_handle;

    file_handle = std::fopen("test_su_file_stream.txt", "w");
    if (file_handle != ft_nullptr)
    {
        std::fputs("data", file_handle);
        std::fclose(file_handle);
    }
    return ;
}

FT_TEST(test_su_fopen_null_path_sets_ft_einval, "su_fopen rejects null path")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, su_fopen(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_su_fopen_propagates_open_error, "su_fopen keeps su_open error")
{
    su_file *file_stream;

    ft_errno = ER_SUCCESS;
    errno = 0;
    std::remove("missing_su_file_stream.txt");
    file_stream = su_fopen("missing_su_file_stream.txt");
    FT_ASSERT_EQ(ft_nullptr, file_stream);
    FT_ASSERT_EQ(ENOENT + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_su_fopen_allocation_failure_sets_ft_ealloc, "su_fopen reports allocation failure")
{
    su_file *file_stream;

    create_test_file_stream_file();
    ft_errno = ER_SUCCESS;
    su_force_file_stream_allocation_failure(true);
    file_stream = su_fopen("test_su_file_stream.txt");
    su_force_file_stream_allocation_failure(false);
    FT_ASSERT_EQ(ft_nullptr, file_stream);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_su_fopen_success_clears_errno, "su_fopen clears ft_errno on success")
{
    su_file *file_stream;

    create_test_file_stream_file();
    ft_errno = FT_EINVAL;
    file_stream = su_fopen("test_su_file_stream.txt");
    if (file_stream == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, su_fclose(file_stream));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_su_fclose_null_stream_sets_ft_einval, "su_fclose rejects null stream")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_fclose(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_su_fclose_invalid_descriptor_propagates_error, "su_fclose preserves cmp_close error")
{
    su_file *file_stream;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    file_stream->_descriptor = -1;
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_fclose(file_stream));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fread_null_buffer_sets_ft_einval, "su_fread rejects null buffer")
{
    su_file *file_stream;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    file_stream->_descriptor = 0;
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, su_fread(ft_nullptr, 1, 1, file_stream));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fread_null_stream_sets_ft_einval, "su_fread rejects null stream")
{
    char buffer[4];

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, su_fread(buffer, 1, 1, ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_su_fread_zero_size_returns_zero, "su_fread handles zero-sized requests")
{
    su_file *file_stream;
    char buffer[1];

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    file_stream->_descriptor = -1;
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, su_fread(buffer, 0, 4, file_stream));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fread_overflow_sets_ft_erange, "su_fread rejects overflowing sizes")
{
    su_file *file_stream;
    char buffer[1];
    size_t maximum_size;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    file_stream->_descriptor = -1;
    maximum_size = std::numeric_limits<size_t>::max();
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, su_fread(buffer, maximum_size, 2, file_stream));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fwrite_null_buffer_sets_ft_einval, "su_fwrite rejects null buffer")
{
    su_file *file_stream;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    file_stream->_descriptor = 0;
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, su_fwrite(ft_nullptr, 1, 1, file_stream));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fwrite_null_stream_sets_ft_einval, "su_fwrite rejects null stream")
{
    char buffer[4] = {'t', 'e', 's', 't'};

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, su_fwrite(buffer, 1, 4, ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_su_fwrite_zero_size_returns_zero, "su_fwrite handles zero-sized requests")
{
    su_file *file_stream;
    char buffer[1] = {'x'};

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    file_stream->_descriptor = -1;
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, su_fwrite(buffer, 0, 4, file_stream));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fwrite_overflow_sets_ft_erange, "su_fwrite rejects overflowing sizes")
{
    su_file *file_stream;
    char buffer[1] = {'x'};
    size_t maximum_size;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    file_stream->_descriptor = -1;
    maximum_size = std::numeric_limits<size_t>::max();
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, su_fwrite(buffer, maximum_size, 2, file_stream));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fseek_null_stream_sets_ft_einval, "su_fseek rejects null stream")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_fseek(ft_nullptr, 0, SEEK_SET));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_su_ftell_null_stream_sets_ft_einval, "su_ftell rejects null stream")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1L, su_ftell(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

