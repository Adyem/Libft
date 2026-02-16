#include "../test_internal.hpp"
#include "../../CPP_class/class_fd_istream.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static int fd_istream_expect_sigabrt(void (*operation)(void))
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

static int create_pipe_descriptors(int &read_descriptor, int &write_descriptor)
{
    int descriptors[2];

    if (pipe(descriptors) != 0)
        return (FT_ERR_INVALID_HANDLE);
    read_descriptor = descriptors[0];
    write_descriptor = descriptors[1];
    return (FT_ERR_SUCCESS);
}

static void fd_istream_destroy_twice_aborts(void)
{
    int read_descriptor;
    int write_descriptor;

    read_descriptor = -1;
    write_descriptor = -1;
    if (create_pipe_descriptors(read_descriptor, write_descriptor) != FT_ERR_SUCCESS)
        _exit(2);
    ft_fd_istream stream_value(read_descriptor);
    if (stream_value.destroy() != FT_ERR_SUCCESS)
    {
        close(read_descriptor);
        close(write_descriptor);
        _exit(2);
    }
    close(read_descriptor);
    close(write_descriptor);
    (void)stream_value.destroy();
    _exit(0);
}

FT_TEST(test_ft_fd_istream_lifecycle_read_destroy_reinitialize,
    "ft_fd_istream supports read then destroy then reinitialize")
{
    int read_descriptor;
    int write_descriptor;
    char buffer[2];
    ft_fd_istream *stream_pointer;

    read_descriptor = -1;
    write_descriptor = -1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, create_pipe_descriptors(read_descriptor, write_descriptor));
    stream_pointer = new ft_fd_istream(read_descriptor);
    FT_ASSERT(stream_pointer != ft_nullptr);
    FT_ASSERT_EQ(1, static_cast<int>(write(write_descriptor, "x", 1)));
    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(1, static_cast<int>(stream_pointer->read(buffer, 1)));
    FT_ASSERT_EQ('x', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_pointer->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_pointer->initialize());
    FT_ASSERT_EQ(1, static_cast<int>(write(write_descriptor, "y", 1)));
    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(1, static_cast<int>(stream_pointer->read(buffer, 1)));
    FT_ASSERT_EQ('y', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_pointer->destroy());
    delete stream_pointer;
    close(read_descriptor);
    close(write_descriptor);
    return (1);
}

FT_TEST(test_ft_fd_istream_thread_safety_toggle_is_explicit,
    "ft_fd_istream enables and disables thread safety only through helpers")
{
    int read_descriptor;
    int write_descriptor;
    ft_fd_istream stream_value(-1);

    read_descriptor = -1;
    write_descriptor = -1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, create_pipe_descriptors(read_descriptor, write_descriptor));
    stream_value.set_fd(read_descriptor);
    FT_ASSERT_EQ(false, stream_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.enable_thread_safety());
    FT_ASSERT_EQ(true, stream_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.disable_thread_safety());
    FT_ASSERT_EQ(false, stream_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    close(read_descriptor);
    close(write_descriptor);
    return (1);
}

FT_TEST(test_ft_fd_istream_destroy_twice_aborts,
    "ft_fd_istream destroy aborts when called in destroyed state")
{
    FT_ASSERT_EQ(1, fd_istream_expect_sigabrt(fd_istream_destroy_twice_aborts));
    return (1);
}
