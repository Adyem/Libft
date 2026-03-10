#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <cstddef>
#include <cmath>
#include <cstring>

#include "../Errno/errno.hpp"

typedef int32_t (*t_test_func)(void);
#ifndef TEST_MODULE
#define TEST_MODULE "Libft"
#endif

int32_t ft_register_test(t_test_func func, const char *description, const char *module, const char *name);
void ft_test_fail(const char *expression, const char *file, int32_t line);
int32_t ft_run_registered_tests(void);

#define FT_TEST(name) \
    static int32_t name(void); \
    static void register_##name(void) __attribute__((constructor)); \
    static void register_##name(void) \
    { \
        ft_register_test(name, #name, TEST_MODULE, #name); \
        return ; \
    } \
    static int32_t name(void)

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
        expression; \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, count) \
    do \
    { \
        (stream).read((buffer), (count)); \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_BAD(result_variable, stream) \
    do \
    { \
        result_variable = !(stream).is_valid(); \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_GCOUNT(result_variable, stream) \
    do \
    { \
        result_variable = (stream).gcount(); \
    } while (0)

#define FT_ASSERT_SINGLE_ERROR_STR(result_variable, stream) \
    do \
    { \
        result_variable = (stream).str(); \
    } while (0)

#define FT_ASSERT_STREAM_BAD_FALSE(stream) \
    do \
    { \
        ft_bool ft_stream_bad_result__ = FT_FALSE; \
        ft_stream_bad_result__ = !(stream).is_valid(); \
        FT_ASSERT_EQ(FT_FALSE, ft_stream_bad_result__); \
    } while (0)

#endif
