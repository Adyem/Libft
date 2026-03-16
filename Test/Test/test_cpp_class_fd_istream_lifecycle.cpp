#include "../test_internal.hpp"
#include "../../CPP_class/class_fd_istream.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static int32_t create_pipe_descriptors(int &read_descriptor, int &write_descriptor)
{
    int descriptors[2];

    if (pipe(descriptors) != 0)
        return (FT_ERR_INVALID_HANDLE);
    read_descriptor = descriptors[0];
    write_descriptor = descriptors[1];
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_cpp_class_fd_istream_lifecycle_read_destroy_reinitialize)
{
    int read_descriptor;
    int write_descriptor;
    char buffer[2];
    ft_fd_istream stream_value;

    read_descriptor = -1;
    write_descriptor = -1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, create_pipe_descriptors(read_descriptor, write_descriptor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize());
    stream_value.set_file_descriptor(read_descriptor);
    FT_ASSERT_EQ(1, static_cast<int>(write(write_descriptor, "x", 1)));
    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(1, static_cast<int>(stream_value.read(buffer, 1)));
    FT_ASSERT_EQ('x', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize());
    stream_value.set_file_descriptor(read_descriptor);
    FT_ASSERT_EQ(1, static_cast<int>(write(write_descriptor, "y", 1)));
    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(1, static_cast<int>(stream_value.read(buffer, 1)));
    FT_ASSERT_EQ('y', buffer[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    close(read_descriptor);
    close(write_descriptor);
    return (1);
}

FT_TEST(test_cpp_class_fd_istream_thread_safety_toggle_is_explicit)
{
    int read_descriptor;
    int write_descriptor;
    ft_fd_istream stream_value;

    read_descriptor = -1;
    write_descriptor = -1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, create_pipe_descriptors(read_descriptor, write_descriptor));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize());
    stream_value.set_file_descriptor(read_descriptor);
    FT_ASSERT_EQ(FT_FALSE, stream_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, stream_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, stream_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    close(read_descriptor);
    close(write_descriptor);
    return (1);
}

FT_TEST(test_cpp_class_fd_istream_destroy_is_no_op_for_destroyed_state)
{
    ft_fd_istream stream_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    return (1);
}
