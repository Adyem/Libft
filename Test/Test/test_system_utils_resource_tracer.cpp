#include "../test_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/system_utils.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int              g_su_tracer_invocations = 0;
static ft_string        g_su_tracer_reason;

static void su_test_tracer(const char *reason)
{
    g_su_tracer_invocations += 1;
    if (reason != ft_nullptr)
        g_su_tracer_reason = reason;
    else
        g_su_tracer_reason = "";
    return ;
}

FT_TEST(test_su_register_resource_tracer_rejects_null)
{
    if (su_register_resource_tracer(ft_nullptr) != -1)
        return (0);
    return (1);
}

FT_TEST(test_su_resource_tracer_runs_with_reason)
{
    g_su_tracer_invocations = 0;
    g_su_tracer_reason = "";
    su_clear_resource_tracers();
    if (su_register_resource_tracer(&su_test_tracer) != 0)
        return (0);
    su_run_resource_tracers("unit test reason");
    if (g_su_tracer_invocations != 1)
        return (0);
    if (!(g_su_tracer_reason == "unit test reason"))
        return (0);
    su_clear_resource_tracers();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_su_tracer_reason.destroy());
    return (1);
}

FT_TEST(test_su_register_resource_tracer_detects_duplicates)
{
    su_clear_resource_tracers();
    if (su_register_resource_tracer(&su_test_tracer) != 0)
        return (0);
    if (su_register_resource_tracer(&su_test_tracer) != -1)
        return (0);
    su_clear_resource_tracers();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_su_tracer_reason.destroy());
    return (1);
}

FT_TEST(test_su_unregister_resource_tracer_reports_missing)
{
    su_clear_resource_tracers();
    if (su_unregister_resource_tracer(&su_test_tracer) != -1)
        return (0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_su_tracer_reason.destroy());
    return (1);
}

FT_TEST(test_su_unregister_resource_tracer_succeeds)
{
    su_clear_resource_tracers();
    g_su_tracer_invocations = 0;
    g_su_tracer_reason = "";
    if (su_register_resource_tracer(&su_test_tracer) != 0)
        return (0);
    if (su_unregister_resource_tracer(&su_test_tracer) != 0)
        return (0);
    su_run_resource_tracers("after unregister");
    if (g_su_tracer_invocations != 0)
        return (0);
    su_clear_resource_tracers();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_su_tracer_reason.destroy());
    return (1);
}
