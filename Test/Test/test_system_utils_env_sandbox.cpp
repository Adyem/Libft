#include "../test_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/system_utils.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_su_environment_snapshot_capture_rejects_null)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, su_environment_snapshot_capture(ft_nullptr));
    return (1);
}

FT_TEST(test_su_environment_snapshot_restore_round_trip)
{
    const char  *variable_one = "FT_SU_SANDBOX_ONE";
    const char  *variable_two = "FT_SU_SANDBOX_TWO";
    const char  *variable_extra = "FT_SU_SANDBOX_EXTRA";
    t_su_environment_snapshot snapshot;
    char        *value;

    if (su_setenv(variable_one, "original_one", 1) != 0)
        return (0);
    if (su_setenv(variable_two, "original_two", 1) != 0)
    {
        su_unsetenv(variable_one);
        return (0);
    }
    if (su_environment_snapshot_capture(&snapshot) != 0)
    {
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        return (0);
    }
    if (su_setenv(variable_one, "mutated", 1) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        return (0);
    }
    if (su_unsetenv(variable_two) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        return (0);
    }
    if (su_setenv(variable_extra, "temporary", 1) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        su_unsetenv(variable_extra);
        return (0);
    }
    if (su_environment_snapshot_restore(&snapshot) != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        su_unsetenv(variable_extra);
        return (0);
    }
    value = su_getenv(variable_one);
    if (value == ft_nullptr || ft_strcmp(value, "original_one") != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        su_unsetenv(variable_extra);
        return (0);
    }
    value = su_getenv(variable_two);
    if (value == ft_nullptr || ft_strcmp(value, "original_two") != 0)
    {
        su_environment_snapshot_dispose(&snapshot);
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        su_unsetenv(variable_extra);
        return (0);
    }
    value = su_getenv(variable_extra);
    if (value != ft_nullptr)
    {
        su_environment_snapshot_dispose(&snapshot);
        su_unsetenv(variable_one);
        su_unsetenv(variable_two);
        su_unsetenv(variable_extra);
        return (0);
    }
    su_environment_snapshot_dispose(&snapshot);
    su_unsetenv(variable_one);
    su_unsetenv(variable_two);
    su_unsetenv(variable_extra);
    return (1);
}

FT_TEST(test_su_environment_sandbox_end_restores_state)
{
    const char  *variable_name = "FT_SU_SANDBOX_AUTOCLEAR";
    t_su_environment_snapshot sandbox;

    su_unsetenv(variable_name);
    if (su_environment_sandbox_begin(&sandbox) != 0)
        return (0);
    if (su_setenv(variable_name, "sandboxed", 1) != 0)
    {
        su_environment_sandbox_end(&sandbox);
        su_unsetenv(variable_name);
        return (0);
    }
    if (su_environment_sandbox_end(&sandbox) != 0)
    {
        su_unsetenv(variable_name);
        return (0);
    }
    if (su_getenv(variable_name) != ft_nullptr)
    {
        su_unsetenv(variable_name);
        return (0);
    }
    return (1);
}
