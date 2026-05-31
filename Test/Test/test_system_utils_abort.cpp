#include "../test_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/system_utils.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_su_internal_take_abort_reason_round_trip)
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

FT_TEST(test_su_internal_set_abort_reason_overwrites_previous)
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
