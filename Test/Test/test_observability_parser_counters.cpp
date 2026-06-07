#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Observability/observability.hpp"
#include "../../Modules/Observability/observability_histogram.hpp"
#include "../../Modules/Parser/document_backend.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include <cstring>

struct histogram_export_capture
{
    char buffer[512];
    ft_size_t length;
    ft_bool overflow;
};

static void test_observability_histogram_export_capture(const char *chunk,
    void *user_data)
{
    histogram_export_capture *capture;
    ft_size_t chunk_length;
    ft_size_t index;

    capture = static_cast<histogram_export_capture *>(user_data);
    if (capture == ft_nullptr || capture->overflow == FT_TRUE || chunk == ft_nullptr)
    {
        if (capture != ft_nullptr)
            capture->overflow = FT_TRUE;
        return ;
    }
    chunk_length = std::strlen(chunk);
    if (capture->length + chunk_length >= sizeof(capture->buffer))
    {
        capture->overflow = FT_TRUE;
        return ;
    }
    index = 0;
    while (index < chunk_length)
    {
        capture->buffer[capture->length + index] = chunk[index];
        index += 1;
    }
    capture->length += chunk_length;
    capture->buffer[capture->length] = '\0';
    return ;
}

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

FT_TEST(test_observability_histogram_records_and_exports)
{
    ft_observability_histogram_bucket buckets[3];
    ft_observability_histogram histogram;
    histogram_export_capture capture;
    int32_t error_code;

    buckets[0].upper_bound = 1.0;
    buckets[1].upper_bound = 10.0;
    buckets[2].upper_bound = 100.0;
    capture.length = 0;
    capture.overflow = FT_FALSE;
    capture.buffer[0] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_histogram_initialize(&histogram, "latency", "ms",
            buckets, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, observability_histogram_record(&histogram, 0.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, observability_histogram_record(&histogram, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, observability_histogram_record(&histogram, 15.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, observability_histogram_record(&histogram, 200.0));
    FT_ASSERT_EQ(1, buckets[0].count);
    FT_ASSERT_EQ(2, buckets[1].count);
    FT_ASSERT_EQ(4, buckets[2].count);
    FT_ASSERT_EQ(4, histogram.count);
    FT_ASSERT_EQ(220.5, histogram.sum);

    error_code = observability_histogram_export_text(&histogram,
            test_observability_histogram_export_capture, &capture);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(FT_FALSE, capture.overflow);
    FT_ASSERT_EQ(0, std::strcmp(capture.buffer,
            "name=latency unit=ms count=4 sum=220.5 "
            "buckets=[{le=1,count=1},{le=10,count=2},{le=100,count=4}]"));

    capture.length = 0;
    capture.overflow = FT_FALSE;
    capture.buffer[0] = '\0';
    error_code = observability_histogram_export_json(&histogram,
            test_observability_histogram_export_capture, &capture);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(FT_FALSE, capture.overflow);
    FT_ASSERT_EQ(0, std::strcmp(capture.buffer,
            "{\"name\":\"latency\",\"unit\":\"ms\",\"count\":4,"
            "\"sum\":220.5,\"buckets\":[{\"upper_bound\":1,\"count\":1},"
            "{\"upper_bound\":10,\"count\":2},{\"upper_bound\":100,\"count\":4}]}"));

    observability_histogram_clear(&histogram);
    FT_ASSERT_EQ(0, buckets[0].count);
    FT_ASSERT_EQ(0, buckets[1].count);
    FT_ASSERT_EQ(0, buckets[2].count);
    FT_ASSERT_EQ(0, histogram.count);
    FT_ASSERT_EQ(0.0, histogram.sum);
    return (1);
}
