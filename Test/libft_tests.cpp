#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"
#include "../CPP_class/nullptr.hpp"

extern "C" int main(void);

struct s_test
{
    int (*func)(void);
    const char *description;
};

static int test_strlen_nullptr(void)
{
    if (ft_strlen(ft_nullptr) == 0)
        return (1);
    return (0);
}

static int test_strlen_simple(void)
{
    if (ft_strlen("test") == 4)
        return (1);
    return (0);
}

static int test_strcmp_equal(void)
{
    if (ft_strcmp("abc", "abc") == 0)
        return (1);
    return (0);
}

static int test_strcmp_null(void)
{
    if (ft_strcmp(ft_nullptr, "abc") == -1)
        return (1);
    return (0);
}

static int test_isdigit_true(void)
{
    if (ft_isdigit('5') == 1)
        return (1);
    return (0);
}

static int test_isdigit_false(void)
{
    if (ft_isdigit('a') == 0)
        return (1);
    return (0);
}

static int test_memset_null(void)
{
    if (ft_memset(ft_nullptr, 'A', 3) == ft_nullptr)
        return (1);
    return (0);
}

static int test_memset_basic(void)
{
    char buf[4];
    ft_memset(buf, 'x', 3);
    buf[3] = '\0';
    if (ft_strcmp(buf, "xxx") == 0)
        return (1);
    return (0);
}

static int test_toupper_basic(void)
{
    char str[4] = "abc";
    ft_to_upper(str);
    if (ft_strcmp(str, "ABC") == 0)
        return (1);
    return (0);
}

static int test_atoi_simple(void)
{
    if (ft_atoi("42") == 42)
        return (1);
    return (0);
}

static int test_atoi_negative(void)
{
    if (ft_atoi("-13") == -13)
        return (1);
    return (0);
}

static void run_test(int index, const s_test *test, int *passed)
{
    if (test->func())
    {
        pf_printf("OK %d %s\n", index, test->description);
        (*passed)++;
        return ;
    }
    pf_printf("KO %d %s\n", index, test->description);
    return ;
}

int main(void)
{
    const s_test tests[] = {
        { test_strlen_nullptr, "strlen nullptr" },
        { test_strlen_simple, "strlen simple" },
        { test_strcmp_equal, "strcmp equal" },
        { test_strcmp_null, "strcmp null" },
        { test_isdigit_true, "isdigit true" },
        { test_isdigit_false, "isdigit false" },
        { test_memset_null, "memset null" },
        { test_memset_basic, "memset basic" },
        { test_toupper_basic, "toupper basic" },
        { test_atoi_simple, "atoi simple" },
        { test_atoi_negative, "atoi negative" }
    };
    const int total = sizeof(tests) / sizeof(tests[0]);
    int index = 0;
    int passed = 0;

    while (index < total)
    {
        run_test(index + 1, &tests[index], &passed);
        index++;
    }
    pf_printf("%d/%d tests passed\n", passed, total);
    return (0);
}

