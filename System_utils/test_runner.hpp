#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <cstddef>
#include <cmath>
#include <cstring>

#include "../Errno/errno.hpp"

typedef int (*t_test_func)(void);
#ifndef TEST_MODULE
#define TEST_MODULE "Libft"
#endif

int ft_register_test(t_test_func func, const char *description, const char *module, const char *name);
void ft_test_fail(const char *expression, const char *file, int line);
int ft_run_registered_tests(void);

#define FT_TEST(name, description) \
    static int name(void); \
    static void register_##name(void) __attribute__((constructor)); \
    static void register_##name(void) \
    { \
        ft_register_test(name, description, TEST_MODULE, #name); \
        return ; \
    } \
    static int name(void)

#define FT_ASSERT(expression) \
    do \
    { \
        if (!(expression)) \
        { \
            ft_test_fail(#expression, __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_EQ(expected, actual) \
    do \
    { \
        if ((expected) != (actual)) \
        { \
            ft_test_fail(#expected " == " #actual, __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_DOUBLE_EQ(expected, actual) \
    do \
    { \
        double ft_expected_value = (expected); \
        double ft_actual_value = (actual); \
        if (std::fabs(ft_expected_value - ft_actual_value) > 0.0000001) \
        { \
            ft_test_fail(#expected " ~= " #actual, __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_NEQ(expected, actual) \
    do \
    { \
        if ((expected) == (actual)) \
        { \
            ft_test_fail(#expected " != " #actual, __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_NE(unexpected, actual) \
    do \
    { \
        if ((unexpected) == (actual)) \
        { \
            ft_test_fail(#unexpected " != " #actual, __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_STR_EQ(expected, actual) \
    do \
    { \
        const char *ft_expected_value = (expected); \
        const char *ft_actual_value = (actual); \
        if (std::strcmp(ft_expected_value, ft_actual_value) != 0) \
        { \
        ft_test_fail(#expected " == " #actual, __FILE__, __LINE__); \
        return (0); \
    } \
    } while (0)

#define FT_ASSERT_SINGLE_GLOBAL_ERROR(expression) \
    do \
    { \
        ft_size_t ft_error_stack_depth_before = ft_global_error_stack_depth(); \
        expression; \
        ft_size_t ft_error_stack_depth_after = ft_global_error_stack_depth(); \
        if (ft_error_stack_depth_after != ft_error_stack_depth_before + 1) \
        { \
            ft_test_fail("FT_ASSERT_SINGLE_GLOBAL_ERROR failed", __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, count) \
    do \
    { \
        ft_size_t ft_error_stack_depth_before = ft_global_error_stack_depth(); \
        (stream).read((buffer), (count)); \
        ft_size_t ft_error_stack_depth_after = ft_global_error_stack_depth(); \
        if (ft_error_stack_depth_after != ft_error_stack_depth_before + 1) \
        { \
            ft_test_fail("FT_ASSERT_SINGLE_ERROR_READ failed", __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_BAD(result_variable, stream) \
    do \
    { \
        ft_size_t ft_error_stack_depth_before = ft_global_error_stack_depth(); \
        result_variable = (stream).bad(); \
        ft_size_t ft_error_stack_depth_after = ft_global_error_stack_depth(); \
        if (ft_error_stack_depth_after != ft_error_stack_depth_before + 1) \
        { \
            ft_test_fail("FT_ASSERT_SINGLE_ERROR_BAD failed", __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_GCOUNT(result_variable, stream) \
    do \
    { \
        ft_size_t ft_error_stack_depth_before = ft_global_error_stack_depth(); \
        result_variable = (stream).gcount(); \
        ft_size_t ft_error_stack_depth_after = ft_global_error_stack_depth(); \
        if (ft_error_stack_depth_after != ft_error_stack_depth_before + 1) \
        { \
            ft_test_fail("FT_ASSERT_SINGLE_ERROR_GCOUNT failed", __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_STR(result_variable, stream) \
    do \
    { \
        ft_size_t ft_error_stack_depth_before = ft_global_error_stack_depth(); \
        result_variable = (stream).str(); \
        ft_size_t ft_error_stack_depth_after = ft_global_error_stack_depth(); \
        if (ft_error_stack_depth_after != ft_error_stack_depth_before + 1) \
        { \
            ft_test_fail("FT_ASSERT_SINGLE_ERROR_STR failed", __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#define FT_ASSERT_STREAM_BAD_FALSE(stream) \
    do \
    { \
        bool ft_stream_bad_result__ = false; \
        ft_size_t ft_error_stack_depth_before__ = ft_global_error_stack_depth(); \
        ft_stream_bad_result__ = (stream).bad(); \
        ft_size_t ft_error_stack_depth_after__ = ft_global_error_stack_depth(); \
        if (ft_error_stack_depth_after__ != ft_error_stack_depth_before__ + 1) \
        { \
            ft_test_fail("FT_ASSERT_STREAM_BAD_FALSE failed", __FILE__, __LINE__); \
            return (0); \
        } \
        FT_ASSERT_EQ(false, ft_stream_bad_result__); \
    } while (0)

#endif
