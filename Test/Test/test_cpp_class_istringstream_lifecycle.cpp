#include "../test_internal.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Basic/basic.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int istringstream_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_istringstream_get_error_returned = FT_FALSE;
static int32_t g_istringstream_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_istringstream_get_error_str_returned = FT_FALSE;
static const char *g_istringstream_get_error_str_result = ft_nullptr;

static void istringstream_get_error_uninitialised_operation(void)
{
    ft_istringstream stream_value;

    g_istringstream_get_error_result = stream_value.get_error();
    g_istringstream_get_error_returned = FT_TRUE;
    return ;
}

static void istringstream_get_error_str_uninitialised_operation(void)
{
    ft_istringstream stream_value;

    g_istringstream_get_error_str_result = stream_value.get_error_str();
    g_istringstream_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_cpp_class_istringstream_initialize_destroy_cycle)
{
    ft_istringstream stream_value;
    ft_string source_value;
    int32_t parsed_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("123"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize(source_value));
    parsed_value = 0;
    stream_value >> parsed_value;
    FT_ASSERT_EQ(123, parsed_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize(source_value));
    parsed_value = 0;
    stream_value >> parsed_value;
    FT_ASSERT_EQ(123, parsed_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_istringstream_destroy_is_no_op_after_destroy)
{
    ft_istringstream stream_value;
    ft_string source_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("7"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize(source_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_istringstream_error_queries_follow_lifecycle_contract)
{
    ft_istringstream stream_value;
    ft_string source_value;

    g_istringstream_get_error_returned = FT_FALSE;
    g_istringstream_get_error_result = FT_ERR_SUCCESS;
    g_istringstream_get_error_str_returned = FT_FALSE;
    g_istringstream_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, istringstream_expect_sigabrt(
        istringstream_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_istringstream_get_error_returned);
    FT_ASSERT_EQ(1, istringstream_expect_sigabrt(
        istringstream_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_istringstream_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("17"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.initialize(source_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(stream_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_istringstream_move_transfers_buffer)
{
    ft_istringstream source_stream;
    ft_istringstream destination_stream;
    ft_string source_value;
    int32_t parsed_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("42"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stream.initialize(source_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stream.move(source_stream));
    parsed_value = 0;
    destination_stream >> parsed_value;
    FT_ASSERT_EQ(42, parsed_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stream.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_istringstream_move_constructor_transfers_buffer)
{
    ft_istringstream source_stream;
    ft_string source_value;
    int32_t parsed_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("84"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stream.initialize(source_value));
    {
        ft_istringstream moved_stream(static_cast<ft_istringstream &&>(
            source_stream));

        parsed_value = 0;
        moved_stream >> parsed_value;
        FT_ASSERT_EQ(84, parsed_value);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_stream.get_error());
        FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source_stream._initialised_state);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_stream.destroy());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}
