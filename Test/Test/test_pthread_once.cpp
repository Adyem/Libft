#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/PThread/concurrency.hpp"
#include "../../Modules/Errno/errno.hpp"

static int32_t pthread_once_increment_counter(int32_t *counter)
{
    *counter = *counter + 1;
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_pthread_once_runs_callback_once)
{
    ft_once once_value;
    int32_t counter;

    counter = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, once_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, once_value.call(ft_function<int32_t()>(
            [&counter]() { return (pthread_once_increment_counter(&counter)); })));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, once_value.call(ft_function<int32_t()>(
            [&counter]() { return (pthread_once_increment_counter(&counter)); })));
    FT_ASSERT_EQ(1, counter);
    FT_ASSERT_EQ(FT_TRUE, once_value.has_run());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, once_value.destroy());
    return (1);
}
