#include "../test_internal.hpp"
#include "../../Template/cancellation.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_cancellation_token_callbacks_after_request,
        "ft_cancellation_token invokes callbacks registered after cancellation")
{
    ft_cancellation_source cancellation_source;
    ft_cancellation_token cancellation_token;
    int callback_count;
    int registration_result;

    cancellation_token = cancellation_source.get_token();
    FT_ASSERT(cancellation_token.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_token.get_error());

    cancellation_source.request_cancel();
    FT_ASSERT(cancellation_source.is_cancellation_requested());
    FT_ASSERT(cancellation_token.is_cancellation_requested());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_token.get_error());

    callback_count = 0;
    registration_result = cancellation_token.register_callback([&callback_count]() {
        callback_count = callback_count + 1;
        return ;
    });
    FT_ASSERT_EQ(FT_ERR_SUCCESS, registration_result);
    FT_ASSERT_EQ(1, callback_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cancellation_token.get_error());
    return (1);
}

FT_TEST(test_ft_cancellation_source_move_preserves_shared_state,
        "ft_cancellation_source move operations preserve cancellation state across tokens")
{
    ft_cancellation_source original_source;
    ft_cancellation_token shared_token;
    ft_cancellation_source moved_source;
    ft_cancellation_token additional_token;

    shared_token = original_source.get_token();
    FT_ASSERT(shared_token.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shared_token.get_error());

    moved_source = ft_cancellation_source(ft_move(original_source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_source.get_error());

    additional_token = moved_source.get_token();
    FT_ASSERT(additional_token.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, additional_token.get_error());

    ft_cancellation_token moved_token(ft_move(additional_token));
    FT_ASSERT(moved_token.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_token.get_error());
    FT_ASSERT(additional_token.is_valid() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, additional_token.get_error());

    FT_ASSERT(shared_token.is_cancellation_requested() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shared_token.get_error());

    moved_source.request_cancel();
    FT_ASSERT(moved_source.is_cancellation_requested());
    FT_ASSERT(shared_token.is_cancellation_requested());
    FT_ASSERT(moved_token.is_cancellation_requested());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shared_token.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_token.get_error());
    return (1);
}
