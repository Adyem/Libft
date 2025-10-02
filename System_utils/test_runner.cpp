#include "test_runner.hpp"

#include <vector>
#include <cstdio>
#include <algorithm>
#include <cstring>

struct s_test_case
{
    t_test_func func;
    const char *description;
    const char *module;
};

static std::vector<s_test_case> &get_tests(void)
{
    static std::vector<s_test_case> tests;
    return (tests);
}

static bool compare_tests(const s_test_case &left, const s_test_case &right)
{
    if (std::strcmp(left.module, right.module) < 0)
        return (1);
    return (0);
}

int ft_register_test(t_test_func func, const char *description, const char *module)
{
    s_test_case test_case;
    test_case.func = func;
    test_case.description = description;
    test_case.module = module;
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
    const char *current_module;

    log_file = fopen("test_failures.log", "w");
    if (log_file)
        fclose(log_file);
    std::sort(get_tests().begin(), get_tests().end(), compare_tests);
    index = 0;
    passed = 0;
    current_module = NULL;
    while (index < static_cast<int>(get_tests().size()))
    {
        if (!current_module || std::strcmp(current_module, get_tests()[index].module) != 0)
        {
            current_module = get_tests()[index].module;
            printf("== %s ==\n", current_module);
            fflush(stdout);
        }
        fflush(stdout);
        if (get_tests()[index].func())
        {
            printf("OK %d %s\n", index + 1, get_tests()[index].description);
            fflush(stdout);
            passed++;
        }
        else
        {
            printf("KO %d %s\n", index + 1, get_tests()[index].description);
            fflush(stdout);
        }
        index++;
    }
    printf("%d/%zu tests passed\n", passed, get_tests().size());
    fflush(stdout);
    if (passed != static_cast<int>(get_tests().size()))
        return (1);
    return (0);
}
