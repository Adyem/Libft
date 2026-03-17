#include "../test_internal.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

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
