#include "../test_internal.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

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

FT_TEST(test_ft_istringstream_destroy_tolerates_destroyed_instance,
    "ft_istringstream destroy tolerates destroyed instance")
{
    ft_string source_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("42"));
    ft_istringstream stream_value(source_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, stream_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}
