#include "../test_internal.hpp"
#include "../../Parser/document_backend.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_parser_memory_document_source_read_all_success)
{
    ft_memory_document_source source;
    ft_string output;
    static const char input_data[] = "memory-data";

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output.initialize());
    source.set_data(input_data, sizeof(input_data) - 1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.read_all(output));
    FT_ASSERT_EQ(0, ft_strcmp(output.c_str(), input_data));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    return (1);
}

FT_TEST(test_parser_memory_document_sink_write_all_success)
{
    ft_memory_document_sink sink;
    ft_string storage;
    static const char input_data[] = "stored-data";

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, storage.initialize());
    sink.set_storage(&storage);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink.write_all(input_data, sizeof(input_data) - 1));
    FT_ASSERT_EQ(0, ft_strcmp(storage.c_str(), input_data));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, storage.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, storage.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink.destroy());
    return (1);
}
