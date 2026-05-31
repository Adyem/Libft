#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Threading/concurrency.hpp"
#include "../../Modules/Template/promise.hpp"
#include "../../Modules/Threading/cancellation.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_pthread_async_returns_terminated_for_precancelled_token)
{
    ft_cancellation_source source;
    ft_promise<int32_t> promise;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.request_cancel());
    FT_ASSERT_EQ(FT_ERR_TERMINATED, pt_async(promise, source.get_token(), []()
        { return (42); }));
    FT_ASSERT_EQ(FT_TRUE, promise.is_ready());
    FT_ASSERT_EQ(0, promise.get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    return (1);
}

FT_TEST(test_pthread_async_runs_when_token_is_active)
{
    ft_cancellation_source source;
    ft_promise<int32_t> promise;
    int32_t value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pt_async(promise, source.get_token(), []()
        { return (99); }));
    while (promise.is_ready() == FT_FALSE)
        pt_thread_sleep(1);
    value = promise.get();
    FT_ASSERT_EQ(99, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    return (1);
}
