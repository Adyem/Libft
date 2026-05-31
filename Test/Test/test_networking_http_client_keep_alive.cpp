#include "../test_internal.hpp"
#include "../../Modules/Networking/http_client.hpp"
#include "../../Modules/Networking/http_server.hpp"
#include "../../Modules/Networking/networking.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Threading/thread.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#if NETWORKING_HAS_OPENSSL
#define FT_TEST_REQUIRE(condition) \
    do \
    { \
        if (!(condition)) \
        { \
            ft_test_fail(#condition, __FILE__, __LINE__); \
            goto cleanup; \
        } \
    } while (0)

struct http_client_server_context
{
    ft_http_server *server;
    int result;
};

static void http_client_server_run_once(http_client_server_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    context->result = context->server->run_once();
    return ;
}

FT_TEST(test_networking_http_client_reuses_keep_alive_connection)
{
    ft_http_server server;
    http_client_server_context context;
    ft_thread server_thread;
    ft_string response;
    int first_result;
    int second_result;
    ft_bool pool_thread_safety_enabled;
    ft_bool server_initialized;
    ft_bool response_initialized;
    ft_bool server_thread_started;
    ft_bool test_passed;

    pool_thread_safety_enabled = FT_FALSE;
    server_initialized = FT_FALSE;
    response_initialized = FT_FALSE;
    server_thread_started = FT_FALSE;
    test_passed = FT_FALSE;
    context.result = FT_ERR_INVALID_STATE;
    FT_TEST_REQUIRE(http_client_pool_enable_thread_safety() == FT_ERR_SUCCESS);
    pool_thread_safety_enabled = FT_TRUE;
    http_client_pool_flush();
    http_client_pool_debug_reset_counters();
    FT_TEST_REQUIRE(server.initialize() == FT_ERR_SUCCESS);
    server_initialized = FT_TRUE;
    FT_TEST_REQUIRE(response.initialize() == FT_ERR_SUCCESS);
    response_initialized = FT_TRUE;
    FT_TEST_REQUIRE(server.start("127.0.0.1", 54340) == FT_ERR_SUCCESS);
    context.server = &server;
    server_thread = ft_thread(http_client_server_run_once, &context);
    FT_TEST_REQUIRE(server_thread.joinable());
    server_thread_started = FT_TRUE;
    first_result = http_get("127.0.0.1", "/first", response, false, "54340");
    FT_TEST_REQUIRE(first_result == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(http_client_pool_get_idle_count() == 1);
    FT_TEST_REQUIRE(http_client_pool_debug_get_miss_count() > 0);
    response.clear();
    second_result = http_get("127.0.0.1", "/second", response, false, "54340");
    FT_TEST_REQUIRE(second_result == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(http_client_pool_debug_get_reuse_count() > 0);
    server_thread.join();
    server_thread_started = FT_FALSE;
    FT_TEST_REQUIRE(context.result == FT_ERR_SUCCESS
        || context.result == FT_ERR_INVALID_OPERATION);
    test_passed = FT_TRUE;
cleanup:
    http_client_pool_flush();
    if (server_thread_started == FT_TRUE)
        server_thread.join();
    if (response_initialized == FT_TRUE)
        FT_ASSERT_EQ(FT_ERR_SUCCESS, response.destroy());
    if (server_initialized == FT_TRUE)
        FT_ASSERT_EQ(FT_ERR_SUCCESS, server.destroy());
    if (pool_thread_safety_enabled == FT_TRUE)
        (void)http_client_pool_disable_thread_safety();
    if (test_passed == FT_FALSE)
        return (0);
    if (http_client_pool_get_idle_count() != 0)
    {
        ft_test_fail("http_client_pool_get_idle_count() == 0", __FILE__, __LINE__);
        return (0);
    }
    return (1);
}
#endif
