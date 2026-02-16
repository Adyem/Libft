#include "../test_internal.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static int istringstream_expect_sigabrt(void (*operation)(void))
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
    if (!WIFSIGNALED(child_status))
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void istringstream_destroy_twice_aborts(void)
{
    ft_string source_value;

    if (source_value.initialize("42") != FT_ERR_SUCCESS)
        _exit(2);
    ft_istringstream stream_value(source_value);
    if (stream_value.destroy() != FT_ERR_SUCCESS)
        _exit(2);
    (void)stream_value.destroy();
    _exit(0);
}

FT_TEST(test_ft_istringstream_lifecycle_read_then_destroy,
    "ft_istringstream supports read and explicit destroy")
{
    ft_string source_value;
    ft_istringstream *stream_pointer;
    char buffer[4];

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("123"));
    stream_pointer = new ft_istringstream(source_value);
    FT_ASSERT(stream_pointer != ft_nullptr);
    buffer[0] = '\0';
    buffer[1] = '\0';
    buffer[2] = '\0';
    buffer[3] = '\0';
    FT_ASSERT_EQ(3, static_cast<int>(stream_pointer->read(buffer, 3)));
    FT_ASSERT_EQ('1', buffer[0]);
    FT_ASSERT_EQ('2', buffer[1]);
    FT_ASSERT_EQ('3', buffer[2]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_pointer->destroy());
    delete stream_pointer;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_ft_istringstream_destroy_then_initialize_reuses_stream,
    "ft_istringstream can be reinitialized after destroy")
{
    ft_string source_value;
    ft_istringstream stream_value(ft_string("7"));
    char buffer[2];

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("7"));
    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(1, static_cast<int>(stream_value.read(buffer, 1)));
    FT_ASSERT_EQ('7', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_ft_istringstream_destroy_twice_aborts,
    "ft_istringstream destroy aborts when called in destroyed state")
{
    FT_ASSERT_EQ(1, istringstream_expect_sigabrt(istringstream_destroy_twice_aborts));
    return (1);
}
