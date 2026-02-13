#include "../test_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"

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

FT_TEST(test_su_register_resource_tracer_rejects_null, "su_register_resource_tracer rejects null")
{
    if (su_register_resource_tracer(ft_nullptr) != -1)
        return (0);
    return (1);
}

FT_TEST(test_su_resource_tracer_runs_with_reason, "su_run_resource_tracers invokes registered tracers")
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
    return (1);
}

FT_TEST(test_su_register_resource_tracer_detects_duplicates, "su_register_resource_tracer prevents duplicates")
{
    su_clear_resource_tracers();
    if (su_register_resource_tracer(&su_test_tracer) != 0)
        return (0);
    if (su_register_resource_tracer(&su_test_tracer) != -1)
        return (0);
    su_clear_resource_tracers();
    return (1);
}

FT_TEST(test_su_unregister_resource_tracer_reports_missing, "su_unregister_resource_tracer reports missing tracer")
{
    su_clear_resource_tracers();
    if (su_unregister_resource_tracer(&su_test_tracer) != -1)
        return (0);
    return (1);
}

FT_TEST(test_su_unregister_resource_tracer_succeeds, "su_unregister_resource_tracer removes tracer")
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
    return (1);
}
