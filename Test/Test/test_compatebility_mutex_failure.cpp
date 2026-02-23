#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/mutex.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include <cstdio>
#include <ctime>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cmp_dir_read_mutex_lock_failure,
    "cmp_dir_read reports failure when the directory mutex lock fails")
{
    int32_t error_code = FT_ERR_SUCCESS;
    file_dir *directory_stream = cmp_dir_open(".", &error_code);

    if (directory_stream == ft_nullptr)
        return (0);
    pt_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    file_dirent *directory_entry = cmp_dir_read(directory_stream, &error_code);
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(ft_nullptr, directory_entry);
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, error_code);
    int32_t close_error = cmp_dir_close(directory_stream, &error_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, close_error);
    return (1);
}

FT_TEST(test_cmp_dir_close_mutex_lock_failure,
    "cmp_dir_close reports failure when the directory mutex lock fails")
{
    int32_t error_code = FT_ERR_SUCCESS;
    file_dir *directory_stream = cmp_dir_open(".", &error_code);

    if (directory_stream == ft_nullptr)
        return (0);
    pt_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    int32_t close_result = cmp_dir_close(directory_stream, &error_code);
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, close_result);
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, error_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cmp_dir_close(directory_stream, &error_code));
    return (1);
}

#if defined(_WIN32) || defined(_WIN64)
FT_TEST(test_cmp_file_read_mutex_lock_failure,
    "cmp_read reports failure when the module-wide file mutex lock fails")
{
    const char *file_path = "test_cmp_file_mutex_failure.bin";
    std::FILE *file_handle = std::fopen(file_path, "w+b");

    if (file_handle == ft_nullptr)
        return (0);
    std::fwrite("data", 1, 4, file_handle);
    std::fclose(file_handle);
    int32_t descriptor = cmp_open(file_path);

    if (descriptor < 0)
    {
        std::remove(file_path);
        return (0);
    }
    pt_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    char buffer[4];
    int64_t bytes_read = 0;
    int32_t read_error = cmp_read(descriptor, buffer, sizeof(buffer),
        &bytes_read);
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, read_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cmp_close(descriptor));
    std::remove(file_path);
    return (1);
}
#endif

#if !defined(_WIN32) && !defined(_WIN64) && !defined(_POSIX_VERSION)
FT_TEST(test_cmp_localtime_mutex_lock_failure,
    "cmp_localtime reports failure when the localtime mutex lock fails")
{
    std::time_t current_time = 0;
    std::tm output;

    pt_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    int32_t localtime_error = cmp_localtime(&current_time, &output);
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, localtime_error);
    return (1);
}
#endif
#if defined(_WIN32) || defined(_WIN64)
FT_TEST(test_cmp_open_mutex_lock_failure,
    "cmp_open returns -1 when the module file mutex lock fails")
{
    pt_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    int32_t descriptor = cmp_open("test_cmp_open_mutex_failure.bin");
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(-1, descriptor);
    return (1);
}

FT_TEST(test_cmp_write_mutex_lock_failure,
    "cmp_write reports failure when the module-wide file mutex lock fails")
{
    const char *file_path = "test_cmp_write_mutex_failure.bin";
    std::FILE *file_handle = std::fopen(file_path, "w+b");

    if (file_handle == ft_nullptr)
        return (0);
    std::fclose(file_handle);
    int32_t descriptor = cmp_open(file_path);
    if (descriptor < 0)
    {
        std::remove(file_path);
        return (0);
    }
    pt_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    const char buffer[] = "data";
    int32_t write_error = cmp_write(descriptor, buffer, sizeof(buffer),
        ft_nullptr);
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, write_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cmp_close(descriptor));
    std::remove(file_path);
    return (1);
}

FT_TEST(test_cmp_close_mutex_lock_failure,
    "cmp_close reports failure when the module-wide file mutex lock fails")
{
    const char *file_path = "test_cmp_close_mutex_failure.bin";
    std::FILE *file_handle = std::fopen(file_path, "w+b");

    if (file_handle == ft_nullptr)
        return (0);
    std::fclose(file_handle);
    int32_t descriptor = cmp_open(file_path);
    if (descriptor < 0)
    {
        std::remove(file_path);
        return (0);
    }
    pt_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    int32_t close_error = cmp_close(descriptor);
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, close_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cmp_close(descriptor));
    std::remove(file_path);
    return (1);
}
#endif
