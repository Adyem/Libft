#include "../test_internal.hpp"
#include "../../CPP_class/cancellation.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

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
