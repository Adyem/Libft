#include "../test_internal.hpp"
#include "../../Modules/GetNextLine/gnl_stream.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

typedef gnl_stream gnl_stream_type;

static int gnl_stream_expect_sigabrt_uninitialised(void (*operation)(gnl_stream_type &))
{
    return (test_expect_sigabrt_signal_uninitialised<gnl_stream_type>(operation));
}

static void gnl_stream_call_destructor(gnl_stream_type &stream_instance)
{
    stream_instance.~gnl_stream_type();
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

static int64_t gnl_stream_noop_callback(void *, char *, ft_size_t) noexcept
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

FT_TEST(test_gnl_stream_uninitialised_destructor_allows_deletion)
{
    FT_ASSERT_EQ(0, gnl_stream_expect_sigabrt_uninitialised(gnl_stream_call_destructor));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialised_destroy_returns_invalid_state)
{
    gnl_stream_type stream_instance;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_uninitialised_init_from_fd_aborts)
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialised(gnl_stream_call_init_from_fd));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialised_init_from_file_aborts)
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialised(gnl_stream_call_init_from_file));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialised_init_from_callback_aborts)
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialised(gnl_stream_call_init_from_callback));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialised_reset_aborts)
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialised(gnl_stream_call_reset));
    return (1);
}

FT_TEST(test_gnl_stream_uninitialised_read_aborts)
{
    FT_ASSERT_EQ(1, gnl_stream_expect_sigabrt_uninitialised(gnl_stream_call_read));
    return (1);
}
