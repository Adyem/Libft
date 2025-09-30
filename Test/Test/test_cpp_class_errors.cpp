#include "../../CPP_class/class_file.hpp"
#include "../../CPP_class/class_fd_istream.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <fcntl.h>
#include <unistd.h>

FT_TEST(test_ft_file_error_resets, "ft_file resets error state after success")
{
    const char *filename = "test_ft_file_error_resets.txt";
    ft_file file;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, file.read(ft_nullptr, 1));
    FT_ASSERT_EQ(FT_EINVAL, file.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(0, file.open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644));
    FT_ASSERT_EQ(ER_SUCCESS, file.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    file.close();
    ::unlink(filename);
    return (1);
}

FT_TEST(test_ft_ofstream_error_resets, "ft_ofstream resets error state after success")
{
    const char *filename = "test_ft_ofstream_error_resets.txt";
    ft_ofstream stream;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(1, stream.open(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, stream.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(0, stream.open(filename));
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(5, stream.write("reset"));
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    stream.close();
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ::unlink(filename);
    return (1);
}

FT_TEST(test_ft_fd_istream_error_resets, "ft_fd_istream preserves su_read errno and clears after success")
{
    int pipe_descriptors[2];
    int target_descriptor;
    int write_descriptor;
    int new_read_descriptor;
    ssize_t write_result;
    char read_buffer[3];
    int failure_error;
    const char message[] = "go";

    FT_ASSERT_EQ(0, ::pipe(pipe_descriptors));
    target_descriptor = pipe_descriptors[0];
    write_descriptor = pipe_descriptors[1];
    ft_fd_istream stream(target_descriptor);

    FT_ASSERT_EQ(0, ::close(target_descriptor));
    FT_ASSERT_EQ(0, ::close(write_descriptor));
    stream.read(read_buffer, sizeof(read_buffer));
    failure_error = ft_errno;
    FT_ASSERT(failure_error != ER_SUCCESS);
    FT_ASSERT_EQ(failure_error, stream.get_error());

    FT_ASSERT_EQ(0, ::pipe(pipe_descriptors));
    new_read_descriptor = pipe_descriptors[0];
    write_descriptor = pipe_descriptors[1];
    if (new_read_descriptor != target_descriptor)
    {
        FT_ASSERT_EQ(target_descriptor, ::dup2(new_read_descriptor, target_descriptor));
        FT_ASSERT_EQ(0, ::close(new_read_descriptor));
    }

    write_result = ::write(write_descriptor, message, sizeof(message) - 1);
    FT_ASSERT_EQ(static_cast<ssize_t>(sizeof(message) - 1), write_result);
    FT_ASSERT_EQ(0, ::close(write_descriptor));

    stream.read(read_buffer, sizeof(message) - 1);
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<std::size_t>(sizeof(message) - 1), stream.gcount());
    FT_ASSERT_EQ('g', read_buffer[0]);
    FT_ASSERT_EQ('o', read_buffer[1]);
    FT_ASSERT_EQ(0, ::close(target_descriptor));
    return (1);
}

FT_TEST(test_ft_istringstream_error_resets, "ft_istringstream resets error state after invalid read")
{
    ft_istringstream stream("reset");
    char read_buffer[6] = {0};

    ft_errno = ER_SUCCESS;
    stream.read(ft_nullptr, 1);
    FT_ASSERT_EQ(FT_EINVAL, stream.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT(stream.bad());

    stream.read(read_buffer, 5);
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(false, stream.bad());
    FT_ASSERT_EQ(static_cast<std::size_t>(5), stream.gcount());
    FT_ASSERT_EQ('r', read_buffer[0]);
    FT_ASSERT_EQ('e', read_buffer[1]);
    FT_ASSERT_EQ('s', read_buffer[2]);
    FT_ASSERT_EQ('e', read_buffer[3]);
    FT_ASSERT_EQ('t', read_buffer[4]);
    FT_ASSERT_EQ(0, read_buffer[5]);
    return (1);
}
