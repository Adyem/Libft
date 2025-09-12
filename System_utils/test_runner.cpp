#include "test_runner.hpp"

#include <vector>
#include <cstdio>

struct s_test_case
{
    t_test_func func;
    const char *description;
};

static std::vector<s_test_case> &get_tests(void)
{
    static std::vector<s_test_case> tests;
    return (tests);
}

int ft_register_test(t_test_func func, const char *description)
{
    s_test_case test_case;
    test_case.func = func;
    test_case.description = description;
    get_tests().push_back(test_case);
    return (0);
}

void ft_test_fail(const char *expression, const char *file, int line)
{
    FILE *log_file;

    log_file = fopen("test_failures.log", "a");
    if (log_file)
    {
        fprintf(log_file, "%s:%d: %s\n", file, line, expression);
        fclose(log_file);
    }
    return ;
}

int ft_run_registered_tests(void)
{
    FILE *log_file;
    int index;
    int passed;

    log_file = fopen("test_failures.log", "w");
    if (log_file)
        fclose(log_file);
    index = 0;
    passed = 0;
    while (index < static_cast<int>(get_tests().size()))
    {
        if (get_tests()[index].func())
        {
            printf("OK %d %s\n", index + 1, get_tests()[index].description);
            passed++;
        }
        else
            printf("KO %d %s\n", index + 1, get_tests()[index].description);
        index++;
    }
    printf("%d/%zu tests passed\n", passed, get_tests().size());
    if (passed != static_cast<int>(get_tests().size()))
        return (1);
    return (0);
}
