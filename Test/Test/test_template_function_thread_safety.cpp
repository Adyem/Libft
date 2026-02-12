#include "../test_internal.hpp"
#include "../../Template/function.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int increment_value(int value)
{
    return (value + 1);
}

FT_TEST(test_function_invokes_callable,
        "ft_function forwards calls to the stored callable")
{
    ft_function<int(int)> function(increment_value);

    FT_ASSERT_EQ(true, static_cast<bool>(function));
    FT_ASSERT_EQ(6, function(5));
    return (1);
}

FT_TEST(test_function_empty_callable_returns_default,
        "Calling an empty ft_function returns the default value")
{
    ft_function<int()> empty_function;

    FT_ASSERT_EQ(false, static_cast<bool>(empty_function));
    FT_ASSERT_EQ(0, empty_function());
    return (1);
}
