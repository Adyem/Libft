#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int g_system_utils_additional_tracer_counter = 0;
static int g_system_utils_additional_handler_counter = 0;

static void system_utils_additional_tracer(const char *reason)
{
    (void)reason;
    g_system_utils_additional_tracer_counter += 1;
    return ;
}

static int system_utils_additional_health_ok(void *context, ft_string &detail)
{
    (void)context;
    if (detail.assign("ok", 2) != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

static int system_utils_additional_health_fail(void *context, ft_string &detail)
{
    (void)context;
    if (detail.assign("fail", 4) != FT_ERR_SUCCESS)
        return (-1);
    return (-1);
}

static void system_utils_additional_service_handler(int signal_number, void *user_context)
{
    (void)signal_number;
    (void)user_context;
    g_system_utils_additional_handler_counter += 1;
    return ;
}

FT_TEST(test_su_environment_enable_thread_safety_reports_success,
    "su_environment_enable_thread_safety succeeds")
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    return (1);
}

FT_TEST(test_su_environment_disable_thread_safety_is_idempotent,
    "su_environment_disable_thread_safety can be called repeatedly")
{
    su_environment_disable_thread_safety();
    su_environment_disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    return (1);
}

FT_TEST(test_su_setenv_and_getenv_round_trip,
    "su_setenv and su_getenv round trip a variable")
{
    char *value;

    FT_ASSERT_EQ(0, su_setenv("FT_SU_ADD_ENV_1", "value_one", 1));
    value = su_getenv("FT_SU_ADD_ENV_1");
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ('v', value[0]);
    FT_ASSERT_EQ(0, ft_unsetenv("FT_SU_ADD_ENV_1"));
    return (1);
}

FT_TEST(test_su_setenv_overwrite_disabled_preserves_value,
    "su_setenv with overwrite zero preserves existing value")
{
    char *value;

    FT_ASSERT_EQ(0, su_setenv("FT_SU_ADD_ENV_2", "original", 1));
    FT_ASSERT_EQ(0, su_setenv("FT_SU_ADD_ENV_2", "mutated", 0));
    value = su_getenv("FT_SU_ADD_ENV_2");
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ('o', value[0]);
    FT_ASSERT_EQ(0, ft_unsetenv("FT_SU_ADD_ENV_2"));
    return (1);
}

FT_TEST(test_su_setenv_null_name_fails,
    "su_setenv rejects null names")
{
    FT_ASSERT_EQ(-1, su_setenv(ft_nullptr, "value", 1));
    return (1);
}

FT_TEST(test_su_putenv_null_pointer_returns_invalid_argument,
    "su_putenv returns FT_ERR_INVALID_ARGUMENT for null input")
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, su_putenv(ft_nullptr));
    return (1);
}

FT_TEST(test_su_environment_snapshot_capture_null_rejected,
    "su_environment_snapshot_capture rejects null snapshot")
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, su_environment_snapshot_capture(ft_nullptr));
    return (1);
}

FT_TEST(test_su_environment_snapshot_restore_null_rejected,
    "su_environment_snapshot_restore rejects null snapshot")
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, su_environment_snapshot_restore(ft_nullptr));
    return (1);
}

