#include "../../Template/function.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <utility>

FT_TEST(test_ft_function_default_empty_reports_error,
    "ft_function default construction yields empty callable state")
{
    ft_function<int(int)> empty_function;
    int invocation_result;

    FT_ASSERT_EQ(false, static_cast<bool>(empty_function));
    ft_errno = FT_ER_SUCCESSS;
    invocation_result = empty_function(7);
    FT_ASSERT_EQ(0, invocation_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, empty_function.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_function_copies_stateful_lambda,
    "ft_function clones callable storage for capturing lambdas")
{
    int accumulator;
    ft_function<int(int)> stateful_callback;
    ft_function<int(int)> copied_callback;

    accumulator = 0;
    stateful_callback = ft_function<int(int)>([&accumulator](int delta) -> int {
        accumulator = accumulator + delta;
        return (accumulator);
    });

    FT_ASSERT_EQ(true, static_cast<bool>(stateful_callback));
    ft_errno = FT_ERR_ALREADY_EXISTS;
    FT_ASSERT_EQ(3, stateful_callback(3));
    FT_ASSERT_EQ(3, accumulator);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, stateful_callback.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);

    copied_callback = stateful_callback;
    FT_ASSERT_EQ(true, static_cast<bool>(copied_callback));
    ft_errno = FT_ERR_CONFIGURATION;
    FT_ASSERT_EQ(6, copied_callback(3));
    FT_ASSERT_EQ(6, accumulator);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copied_callback.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, stateful_callback.get_error());
    return (1);
}

FT_TEST(test_ft_function_move_assignment_transfers_callable,
    "ft_function move assignment clears source and preserves target invocation")
{
    ft_function<int(int)> source([](int value) -> int {
        return (value * 2);
    });
    ft_function<int(int)> destination;

    FT_ASSERT_EQ(true, static_cast<bool>(source));
    FT_ASSERT_EQ(false, static_cast<bool>(destination));

    destination = std::move(source);

    FT_ASSERT_EQ(true, static_cast<bool>(destination));
    FT_ASSERT_EQ(false, static_cast<bool>(source));

    ft_errno = FT_ERR_TERMINATED;
    FT_ASSERT_EQ(10, destination(5));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source.get_error());
    return (1);
}
