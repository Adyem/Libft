#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"

static int su_test_health_success(void *context, ft_string &detail)
{
    (void)context;

    detail = "ready";
    if (detail.get_error() != FT_ER_SUCCESSS)
        return (-1);
    return (0);
}

static int su_test_health_failure(void *context, ft_string &detail)
{
    int *failure_counter;

    failure_counter = static_cast<int *>(context);
    if (failure_counter != ft_nullptr)
        *failure_counter += 1;
    detail = "database unavailable";
    if (detail.get_error() != FT_ER_SUCCESSS)
        return (-1);
    ft_errno = FT_ERR_INTERNAL;
    return (-1);
}

FT_TEST(test_su_health_register_and_run_success,
        "su_health_run_checks reports healthy status when callbacks succeed")
{
    t_su_health_check_result results[1];
    size_t                   count;

    su_health_clear_checks();
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    if (su_health_register_check("ready", &su_test_health_success, ft_nullptr) != 0)
        return (0);
    if (ft_errno != FT_ER_SUCCESSS)
        return (0);
    count = 0;
    if (su_health_run_checks(results, 1, &count) != 0)
        return (0);
    if (count != 1)
        return (0);
    if (!(results[0].name == "ready"))
        return (0);
    if (!results[0].healthy)
        return (0);
    if (!(results[0].detail == "ready"))
        return (0);
    if (results[0].error_code != FT_ER_SUCCESSS)
        return (0);
    su_health_clear_checks();
    return (1);
}

FT_TEST(test_su_health_run_checks_reports_failure,
        "su_health_run_checks captures failing callbacks and surfaces errors")
{
    t_su_health_check_result results[1];
    size_t                   count;
    int                      failure_counter;

    su_health_clear_checks();
    failure_counter = 0;
    if (su_health_register_check("database", &su_test_health_failure, &failure_counter) != 0)
        return (0);
    count = 0;
    if (su_health_run_checks(results, 1, &count) != -1)
        return (0);
    if (ft_errno != FT_ERR_INTERNAL)
        return (0);
    if (count != 1)
        return (0);
    if (!(results[0].name == "database"))
        return (0);
    if (results[0].healthy)
        return (0);
    if (!(results[0].detail == "database unavailable"))
        return (0);
    if (results[0].error_code != FT_ERR_INTERNAL)
        return (0);
    if (failure_counter != 1)
        return (0);
    su_health_clear_checks();
    return (1);
}

FT_TEST(test_su_health_run_check_targets_single_entry,
        "su_health_run_check executes a specific health check by name")
{
    t_su_health_check_result result;

    su_health_clear_checks();
    if (su_health_register_check("database", &su_test_health_success, ft_nullptr) != 0)
        return (0);
    if (su_health_register_check("database", &su_test_health_success, ft_nullptr) != -1)
        return (0);
    if (ft_errno != FT_ERR_ALREADY_EXISTS)
        return (0);
    if (su_health_run_check("database", &result) != 0)
        return (0);
    if (!(result.name == "database"))
        return (0);
    if (!result.healthy)
        return (0);
    if (!(result.detail == "ready"))
        return (0);
    if (result.error_code != FT_ER_SUCCESSS)
        return (0);
    if (su_health_run_check("missing", &result) != -1)
        return (0);
    if (ft_errno != FT_ERR_NOT_FOUND)
        return (0);
    su_health_clear_checks();
    return (1);
}

FT_TEST(test_su_health_unregister_check_validates_name,
        "su_health_unregister_check rejects null identifiers")
{
    su_health_clear_checks();
    ft_errno = FT_ER_SUCCESSS;
    if (su_health_unregister_check(ft_nullptr) != -1)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_su_health_unregister_check_removes_registered_entry,
        "su_health_unregister_check removes existing health checks")
{
    t_su_health_check_result result;

    su_health_clear_checks();
    if (su_health_register_check("cache", &su_test_health_success, ft_nullptr) != 0)
        return (0);
    if (su_health_run_check("cache", &result) != 0)
        return (0);
    if (!(result.name == "cache"))
        return (0);
    if (su_health_unregister_check("cache") != 0)
        return (0);
    if (ft_errno != FT_ER_SUCCESSS)
        return (0);
    if (su_health_run_check("cache", &result) != -1)
        return (0);
    if (ft_errno != FT_ERR_NOT_FOUND)
        return (0);
    su_health_clear_checks();
    return (1);
}

