#include "../test_internal.hpp"
#include "../../CPP_class/cancellation.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

static int cancellation_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_cancellation_source_get_error_returned = FT_FALSE;
static int32_t g_cancellation_source_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_cancellation_source_get_error_str_returned = FT_FALSE;
static const char *g_cancellation_source_get_error_str_result = ft_nullptr;
static ft_bool g_cancellation_state_get_error_returned = FT_FALSE;
static int32_t g_cancellation_state_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_cancellation_state_get_error_str_returned = FT_FALSE;
static const char *g_cancellation_state_get_error_str_result = ft_nullptr;

static void cancellation_source_get_error_uninitialised_operation(void)
{
    ft_cancellation_source source_value;

    g_cancellation_source_get_error_result = source_value.get_error();
    g_cancellation_source_get_error_returned = FT_TRUE;
    return ;
}

static void cancellation_source_get_error_str_uninitialised_operation(void)
{
    ft_cancellation_source source_value;

    g_cancellation_source_get_error_str_result = source_value.get_error_str();
    g_cancellation_source_get_error_str_returned = FT_TRUE;
    return ;
}

static void cancellation_state_get_error_uninitialised_operation(void)
{
    ft_cancellation_state state_value;

    g_cancellation_state_get_error_result = state_value.get_error();
    g_cancellation_state_get_error_returned = FT_TRUE;
    return ;
}

static void cancellation_state_get_error_str_uninitialised_operation(void)
{
    ft_cancellation_state state_value;

    g_cancellation_state_get_error_str_result = state_value.get_error_str();
    g_cancellation_state_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_cancellation_token_callbacks_after_request)
{
    ft_cancellation_state source_state;
    std::atomic<int32_t> callback_count;
    int32_t registration_status;

    callback_count.store(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_state.initialize());
    registration_status = source_state.register_callback([&callback_count]()
    {
        callback_count.fetch_add(1);
        return ;
    });
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registration_status);

    ft_cancellation_state moved_state(static_cast<ft_cancellation_state &&>(source_state));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_state.request_cancel());
    FT_ASSERT_EQ(1, callback_count.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_state.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_state.destroy());
    return (1);
}

FT_TEST(test_ft_cancellation_source_request_affects_future_tokens)
{
    ft_cancellation_source source_value;
    std::atomic<int32_t> callback_count;
    int32_t registration_status;

    callback_count.store(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize());
    FT_ASSERT_EQ(FT_FALSE, source_value.is_cancellation_requested());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.request_cancel());
    FT_ASSERT_EQ(FT_TRUE, source_value.is_cancellation_requested());
    const ft_cancellation_token token_value(source_value.get_token());
    FT_ASSERT_EQ(FT_TRUE, token_value.is_valid());
    FT_ASSERT_EQ(FT_TRUE, token_value.is_cancellation_requested());
    registration_status = token_value.register_callback([&callback_count]()
    {
        callback_count.fetch_add(1);
        return ;
    });
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registration_status);
    FT_ASSERT_EQ(1, callback_count.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_ft_cancellation_source_error_queries_follow_lifecycle_contract)
{
    ft_cancellation_source source_value;

    g_cancellation_source_get_error_returned = FT_FALSE;
    g_cancellation_source_get_error_result = FT_ERR_SUCCESS;
    g_cancellation_source_get_error_str_returned = FT_FALSE;
    g_cancellation_source_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, cancellation_expect_sigabrt(
        cancellation_source_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_cancellation_source_get_error_returned);
    FT_ASSERT_EQ(1, cancellation_expect_sigabrt(
        cancellation_source_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_cancellation_source_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(source_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}

FT_TEST(test_ft_cancellation_state_error_queries_follow_lifecycle_contract)
{
    ft_cancellation_state state_value;

    g_cancellation_state_get_error_returned = FT_FALSE;
    g_cancellation_state_get_error_result = FT_ERR_SUCCESS;
    g_cancellation_state_get_error_str_returned = FT_FALSE;
    g_cancellation_state_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, cancellation_expect_sigabrt(
        cancellation_state_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_cancellation_state_get_error_returned);
    FT_ASSERT_EQ(1, cancellation_expect_sigabrt(
        cancellation_state_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_cancellation_state_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(state_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
