#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include <cerrno>

FT_TEST(test_ft_unsetenv_rejects_empty_name, "ft_unsetenv rejects empty names")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_unsetenv(""));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_ft_getenv_empty_name_sets_errno, "ft_getenv rejects empty names")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_getenv(""));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_ft_getenv_missing_clears_errno, "ft_getenv clears errno when variable absent")
{
    const char *variable_name;

    variable_name = "LIBFT_TEST_GETENV_MISSING";
    ft_unsetenv(variable_name);
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(ft_nullptr, ft_getenv(variable_name));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_setenv_null_value_sets_errno, "ft_setenv null value sets FT_EINVAL")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_setenv("LIBFT_TEST_NULL_VALUE", ft_nullptr, 1));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_ft_unsetenv_rejects_equals_sign, "ft_unsetenv rejects names containing equals")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_unsetenv("INVALID=NAME"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_ft_unsetenv_success_resets_errno, "ft_unsetenv clears ft_errno on success")
{
    const char *variable_name;

    variable_name = "LIBFT_TEST_UNSET_OK";
    FT_ASSERT_EQ(0, ft_setenv(variable_name, "value", 1));
    ft_errno = FT_ETERM;
    FT_ASSERT_EQ(0, ft_unsetenv(variable_name));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_getenv(variable_name));
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_propagates_errno, "ft_unsetenv propagates errno on failure")
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_FAIL";
    FT_ASSERT_EQ(0, ft_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, ENOMEM);
    ft_errno = ER_SUCCESS;
    function_result = ft_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(-1, function_result);
    FT_ASSERT_EQ(ENOMEM + ERRNO_OFFSET, ft_errno);
    ft_unsetenv(variable_name);
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_without_errno, "ft_unsetenv falls back when errno missing")
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_NO_ERRNO";
    FT_ASSERT_EQ(0, ft_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, 0);
    ft_errno = ER_SUCCESS;
    function_result = ft_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(-1, function_result);
    FT_ASSERT_EQ(FT_ETERM, ft_errno);
    ft_unsetenv(variable_name);
    return (1);
}

#if defined(_WIN32) || defined(_WIN64)
FT_TEST(test_ft_unsetenv_failure_uses_return_value, "ft_unsetenv uses return value on Windows")
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_WIN_RETURN";
    FT_ASSERT_EQ(0, ft_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(42, 0);
    cmp_set_force_unsetenv_windows_errors(0, 0);
    ft_errno = ER_SUCCESS;
    function_result = ft_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(42, function_result);
    FT_ASSERT_EQ(42 + ERRNO_OFFSET, ft_errno);
    ft_unsetenv(variable_name);
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_uses_windows_errors, "ft_unsetenv falls back to Windows errors")
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_WIN_ERRORS";
    FT_ASSERT_EQ(0, ft_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, 0);
    cmp_set_force_unsetenv_windows_errors(123, 456);
    ft_errno = ER_SUCCESS;
    function_result = ft_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(-1, function_result);
    FT_ASSERT_EQ(123 + ERRNO_OFFSET, ft_errno);
    ft_unsetenv(variable_name);
    return (1);
}

FT_TEST(test_ft_unsetenv_failure_uses_wsa_error, "ft_unsetenv uses WSA error when needed")
{
    const char *variable_name;
    int function_result;

    variable_name = "LIBFT_TEST_UNSET_WIN_WSA";
    FT_ASSERT_EQ(0, ft_setenv(variable_name, "value", 1));
    cmp_set_force_unsetenv_result(-1, 0);
    cmp_set_force_unsetenv_windows_errors(0, 321);
    ft_errno = ER_SUCCESS;
    function_result = ft_unsetenv(variable_name);
    cmp_clear_force_unsetenv_result();
    FT_ASSERT_EQ(-1, function_result);
    FT_ASSERT_EQ(321 + ERRNO_OFFSET, ft_errno);
    ft_unsetenv(variable_name);
    return (1);
}
#endif

