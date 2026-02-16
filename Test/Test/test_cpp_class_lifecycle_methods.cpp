#include "../test_internal.hpp"
#include "../../CPP_class/class_data_buffer.hpp"
#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_stringbuf.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static int lifecycle_expect_sigabrt(void (*operation)(void))
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

static void data_buffer_initialize_copy_uninitialized_source_aborts(void)
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    (void)destination_buffer.initialize(source_buffer);
    _exit(0);
}

static void data_buffer_initialize_move_uninitialized_source_aborts(void)
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    (void)destination_buffer.initialize_move(source_buffer);
    _exit(0);
}

static void data_buffer_destroy_twice_aborts(void)
{
    DataBuffer buffer_value;

    if (buffer_value.initialize() != FT_ERR_SUCCESS)
        _exit(2);
    if (buffer_value.destroy() != FT_ERR_SUCCESS)
        _exit(2);
    (void)buffer_value.destroy();
    _exit(0);
}

static void stringbuf_initialize_twice_aborts(void)
{
    ft_string source_value;
    ft_stringbuf buffer_value;

    if (source_value.initialize("abc") != FT_ERR_SUCCESS)
        _exit(2);
    if (buffer_value.initialize(source_value) != FT_ERR_SUCCESS)
        _exit(2);
    (void)buffer_value.initialize(source_value);
    _exit(0);
}

static void ofstream_initialize_twice_aborts(void)
{
    ft_ofstream stream_value;

    if (stream_value.initialize() != FT_ERR_SUCCESS)
        _exit(2);
    (void)stream_value.initialize();
    _exit(0);
}

FT_TEST(test_data_buffer_initialize_copy_into_uninitialized_destination,
    "DataBuffer initialize(copy) succeeds with uninitialized destination")
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, (source_buffer << 42).get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize(source_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_copy_into_destroyed_destination,
    "DataBuffer initialize(copy) succeeds with destroyed destination")
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, (source_buffer << 7).get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize(source_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_move_into_uninitialized_destination,
    "DataBuffer initialize_move succeeds with uninitialized destination")
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, (source_buffer << 99).get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize_move(source_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_move_into_destroyed_destination,
    "DataBuffer initialize_move succeeds with destroyed destination")
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, (source_buffer << 123).get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize_move(source_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_move_self_is_noop_success,
    "DataBuffer initialize_move self returns success")
{
    DataBuffer buffer_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize_move(buffer_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_copy_uninitialized_source_aborts,
    "DataBuffer initialize(copy) aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(data_buffer_initialize_copy_uninitialized_source_aborts));
    return (1);
}

FT_TEST(test_data_buffer_initialize_move_uninitialized_source_aborts,
    "DataBuffer initialize_move aborts for uninitialized source")
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(data_buffer_initialize_move_uninitialized_source_aborts));
    return (1);
}

FT_TEST(test_data_buffer_destroy_twice_aborts,
    "DataBuffer destroy aborts when called in destroyed state")
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(data_buffer_destroy_twice_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_initialize_destroy_cycle,
    "ft_stringbuf supports initialize and destroy lifecycle")
{
    ft_string source_value;
    ft_stringbuf buffer_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("abcdef"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize(source_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_ft_stringbuf_initialize_twice_aborts,
    "ft_stringbuf initialize aborts while already initialized")
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(stringbuf_initialize_twice_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_initialize_destroy_cycle,
    "ft_ofstream supports initialize and destroy lifecycle")
{
    ft_ofstream stream_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    return (1);
}

FT_TEST(test_ft_ofstream_initialize_twice_aborts,
    "ft_ofstream initialize aborts while already initialized")
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt(ofstream_initialize_twice_aborts));
    return (1);
}
