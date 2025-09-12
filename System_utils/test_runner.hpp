#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <cstddef>

typedef int (*t_test_func)(void);

int ft_register_test(t_test_func func, const char *description);
void ft_test_fail(const char *expression, const char *file, int line);
int ft_run_registered_tests(void);

#define FT_TEST(name, description) \
    static int name(void); \
    static void register_##name(void) __attribute__((constructor)); \
    static void register_##name(void) \
    { \
        ft_register_test(name, description); \
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

#endif
