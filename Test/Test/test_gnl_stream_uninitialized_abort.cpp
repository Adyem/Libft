#include "../test_internal.hpp"
#include "../../GetNextLine/gnl_stream.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

typedef gnl_stream gnl_stream_type;

static int gnl_stream_expect_sigabrt_uninitialized(void (*operation)(gnl_stream_type &))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(gnl_stream_type) unsigned char storage[sizeof(gnl_stream_type)];
        gnl_stream_type *stream_pointer;

        std::memset(storage, 0, sizeof(storage));
        stream_pointer = reinterpret_cast<gnl_stream_type *>(storage);
        operation(*stream_pointer);
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (!WIFSIGNALED(child_status))
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void gnl_stream_call_destructor(gnl_stream_type &stream_instance)
{
    stream_instance.~gnl_stream_type();
    return ;
}

static void gnl_stream_call_destroy(gnl_stream_type &stream_instance)
{
    (void)stream_instance.destroy();
    return ;
}

static void gnl_stream_call_init_from_fd(gnl_stream_type &stream_instance)
{
    (void)stream_instance.init_from_fd(0);
    return ;
}

static void gnl_stream_call_init_from_file(gnl_stream_type &stream_instance)
{
    (void)stream_instance.init_from_file(ft_nullptr, false);
    return ;
}

static ssize_t gnl_stream_noop_callback(void *, char *, size_t) noexcept
{
    return (0);
}

static void gnl_stream_call_init_from_callback(gnl_stream_type &stream_instance)
{
    (void)stream_instance.init_from_callback(gnl_stream_noop_callback, ft_nullptr);
    return ;
}

static void gnl_stream_call_reset(gnl_stream_type &stream_instance)
{
    stream_instance.reset();
    return ;
}

static void gnl_stream_call_read(gnl_stream_type &stream_instance)
{
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    (void)stream_instance.read(buffer, 1);
    return ;
}

FT_TEST(test_gnl_stream_uninitialized_destructor_aborts,
    "gnl_stream destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialized(gnl_stream_call_destructor));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialized_destroy_aborts,
    "gnl_stream destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialized(gnl_stream_call_destroy));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialized_init_from_fd_aborts,
    "gnl_stream init_from_fd aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialized(gnl_stream_call_init_from_fd));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialized_init_from_file_aborts,
    "gnl_stream init_from_file aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialized(gnl_stream_call_init_from_file));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialized_init_from_callback_aborts,
    "gnl_stream init_from_callback aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialized(gnl_stream_call_init_from_callback));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialized_reset_aborts,
    "gnl_stream reset aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialized(gnl_stream_call_reset));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialized_read_aborts,
    "gnl_stream read aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialized(gnl_stream_call_read));
    return (1);
}
