#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Observability/observability.hpp"
#include "../../Modules/Parser/document_backend.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_observability_parser_memory_source_counter)
{
    ft_memory_document_source source;
    ft_string output;
    ft_observability_counters counters;

    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_reset_counters(FT_OBSERVABILITY_MODULE_PARSER));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output.initialize());
    source.set_data("parser", 6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.read_all(output));

    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_get_counters(FT_OBSERVABILITY_MODULE_PARSER, &counters));
    FT_ASSERT_EQ(1, counters.operations);
    FT_ASSERT_EQ(1, counters.successes);
    FT_ASSERT_EQ(0, counters.failures);
    FT_ASSERT_EQ(6, counters.bytes_read);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, output.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    return (1);
}
