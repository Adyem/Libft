#include "../test_internal.hpp"
#include "../../Modules/API/api_request_metrics.hpp"
#include "../../Modules/Observability/observability_networking_metrics.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_networking_observability_sample g_api_request_metrics_sample;
static int32_t g_api_request_metrics_export_count = 0;

static void api_request_metrics_reset_sample(void)
{
    g_api_request_metrics_sample.labels.component = ft_nullptr;
    g_api_request_metrics_sample.labels.operation = ft_nullptr;
    g_api_request_metrics_sample.labels.target = ft_nullptr;
    g_api_request_metrics_sample.labels.resource = ft_nullptr;
    g_api_request_metrics_sample.duration_ms = 0;
    g_api_request_metrics_sample.request_bytes = 0;
    g_api_request_metrics_sample.response_bytes = 0;
    g_api_request_metrics_sample.status_code = 0;
    g_api_request_metrics_sample.error_code = 0;
    g_api_request_metrics_sample.error_tag = ft_nullptr;
    g_api_request_metrics_sample.success = FT_FALSE;
    g_api_request_metrics_export_count = 0;
    return ;
}

static void api_request_metrics_capture_export(
    const ft_networking_observability_sample &sample)
{
    g_api_request_metrics_sample = sample;
    g_api_request_metrics_export_count += 1;
    return ;
}

FT_TEST(test_api_request_metrics_guard_initialize_builds_endpoint)
{
    api_request_metrics_guard guard;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, guard.initialize("example.com", 443,
        "GET", "/health", 12, ft_nullptr, ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, guard._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, guard._endpoint.get_error());
    FT_ASSERT(guard._endpoint == "example.com:443");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, guard.destroy());
    return (1);
}

FT_TEST(test_api_request_metrics_guard_destructor_records_sample)
{
    char *result_body;
    int32_t status_code;
    int32_t error_code;

    observability_networking_metrics_shutdown();
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_networking_metrics_enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_networking_metrics_initialize(
            api_request_metrics_capture_export));
    api_request_metrics_reset_sample();

    result_body = const_cast<char *>("payload");
    status_code = 204;
    error_code = FT_ERR_SUCCESS;
    {
        api_request_metrics_guard guard;

        FT_ASSERT_EQ(FT_ERR_SUCCESS, guard.initialize("service.local", 8080,
            "POST", "/submit", 17, &result_body, &status_code, &error_code));
        FT_ASSERT_EQ(FT_ERR_SUCCESS, guard._endpoint.get_error());
    }

    FT_ASSERT_EQ(1, g_api_request_metrics_export_count);
    FT_ASSERT(g_api_request_metrics_sample.labels.target != ft_nullptr);
    FT_ASSERT(ft_strcmp(g_api_request_metrics_sample.labels.target,
        "service.local:8080") == 0);
    FT_ASSERT_EQ(static_cast<ft_size_t>(17),
        g_api_request_metrics_sample.request_bytes);
    FT_ASSERT_EQ(static_cast<ft_size_t>(7),
        g_api_request_metrics_sample.response_bytes);
    FT_ASSERT_EQ(204, g_api_request_metrics_sample.status_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_api_request_metrics_sample.error_code);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, observability_networking_metrics_shutdown());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_networking_metrics_disable_thread_safety());
    return (1);
}
