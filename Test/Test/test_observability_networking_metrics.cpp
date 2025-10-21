#include "../../Observability/observability_networking_metrics.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"

static ft_networking_observability_sample g_observability_last_sample;
static int g_observability_export_count = 0;

static bool observability_strings_equal(const char *left, const char *right)
{
    size_t index;

    if (left == ft_nullptr || right == ft_nullptr)
        return (false);
    index = 0;
    while (left[index] != '\0' && right[index] != '\0')
    {
        if (left[index] != right[index])
            return (false);
        index = index + 1;
    }
    if (left[index] != '\0')
        return (false);
    if (right[index] != '\0')
        return (false);
    return (true);
}

static void observability_reset_last_sample(void)
{
    g_observability_last_sample.labels.component = ft_nullptr;
    g_observability_last_sample.labels.operation = ft_nullptr;
    g_observability_last_sample.labels.target = ft_nullptr;
    g_observability_last_sample.labels.resource = ft_nullptr;
    g_observability_last_sample.duration_ms = 0;
    g_observability_last_sample.request_bytes = 0;
    g_observability_last_sample.response_bytes = 0;
    g_observability_last_sample.status_code = 0;
    g_observability_last_sample.error_code = 0;
    g_observability_last_sample.error_tag = ft_nullptr;
    g_observability_last_sample.success = false;
    return ;
}

static void observability_test_exporter(const ft_networking_observability_sample &sample)
{
    g_observability_export_count = g_observability_export_count + 1;
    g_observability_last_sample = sample;
    return ;
}

FT_TEST(test_observability_networking_initialize_rejects_null_exporter,
        "observability networking metrics initialize rejects null exporter")
{
    ft_errno = ER_SUCCESS;
    observability_networking_metrics_shutdown();
    FT_ASSERT_EQ(-1, observability_networking_metrics_initialize(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, observability_networking_metrics_get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    observability_networking_metrics_shutdown();
    return (1);
}

FT_TEST(test_observability_networking_record_without_initialize_does_not_emit,
        "observability networking metrics record without initialize does not emit")
{
    ft_networking_observability_sample sample;

    observability_networking_metrics_shutdown();
    observability_reset_last_sample();
    g_observability_export_count = 0;
    ft_errno = FT_ERR_ALREADY_EXISTS;

    sample.labels.component = "http_client";
    sample.labels.operation = "GET";
    sample.labels.target = "example.com";
    sample.labels.resource = "/resource";
    sample.duration_ms = 10;
    sample.request_bytes = 64;
    sample.response_bytes = 128;
    sample.status_code = 200;
    sample.error_code = ER_SUCCESS;
    sample.error_tag = ft_nullptr;
    sample.success = true;

    observability_networking_metrics_record(sample);

    FT_ASSERT_EQ(0, g_observability_export_count);
    FT_ASSERT_EQ(ER_SUCCESS, observability_networking_metrics_get_error());
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, ft_errno);
    return (1);
}

FT_TEST(test_observability_networking_record_populates_defaults,
        "observability networking metrics record populates default values")
{
    ft_networking_observability_sample sample;

    observability_networking_metrics_shutdown();
    observability_reset_last_sample();
    g_observability_export_count = 0;

    FT_ASSERT_EQ(0, observability_networking_metrics_initialize(observability_test_exporter));

    ft_errno = FT_ERR_NO_MEMORY;
    sample.labels.component = "http_client";
    sample.labels.operation = "POST";
    sample.labels.target = "example.org";
    sample.labels.resource = "/submit";
    sample.duration_ms = 42;
    sample.request_bytes = 512;
    sample.response_bytes = 256;
    sample.status_code = 201;
    sample.error_code = ER_SUCCESS;
    sample.error_tag = ft_nullptr;
    sample.success = true;

    observability_networking_metrics_record(sample);

    FT_ASSERT_EQ(1, g_observability_export_count);
    FT_ASSERT_EQ(ER_SUCCESS, g_observability_last_sample.error_code);
    FT_ASSERT(g_observability_last_sample.success);
    FT_ASSERT(g_observability_last_sample.error_tag != ft_nullptr);
    FT_ASSERT(observability_strings_equal("ok", g_observability_last_sample.error_tag));
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);

    FT_ASSERT_EQ(0, observability_networking_metrics_shutdown());
    return (1);
}

FT_TEST(test_observability_networking_record_propagates_error_details,
        "observability networking metrics record propagates error details")
{
    ft_networking_observability_sample sample;
    const char *expected_tag;

    observability_networking_metrics_shutdown();
    observability_reset_last_sample();
    g_observability_export_count = 0;

    FT_ASSERT_EQ(0, observability_networking_metrics_initialize(observability_test_exporter));

    expected_tag = ft_strerror(FT_ERR_CONFIGURATION);
    sample.labels.component = "http_server";
    sample.labels.operation = "ACCEPT";
    sample.labels.target = "0.0.0.0";
    sample.labels.resource = "443";
    sample.duration_ms = 7;
    sample.request_bytes = 0;
    sample.response_bytes = 0;
    sample.status_code = 500;
    sample.error_code = FT_ERR_CONFIGURATION;
    sample.error_tag = ft_nullptr;
    sample.success = false;

    observability_networking_metrics_record(sample);

    FT_ASSERT_EQ(1, g_observability_export_count);
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, g_observability_last_sample.error_code);
    FT_ASSERT(g_observability_last_sample.error_tag != ft_nullptr);
    FT_ASSERT(observability_strings_equal(expected_tag, g_observability_last_sample.error_tag));
    FT_ASSERT(g_observability_last_sample.success == false);

    FT_ASSERT_EQ(0, observability_networking_metrics_shutdown());
    return (1);
}
