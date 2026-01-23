#include "../../Template/function.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_function_default_empty_callable,
    "ft_function default construction yields an empty callable")
{
    ft_function<int(int)> empty_function;

    FT_ASSERT_EQ(false, static_cast<bool>(empty_function));
    FT_ASSERT_EQ(0, empty_function(7));
    FT_ASSERT_EQ(false, static_cast<bool>(empty_function));
    return (1);
}

FT_TEST(test_ft_function_invokes_callable,
    "ft_function calls the stored callable with provided arguments")
{
    ft_function<int(int)> callable([](int value) -> int {
        return (value * 2);
    });

    FT_ASSERT_EQ(true, static_cast<bool>(callable));
    FT_ASSERT_EQ(10, callable(5));
    return (1);
}
