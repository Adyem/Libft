#include "../test_internal.hpp"
#include "../../Template/cancellation.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_cancellation_token_callbacks_after_request,
        "callbacks registered after request_cancel execute immediately")
{
    ft_cancellation_source cancellation_source;
    int callback_count = 0;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_source.request_cancel());
    FT_ASSERT(cancellation_source.is_cancellation_requested());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_source.get_token().register_callback([&callback_count]() {
        callback_count += 1;
        return ;
    }));
    FT_ASSERT_EQ(1, callback_count);
    return (1);
}

FT_TEST(test_ft_cancellation_source_request_affects_future_tokens,
        "request_cancel makes newly fetched tokens see cancellation")
{
    ft_cancellation_source cancellation_source;
    int callback_count = 0;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_source.get_token().register_callback([&callback_count]() {
        callback_count += 1;
        return ;
    }));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_source.request_cancel());
    FT_ASSERT_EQ(1, callback_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_source.get_token().register_callback([&callback_count]() {
        callback_count += 1;
        return ;
    }));
    FT_ASSERT_EQ(2, callback_count);
    return (1);
}
