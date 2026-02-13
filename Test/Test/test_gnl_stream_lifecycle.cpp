#include "../test_internal.hpp"
#include "../../GetNextLine/gnl_stream.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

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
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        operation();
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status) == 0)
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
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

FT_TEST(test_gnl_stream_initialize_destroy_reinitialize_success,
    "gnl_stream initialize and destroy can be repeated safely")
{
    gnl_stream stream_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_reinitialize_after_destroy_with_callback,
    "gnl_stream supports callback use after reinitialize from destroyed state")
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

FT_TEST(test_gnl_stream_initialize_twice_aborts,
    "gnl_stream initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt(gnl_stream_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_gnl_stream_destroy_twice_aborts,
    "gnl_stream destroy aborts when called after object is destroyed")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt(gnl_stream_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_gnl_stream_read_on_destroyed_aborts,
    "gnl_stream read aborts when called after destroy")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt(gnl_stream_read_on_destroyed_aborts_operation));
    return (1);
}
