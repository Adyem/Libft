#include "../test_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_su_internal_take_abort_reason_round_trip,
        "su_internal_take_abort_reason returns and clears stored reasons")
{
    const char  *captured_reason;

    su_internal_set_abort_reason("unit test abort");
    captured_reason = su_internal_take_abort_reason();
    if (captured_reason == ft_nullptr)
        return (0);
    if (ft_strcmp(captured_reason, "unit test abort") != 0)
        return (0);
    captured_reason = su_internal_take_abort_reason();
    if (captured_reason != ft_nullptr)
        return (0);
    return (1);
}

FT_TEST(test_su_internal_set_abort_reason_overwrites_previous,
        "su_internal_set_abort_reason replaces earlier messages")
{
    const char  *captured_reason;

    su_internal_set_abort_reason("first reason");
    su_internal_set_abort_reason("second reason");
    captured_reason = su_internal_take_abort_reason();
    if (captured_reason == ft_nullptr)
        return (0);
    if (ft_strcmp(captured_reason, "second reason") != 0)
        return (0);
    return (1);
}

