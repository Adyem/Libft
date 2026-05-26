#include "../test_internal.hpp"
#include "../../Modules/GetNextLine/gnl_stream.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

static int64_t gnl_stream_test_callback(void *, char *buffer,
    ft_size_t max_size) noexcept
{
    (void)buffer;
    return (static_cast<int64_t>(max_size));
}

#define GNL_STREAM_PREPARE(stream) \
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream.initialize()); \
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream.enable_thread_safety());

#define GNL_STREAM_CLEANUP(stream) \
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream.destroy());

#define GNL_STREAM_LOCK_FAILURE_TEST(name, call) \
FT_TEST(name) \
{ \
    gnl_stream stream; \
    GNL_STREAM_PREPARE(stream); \
    pt_recursive_mutex_lock_override_error_code.store( \
        FT_ERR_SYS_MUTEX_LOCK_FAILED, std::memory_order_release); \
    int result = call; \
    pt_recursive_mutex_lock_override_error_code.store( \
        FT_ERR_SUCCESS, std::memory_order_release); \
    FT_ASSERT_EQ(FT_ERR_SYS_MUTEX_LOCK_FAILED, result); \
    GNL_STREAM_CLEANUP(stream); \
    return (1); \
}

GNL_STREAM_LOCK_FAILURE_TEST(test_gnl_stream_init_from_fd_lock_failure,
    stream.init_from_fd(0))

GNL_STREAM_LOCK_FAILURE_TEST(test_gnl_stream_init_from_file_lock_failure,
    stream.init_from_file(ft_nullptr, false))

GNL_STREAM_LOCK_FAILURE_TEST(test_gnl_stream_init_from_callback_lock_failure,
    stream.init_from_callback(gnl_stream_test_callback, ft_nullptr))

FT_TEST(test_gnl_stream_read_lock_failure)
{
    gnl_stream stream;
    GNL_STREAM_PREPARE(stream);
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        stream.init_from_callback(gnl_stream_test_callback, ft_nullptr));
    char buffer[4] = {0};
    pt_recursive_mutex_lock_override_error_code.store(
        FT_ERR_SYS_MUTEX_LOCK_FAILED, std::memory_order_release);
    FT_ASSERT_EQ(-1, stream.read(buffer, sizeof(buffer)));
    pt_recursive_mutex_lock_override_error_code.store(
        FT_ERR_SUCCESS, std::memory_order_release);
    GNL_STREAM_CLEANUP(stream);
    return (1);
}

FT_TEST(test_gnl_stream_reset_lock_failure)
{
    gnl_stream stream;
    GNL_STREAM_PREPARE(stream);
    pt_recursive_mutex_lock_override_error_code.store(
        FT_ERR_SYS_MUTEX_LOCK_FAILED, std::memory_order_release);
    stream.reset();
    pt_recursive_mutex_lock_override_error_code.store(
        FT_ERR_SUCCESS, std::memory_order_release);
    stream.reset();
    GNL_STREAM_CLEANUP(stream);
    return (1);
}

FT_TEST(test_gnl_stream_destroy_lock_failure)
{
    gnl_stream stream;

    GNL_STREAM_PREPARE(stream);
#ifdef LIBFT_TEST_BUILD
    (void)cma_untrack_leak(stream._mutex);
    if (stream._mutex != ft_nullptr && stream._mutex->_native_mutex != ft_nullptr)
        (void)cma_untrack_leak(stream._mutex->_native_mutex);
#endif
    pt_recursive_mutex_lock_override_error_code.store(
        FT_ERR_SYS_MUTEX_LOCK_FAILED, std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream.destroy());
    pt_recursive_mutex_lock_override_error_code.store(
        FT_ERR_SUCCESS, std::memory_order_release);
    return (1);
}