FT_TEST(test_su_environment_snapshot_capture_and_restore_succeeds,
    "su_environment_snapshot capture and restore return success")
{
    t_su_environment_snapshot snapshot;

    FT_ASSERT_EQ(0, su_setenv("FT_SU_ADD_ENV_3", "before", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_snapshot_capture(&snapshot));
    FT_ASSERT_EQ(0, su_setenv("FT_SU_ADD_ENV_3", "after", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_snapshot_restore(&snapshot));
    su_environment_snapshot_dispose(&snapshot);
    FT_ASSERT_EQ(0, ft_unsetenv("FT_SU_ADD_ENV_3"));
    return (1);
}

FT_TEST(test_su_environment_sandbox_begin_end_succeeds,
    "su_environment_sandbox_begin and end return success")
{
    t_su_environment_snapshot sandbox_snapshot;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_sandbox_begin(&sandbox_snapshot));
    FT_ASSERT_EQ(0, su_setenv("FT_SU_ADD_ENV_4", "inside", 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_sandbox_end(&sandbox_snapshot));
    FT_ASSERT_EQ(0, ft_unsetenv("FT_SU_ADD_ENV_4"));
    return (1);
}

FT_TEST(test_su_locale_compare_rejects_null_result,
    "su_locale_compare rejects null output pointers")
{
    FT_ASSERT_EQ(-1, su_locale_compare("a", "b", ft_nullptr, ft_nullptr));
    return (1);
}

FT_TEST(test_su_locale_compare_equal_strings_returns_zero,
    "su_locale_compare reports equal strings as zero")
{
    int compare_result;

    compare_result = 42;
    FT_ASSERT_EQ(0, su_locale_compare("same", "same", ft_nullptr, &compare_result));
    FT_ASSERT_EQ(0, compare_result);
    return (1);
}

FT_TEST(test_su_locale_compare_orders_strings,
    "su_locale_compare orders strings lexically")
{
    int compare_result;

    compare_result = 0;
    FT_ASSERT_EQ(0, su_locale_compare("alpha", "beta", ft_nullptr, &compare_result));
    FT_ASSERT(compare_result < 0);
    return (1);
}

FT_TEST(test_su_locale_casefold_rejects_null_input,
    "su_locale_casefold rejects null input")
{
    ft_string output;

    FT_ASSERT_EQ(-1, su_locale_casefold(ft_nullptr, ft_nullptr, output));
    return (1);
}

FT_TEST(test_su_locale_casefold_lowercases_text,
    "su_locale_casefold lowercases ASCII text")
{
    ft_string output;

    FT_ASSERT_EQ(0, su_locale_casefold("AbC", ft_nullptr, output));
    FT_ASSERT(output == "abc");
    return (1);
}

FT_TEST(test_su_health_register_rejects_null_name,
    "su_health_register_check rejects null names")
{
    su_health_clear_checks();
    FT_ASSERT_EQ(-1, su_health_register_check(ft_nullptr, system_utils_additional_health_ok, ft_nullptr));
    return (1);
}

FT_TEST(test_su_health_register_rejects_empty_name,
    "su_health_register_check rejects empty names")
{
    su_health_clear_checks();
    FT_ASSERT_EQ(-1, su_health_register_check("", system_utils_additional_health_ok, ft_nullptr));
    return (1);
}

FT_TEST(test_su_health_register_rejects_null_callback,
    "su_health_register_check rejects null callbacks")
{
    su_health_clear_checks();
    FT_ASSERT_EQ(-1, su_health_register_check("health", ft_nullptr, ft_nullptr));
    return (1);
}

FT_TEST(test_su_health_register_rejects_duplicate_name,
    "su_health_register_check rejects duplicate names")
{
    su_health_clear_checks();
    FT_ASSERT_EQ(0, su_health_register_check("health_dup", system_utils_additional_health_ok, ft_nullptr));
    FT_ASSERT_EQ(-1, su_health_register_check("health_dup", system_utils_additional_health_ok, ft_nullptr));
    su_health_clear_checks();
    return (1);
}

FT_TEST(test_su_health_run_checks_requires_count_pointer,
    "su_health_run_checks requires count output")
{
    su_health_clear_checks();
    FT_ASSERT_EQ(-1, su_health_run_checks(ft_nullptr, 0, ft_nullptr));
    return (1);
}

FT_TEST(test_su_health_run_check_reports_missing_name,
    "su_health_run_check reports missing checks")
{
    t_su_health_check_result result;

    su_health_clear_checks();
    FT_ASSERT_EQ(-1, su_health_run_check("missing_check", &result));
    return (1);
}

FT_TEST(test_su_health_run_checks_zero_checks_reports_zero_count,
    "su_health_run_checks reports zero checks when registry is empty")
{
    size_t count;

    su_health_clear_checks();
    count = 99;
    FT_ASSERT_EQ(0, su_health_run_checks(ft_nullptr, 0, &count));
    FT_ASSERT_EQ(0, static_cast<int>(count));
    return (1);
}

FT_TEST(test_su_health_run_checks_surfaces_failing_callback,
    "su_health_run_checks returns failure for failing callback")
{
    t_su_health_check_result result_buffer[1];
    size_t count;

    su_health_clear_checks();
    FT_ASSERT_EQ(0, su_health_register_check("health_fail", system_utils_additional_health_fail, ft_nullptr));
    count = 0;
    FT_ASSERT_EQ(-1, su_health_run_checks(result_buffer, 1, &count));
    FT_ASSERT_EQ(1, static_cast<int>(count));
    su_health_clear_checks();
    return (1);
}

FT_TEST(test_su_register_resource_tracer_rejects_null,
    "su_register_resource_tracer rejects null tracer")
{
    su_clear_resource_tracers();
    FT_ASSERT_EQ(-1, su_register_resource_tracer(ft_nullptr));
    return (1);
}

FT_TEST(test_su_register_resource_tracer_rejects_duplicate,
    "su_register_resource_tracer rejects duplicates")
{
    su_clear_resource_tracers();
    FT_ASSERT_EQ(0, su_register_resource_tracer(system_utils_additional_tracer));
    FT_ASSERT_EQ(-1, su_register_resource_tracer(system_utils_additional_tracer));
    su_clear_resource_tracers();
    return (1);
}

FT_TEST(test_su_unregister_resource_tracer_missing_fails,
    "su_unregister_resource_tracer fails when tracer is missing")
{
    su_clear_resource_tracers();
    FT_ASSERT_EQ(-1, su_unregister_resource_tracer(system_utils_additional_tracer));
    return (1);
}

FT_TEST(test_su_run_resource_tracers_invokes_registered_tracer,
    "su_run_resource_tracers invokes callbacks")
{
    g_system_utils_additional_tracer_counter = 0;
    su_clear_resource_tracers();
    FT_ASSERT_EQ(0, su_register_resource_tracer(system_utils_additional_tracer));
    su_run_resource_tracers("reason");
    FT_ASSERT_EQ(1, g_system_utils_additional_tracer_counter);
    su_clear_resource_tracers();
    return (1);
}

FT_TEST(test_su_service_install_signal_handlers_and_clear,
    "su_service_install_signal_handlers and clear succeed")
{
    g_system_utils_additional_handler_counter = 0;
    FT_ASSERT_EQ(0, su_service_install_signal_handlers(system_utils_additional_service_handler, ft_nullptr));
    su_service_clear_signal_handlers();
    return (1);
}
