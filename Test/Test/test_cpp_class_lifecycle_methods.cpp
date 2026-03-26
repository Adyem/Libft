#include "../test_internal.hpp"
#include "../../CPP_class/class_data_buffer.hpp"
#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_stringbuf.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <csetjmp>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static sigjmp_buf g_lifecycle_abort_jump;
static ft_string g_stringbuf_abort_source_value;
static ft_stringbuf g_stringbuf_abort_buffer_value;

static void lifecycle_abort_handler(int /*signal_number*/)
{
    siglongjmp(g_lifecycle_abort_jump, 1);
    return ;
}

static int lifecycle_expect_sigabrt_signal_handler(void (*operation)(void))
{
    struct sigaction action;
    struct sigaction backup;
    int result;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = lifecycle_abort_handler;
    sigemptyset(&action.sa_mask);
    result = 0;
    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_lifecycle_abort_jump, 1) == 0)
    {
        operation();
        result = 0;
    }
    else
        result = 1;
    (void)sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

static void data_buffer_initialize_copy_uninitialised_source_aborts(void)
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    (void)destination_buffer.initialize(source_buffer);
    return ;
}

static void data_buffer_initialize_move_uninitialised_source_aborts(void)
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    (void)destination_buffer.initialize(ft_move(source_buffer));
    return ;
}

static void stringbuf_initialize_twice_aborts(void)
{
    if (g_stringbuf_abort_source_value.initialize("abc") != FT_ERR_SUCCESS)
        return ;
    if (g_stringbuf_abort_buffer_value.initialize(g_stringbuf_abort_source_value)
        != FT_ERR_SUCCESS)
        return ;
    (void)g_stringbuf_abort_buffer_value.initialize(g_stringbuf_abort_source_value);
    return ;
}

static void ofstream_initialize_twice_aborts(void)
{
    ft_ofstream stream_value;

    if (stream_value.initialize() != FT_ERR_SUCCESS)
        return ;
    (void)stream_value.initialize();
    return ;
}

FT_TEST(test_data_buffer_initialize_copy_into_uninitialised_destination)
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

FT_TEST(test_data_buffer_initialize_copy_into_destroyed_destination)
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

FT_TEST(test_data_buffer_initialize_move_into_uninitialised_destination)
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, (source_buffer << 99).get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize(ft_move(source_buffer)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_move_into_destroyed_destination)
{
    DataBuffer source_buffer;
    DataBuffer destination_buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, (source_buffer << 123).get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize(ft_move(source_buffer)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_move_self_is_noop_success)
{
    DataBuffer buffer_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize(ft_move(buffer_value)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.destroy());
    return (1);
}

FT_TEST(test_data_buffer_initialize_copy_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt_signal_handler(
        data_buffer_initialize_copy_uninitialised_source_aborts));
    return (1);
}

FT_TEST(test_data_buffer_initialize_move_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt_signal_handler(
        data_buffer_initialize_move_uninitialised_source_aborts));
    return (1);
}

FT_TEST(test_data_buffer_destroy_tolerates_destroyed_instance)
{
    DataBuffer buffer_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.destroy());
    return (1);
}

FT_TEST(test_data_buffer_string_round_trip_preserves_instance_error_success)
{
    DataBuffer buffer_value;
    ft_string write_value;
    ft_string read_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, write_value.initialize("payload"));
    buffer_value << write_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.get_error());
    buffer_value >> read_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, read_value.get_error());
    FT_ASSERT(read_value == "payload");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.destroy());
    return (1);
}


FT_TEST(test_ft_stringbuf_initialize_destroy_cycle)
{
    ft_string source_value;
    ft_stringbuf buffer_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("abcdef"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.initialize(source_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_ft_stringbuf_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt_signal_handler(
        stringbuf_initialize_twice_aborts));
    (void)g_stringbuf_abort_buffer_value.destroy();
    (void)g_stringbuf_abort_source_value.destroy();
    return (1);
}

FT_TEST(test_ft_ofstream_initialize_destroy_cycle)
{
    ft_ofstream stream_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    return (1);
}

FT_TEST(test_ft_ofstream_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, lifecycle_expect_sigabrt_signal_handler(
        ofstream_initialize_twice_aborts));
    return (1);
}
