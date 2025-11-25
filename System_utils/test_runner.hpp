#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <cstddef>
#include <cmath>

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

#define FT_ASSERT_NE(unexpected, actual) \
    do \
    { \
        if ((unexpected) == (actual)) \
        { \
            ft_test_fail(#unexpected " != " #actual, __FILE__, __LINE__); \
            return (0); \
        } \
    } while (0)

#endif
