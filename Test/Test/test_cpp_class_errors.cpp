#include "../../CPP_class/class_data_buffer.hpp"
#include "../../CPP_class/class_file.hpp"
#include "../../CPP_class/class_fd_istream.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../CPP_class/class_big_number.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <ios>
#include <utility>

FT_TEST(test_data_buffer_error_state_copy_move_reset_errno,
        "DataBuffer copy and move keep error codes but reset errno to success")
{
    DataBuffer error_buffer;
    DataBuffer assigned_buffer;
    DataBuffer move_assigned_buffer;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(false, error_buffer.seek(1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_buffer.get_error());

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    DataBuffer copy_constructed_buffer(error_buffer);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, copy_constructed_buffer.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_buffer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_errno = FT_ERR_CONFIGURATION;
    assigned_buffer = error_buffer;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, assigned_buffer.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_buffer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    DataBuffer move_constructor_source;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(false, move_constructor_source.seek(1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_constructor_source.get_error());

    DataBuffer moved_buffer(std::move(move_constructor_source));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, moved_buffer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, move_constructor_source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    DataBuffer move_assignment_source;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(false, move_assignment_source.seek(1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_assignment_source.get_error());
    ft_errno = FT_ERR_GAME_GENERAL_ERROR;
    move_assigned_buffer = std::move(move_assignment_source);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_assigned_buffer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, move_assignment_source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_string_error_state_copy_move_reset_errno,
        "ft_string copy and move reset errno while propagating error state")
{
    ft_string error_string(FT_ERR_INVALID_ARGUMENT);
    ft_string copy_string(error_string);
    ft_string assigned_string;

    ft_errno = FT_ERR_INVALID_OPERATION;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_string.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, copy_string.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_ERR_SYSTEM;
    assigned_string = error_string;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, assigned_string.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_string.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_string move_constructor_source(FT_ERR_INVALID_ARGUMENT);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_constructor_source.get_error());

    ft_errno = FT_ERR_INVALID_STATE;
    ft_string moved_string(std::move(move_constructor_source));

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, moved_string.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, move_constructor_source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_string move_assignment_source(FT_ERR_INVALID_ARGUMENT);
    ft_string move_assigned_string;

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_assignment_source.get_error());
    ft_errno = FT_ERR_THREAD_BUSY;
    move_assigned_string = std::move(move_assignment_source);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_assigned_string.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, move_assignment_source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_big_number_error_state_copy_move_reset_errno,
        "ft_big_number copy and move reset errno while keeping error state")
{
    ft_big_number error_number;
    ft_big_number assigned_number;
    ft_big_number move_assigned_number;

    error_number.append_digit('X');
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_number.get_error());

    ft_errno = FT_ERR_NETWORK_CONNECT_FAILED;
    ft_big_number copy_number(error_number);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, copy_number.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_number.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    assigned_number = error_number;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, assigned_number.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_number.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_big_number move_constructor_source;

    move_constructor_source.append_digit('X');
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_constructor_source.get_error());

    ft_errno = FT_ERR_SYSTEM;
    ft_big_number moved_number(std::move(move_constructor_source));

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, moved_number.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, move_constructor_source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_big_number move_assignment_source;

    move_assignment_source.append_digit('X');
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_assignment_source.get_error());
    ft_errno = FT_ERR_CRYPTO_INVALID_PADDING;
    move_assigned_number = std::move(move_assignment_source);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, move_assigned_number.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, move_assignment_source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_file_error_resets, "ft_file resets error state after success")
{
    const char *filename = "test_ft_file_error_resets.txt";
    ft_file file;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, file.read(ft_nullptr, 1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, file.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
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
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, stream.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
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

FT_TEST(test_ft_fd_istream_error_resets, "ft_fd_istream sets su_read errno and clears after success")
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
    ft_errno = FT_ERR_CONFIGURATION;
    stream.read(read_buffer, sizeof(read_buffer));
    failure_error = ft_errno;
    FT_ASSERT(failure_error != ER_SUCCESS);
    FT_ASSERT_NE(FT_ERR_CONFIGURATION, failure_error);
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
    FT_ASSERT_EQ(static_cast<std::streamsize>(sizeof(message) - 1), stream.gcount());
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
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, stream.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT(stream.bad());

    stream.read(read_buffer, 5);
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(false, stream.bad());
    FT_ASSERT_EQ(static_cast<std::streamsize>(5), stream.gcount());
    FT_ASSERT_EQ('r', read_buffer[0]);
    FT_ASSERT_EQ('e', read_buffer[1]);
    FT_ASSERT_EQ('s', read_buffer[2]);
    FT_ASSERT_EQ('e', read_buffer[3]);
    FT_ASSERT_EQ('t', read_buffer[4]);
    FT_ASSERT_EQ(0, read_buffer[5]);
    return (1);
}
