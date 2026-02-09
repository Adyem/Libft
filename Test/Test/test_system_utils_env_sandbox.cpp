#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_su_environment_snapshot_capture_rejects_null,
    "su_environment_snapshot_capture rejects null snapshot")
{
    ft_errno = FT_ERR_SUCCESSS;
    if (su_environment_snapshot_capture(ft_nullptr) != -1)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_su_environment_snapshot_restore_round_trip,
    "su_environment_snapshot_restore restores environment mutations")
{
    const char  *variable_one = "FT_SU_SANDBOX_ONE";
    const char  *variable_two = "FT_SU_SANDBOX_TWO";
    const char  *variable_extra = "FT_SU_SANDBOX_EXTRA";
    t_su_environment_snapshot snapshot;
    char        *value;

    if (ft_setenv(variable_one, "original_one", 1) != 0)
        return (0);
    if (ft_setenv(variable_two, "original_two", 1) != 0)
    {
        ft_unsetenv(variable_one);
        return (0);
    }
    if (su_environment_snapshot_capture(&snapshot) != 0)
    {
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        return (0);
    }
    if (ft_setenv(variable_one, "mutated", 1) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        return (0);
    }
    if (ft_unsetenv(variable_two) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        return (0);
    }
    if (ft_setenv(variable_extra, "temporary", 1) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        ft_unsetenv(variable_extra);
        return (0);
    }
    if (su_environment_snapshot_restore(&snapshot) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        ft_unsetenv(variable_extra);
        return (0);
    }
    value = ft_getenv(variable_one);
    if (value == ft_nullptr || ft_strcmp(value, "original_one") != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        ft_unsetenv(variable_extra);
        return (0);
    }
    value = ft_getenv(variable_two);
    if (value == ft_nullptr || ft_strcmp(value, "original_two") != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        ft_unsetenv(variable_extra);
        return (0);
    }
    value = ft_getenv(variable_extra);
    if (value != ft_nullptr)
    {
        su_environment_snapshot_dispose(&snapshot);
        ft_unsetenv(variable_one);
        ft_unsetenv(variable_two);
        ft_unsetenv(variable_extra);
        return (0);
    }
    su_environment_snapshot_dispose(&snapshot);
    ft_unsetenv(variable_one);
    ft_unsetenv(variable_two);
    ft_unsetenv(variable_extra);
    return (1);
}

FT_TEST(test_su_environment_sandbox_end_restores_state,
    "su_environment_sandbox_end restores sandbox mutations")
{
    const char  *variable_name = "FT_SU_SANDBOX_AUTOCLEAR";
    t_su_environment_snapshot sandbox;

    ft_unsetenv(variable_name);
    if (su_environment_sandbox_begin(&sandbox) != 0)
        return (0);
    if (ft_setenv(variable_name, "sandboxed", 1) != 0)
    {
        su_environment_sandbox_end(&sandbox);
        ft_unsetenv(variable_name);
        return (0);
    }
    if (su_environment_sandbox_end(&sandbox) != 0)
    {
        ft_unsetenv(variable_name);
        return (0);
    }
    if (ft_getenv(variable_name) != ft_nullptr)
    {
        ft_unsetenv(variable_name);
        return (0);
    }
    return (1);
}
