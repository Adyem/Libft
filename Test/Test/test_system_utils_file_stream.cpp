#include "../test_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

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

FT_TEST(test_su_file_prepare_thread_safety_initializes_mutex,
        "su_file_prepare_thread_safety installs mutex guard")
{
    su_file file_stream;

    std::memset(&file_stream, 0, sizeof(su_file));
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_file_prepare_thread_safety(&file_stream));
    FT_ASSERT(file_stream.mutex != ft_nullptr);
    FT_ASSERT(file_stream.thread_safe_enabled);
    su_file_teardown_thread_safety(&file_stream);
    return (1);
}

FT_TEST(test_su_file_lock_unlock_acquires_mutex,
        "su_file_lock acquires the prepared mutex")
{
    su_file file_stream;
    bool    lock_acquired;

    std::memset(&file_stream, 0, sizeof(su_file));
    lock_acquired = false;
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_file_prepare_thread_safety(&file_stream));
    FT_ASSERT_EQ(0, su_file_lock(&file_stream, &lock_acquired));
    FT_ASSERT(lock_acquired);
    su_file_unlock(&file_stream, lock_acquired);
    su_file_teardown_thread_safety(&file_stream);
    return (1);
}

FT_TEST(test_su_fopen_null_path_sets_ft_einval, "su_fopen rejects null path")
{
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, su_fopen(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_fopen_propagates_open_error, "su_fopen keeps su_open error")
{
    su_file *file_stream;

    ft_errno = FT_ERR_SUCCESS;
    errno = 0;
    std::remove("missing_su_file_stream.txt");
    file_stream = su_fopen("missing_su_file_stream.txt");
    FT_ASSERT_EQ(ft_nullptr, file_stream);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_su_fopen_allocation_failure_sets_ft_ealloc, "su_fopen reports allocation failure")
{
    su_file *file_stream;

    create_test_file_stream_file();
    ft_errno = FT_ERR_SUCCESS;
    su_force_file_stream_allocation_failure(true);
    file_stream = su_fopen("test_su_file_stream.txt");
    su_force_file_stream_allocation_failure(false);
    FT_ASSERT_EQ(ft_nullptr, file_stream);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_su_fopen_success_clears_errno, "su_fopen clears ft_errno on success")
{
    su_file *file_stream;

    create_test_file_stream_file();
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    file_stream = su_fopen("test_su_file_stream.txt");
    if (file_stream == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, su_fclose(file_stream));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_su_fclose_null_stream_sets_ft_einval, "su_fclose rejects null stream")
{
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(-1, su_fclose(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_fclose_invalid_descriptor_sets_errno, "su_fclose sets errno when closing an invalid descriptor")
{
    su_file *file_stream;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    std::memset(file_stream, 0, sizeof(su_file));
    file_stream->_descriptor = -1;
    ft_errno = FT_ERR_CONFIGURATION;
    FT_ASSERT_EQ(-1, su_fclose(file_stream));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fread_null_buffer_sets_ft_einval, "su_fread rejects null buffer")
{
    su_file *file_stream;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    std::memset(file_stream, 0, sizeof(su_file));
    file_stream->_descriptor = 0;
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_fread(ft_nullptr, 1, 1, file_stream));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fread_null_stream_sets_ft_einval, "su_fread rejects null stream")
{
    char buffer[4];

    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_fread(buffer, 1, 1, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_fread_zero_size_returns_zero, "su_fread handles zero-sized requests")
{
    su_file *file_stream;
    char buffer[1];

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    std::memset(file_stream, 0, sizeof(su_file));
    file_stream->_descriptor = -1;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, su_fread(buffer, 0, 4, file_stream));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
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
    std::memset(file_stream, 0, sizeof(su_file));
    file_stream->_descriptor = -1;
    maximum_size = std::numeric_limits<size_t>::max();
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_fread(buffer, maximum_size, 2, file_stream));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fread_success_clears_errno, "su_fread clears ft_errno after successful read")
{
    su_file *file_stream;
    char buffer[5];
    size_t read_count;

    create_test_file_stream_file();
    file_stream = su_fopen("test_su_file_stream.txt");
    if (file_stream == ft_nullptr)
        return (0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    read_count = su_fread(buffer, 1, 4, file_stream);
    if (read_count > 4)
        read_count = 4;
    buffer[read_count] = '\0';
    FT_ASSERT_EQ(static_cast<size_t>(4), read_count);
    FT_ASSERT_EQ('d', buffer[0]);
    FT_ASSERT_EQ('a', buffer[1]);
    FT_ASSERT_EQ('t', buffer[2]);
    FT_ASSERT_EQ('a', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, su_fclose(file_stream));
    return (1);
}

FT_TEST(test_su_fwrite_null_buffer_sets_ft_einval, "su_fwrite rejects null buffer")
{
    su_file *file_stream;

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    std::memset(file_stream, 0, sizeof(su_file));
    file_stream->_descriptor = 0;
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_fwrite(ft_nullptr, 1, 1, file_stream));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fwrite_null_stream_sets_ft_einval, "su_fwrite rejects null stream")
{
    char buffer[4] = {'t', 'e', 's', 't'};

    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_fwrite(buffer, 1, 4, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_fwrite_zero_size_returns_zero, "su_fwrite handles zero-sized requests")
{
    su_file *file_stream;
    char buffer[1] = {'x'};

    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
        return (0);
    std::memset(file_stream, 0, sizeof(su_file));
    file_stream->_descriptor = -1;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, su_fwrite(buffer, 0, 4, file_stream));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
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
    std::memset(file_stream, 0, sizeof(su_file));
    file_stream->_descriptor = -1;
    maximum_size = std::numeric_limits<size_t>::max();
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(0, su_fwrite(buffer, maximum_size, 2, file_stream));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    std::free(file_stream);
    return (1);
}

FT_TEST(test_su_fseek_null_stream_sets_ft_einval, "su_fseek rejects null stream")
{
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(-1, su_fseek(ft_nullptr, 0, SEEK_SET));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_ftell_null_stream_sets_ft_einval, "su_ftell rejects null stream")
{
    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT_EQ(-1L, su_ftell(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

