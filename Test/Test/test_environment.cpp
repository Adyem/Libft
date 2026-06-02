#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/system_utils.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "compatebility_system_test_hooks.hpp"
#include <cerrno>
#include <cstring>

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_unsetenv_rejects_empty_name)
{
    FT_ASSERT_EQ(-1, su_unsetenv(""));
    return (1);
}

FT_TEST(test_ft_getenv_empty_name_sets_errno)
{
    FT_ASSERT_EQ(ft_nullptr, su_getenv(""));
    return (1);
}

FT_TEST(test_ft_getenv_null_name_sets_errno)
{
    FT_ASSERT_EQ(ft_nullptr, su_getenv(ft_nullptr));
    return (1);
}

FT_TEST(test_ft_getenv_missing_clears_errno)
{
    const char *variable_name;

    variable_name = "LIBFT_TEST_GETENV_MISSING";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    su_unsetenv(variable_name);
    FT_ASSERT_EQ(ft_nullptr, su_getenv(variable_name));
    su_environment_disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_getenv_returns_value)
{
    const char *variable_name;
    char *value;

    variable_name = "LIBFT_TEST_GETENV_PRESENT";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    FT_ASSERT_EQ(0, su_setenv(variable_name, "present", 1));
    value = su_getenv(variable_name);
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(value, "present"));
    su_unsetenv(variable_name);
    su_environment_disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_setenv_null_value_sets_errno)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        su_setenv("LIBFT_TEST_NULL_VALUE", ft_nullptr, 1));
    return (1);
}

FT_TEST(test_ft_setenv_null_name_sets_errno)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, su_setenv(ft_nullptr, "value", 1));
    return (1);
}

FT_TEST(test_ft_setenv_empty_name_sets_errno)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, su_setenv("", "value", 1));
    return (1);
}

FT_TEST(test_ft_setenv_equals_sign_sets_errno)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION,
        su_setenv("INVALID=NAME", "value", 1));
    return (1);
}

FT_TEST(test_ft_setenv_success_resets_errno)
{
    const char *variable_name;
    char *value;

    variable_name = "LIBFT_TEST_SETENV_SUCCESS";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    FT_ASSERT_EQ(0, su_setenv(variable_name, "stored", 1));
    value = su_getenv(variable_name);
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(value, "stored"));
    su_unsetenv(variable_name);
    su_environment_disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_setenv_overwrite_disabled_preserves_existing)
{
    const char *variable_name;
    char *value;

    variable_name = "LIBFT_TEST_SETENV_NO_OVERWRITE";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    FT_ASSERT_EQ(0, su_setenv(variable_name, "initial", 1));
    FT_ASSERT_EQ(0, su_setenv(variable_name, "replacement", 0));
    value = su_getenv(variable_name);
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(value, "initial"));
    FT_ASSERT_EQ(0, su_unsetenv(variable_name));
    su_environment_disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_setenv_overwrite_disabled_creates_variable)
{
    const char *variable_name;
    char *value;

    variable_name = "LIBFT_TEST_SETENV_CREATE_NO_OVERWRITE";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    su_unsetenv(variable_name);
    FT_ASSERT_EQ(0, su_setenv(variable_name, "created", 0));
    value = su_getenv(variable_name);
    FT_ASSERT(value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(value, "created"));
    FT_ASSERT_EQ(0, su_unsetenv(variable_name));
    su_environment_disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_unsetenv_rejects_equals_sign)
{
    FT_ASSERT_EQ(-1, su_unsetenv("INVALID=NAME"));
    return (1);
}

FT_TEST(test_ft_unsetenv_null_name_sets_errno)
{
    FT_ASSERT_EQ(-1, su_unsetenv(ft_nullptr));
    return (1);
}

FT_TEST(test_ft_unsetenv_success_resets_errno)
{
    const char *variable_name;

    variable_name = "LIBFT_TEST_UNSET_OK";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    FT_ASSERT_EQ(0, su_setenv(variable_name, "value", 1));
    FT_ASSERT_EQ(0, su_unsetenv(variable_name));
    FT_ASSERT_EQ(ft_nullptr, su_getenv(variable_name));
    su_environment_disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_propagates_errno)
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_FAIL";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    FT_ASSERT_EQ(0, su_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, ENOMEM);
    function_result = su_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(-1, function_result);
    su_unsetenv(variable_name);
    su_environment_disable_thread_safety();
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_without_errno)
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_NO_ERRNO";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, su_environment_enable_thread_safety());
    FT_ASSERT_EQ(0, su_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, 0);
    function_result = su_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, function_result);
    su_unsetenv(variable_name);
    su_environment_disable_thread_safety();
    return (1);
}

#if defined(_WIN32) || defined(_WIN64)
FT_TEST(test_ft_unsetenv_failure_uses_return_value)
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_WIN_RETURN";
    FT_ASSERT_EQ(0, su_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(42, 0);
    cmp_set_force_unsetenv_windows_errors(0, 0);
    function_result = su_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(42, function_result);
    su_unsetenv(variable_name);
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_uses_windows_errors)
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_WIN_ERRORS";
    FT_ASSERT_EQ(0, su_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, 0);
    cmp_set_force_unsetenv_windows_errors(123, 456);
    function_result = su_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(-1, function_result);
    su_unsetenv(variable_name);
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_uses_wsa_error)
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_WIN_WSA";
    FT_ASSERT_EQ(0, su_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, 0);
    cmp_set_force_unsetenv_windows_errors(0, 321);
    function_result = su_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(-1, function_result);
    su_unsetenv(variable_name);
    return (1);
}
#endif
