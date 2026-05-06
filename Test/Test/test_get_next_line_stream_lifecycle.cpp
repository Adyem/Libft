#include "../test_internal.hpp"
#include "../../Modules/GetNextLine/gnl_stream.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static ssize_t gnl_stream_lifecycle_callback(void *, char *buffer, size_t max_size) noexcept
{
    if (buffer == ft_nullptr || max_size == 0)
        return (-1);
    buffer[0] = 'z';
    return (1);
}

static int gnl_stream_expect_sigabrt(void (*operation)())
{
    return (test_expect_sigabrt_signal(operation));
}

static void gnl_stream_initialize_twice_aborts_operation()
{
    gnl_stream stream_instance;

    (void)stream_instance.initialize();
    (void)stream_instance.initialize();
    return ;
}

static void gnl_stream_destroy_twice_aborts_operation()
{
    gnl_stream stream_instance;

    (void)stream_instance.initialize();
    (void)stream_instance.destroy();
    (void)stream_instance.destroy();
    return ;
}

static void gnl_stream_read_on_destroyed_aborts_operation()
{
    gnl_stream stream_instance;
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    (void)stream_instance.initialize();
    (void)stream_instance.destroy();
    (void)stream_instance.read(buffer, 1);
    return ;
}

FT_TEST(test_gnl_stream_initialize_destroy_reinitialize_success)
{
    gnl_stream stream_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_reinitialize_after_destroy_with_callback)
{
    gnl_stream stream_instance;
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        stream_instance.init_from_callback(gnl_stream_lifecycle_callback, ft_nullptr));
    FT_ASSERT_EQ(1, stream_instance.read(buffer, 1));
    FT_ASSERT_EQ('z', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt(gnl_stream_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_gnl_stream_destroy_twice_succeeds)
{
    FT_ASSERT_EQ(0, gnl_stream_expect_sigabrt(gnl_stream_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_gnl_stream_read_on_destroyed_aborts)
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt(gnl_stream_read_on_destroyed_aborts_operation));
    return (1);
}
