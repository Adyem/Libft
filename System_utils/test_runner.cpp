#include "test_runner.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <unistd.h>
#include "../Errno/errno.hpp"

static int *get_test_count(void)
{
    static int test_count = 0;

    return (&test_count);
}

static int get_test_capacity(void)
{
    return (4096);
}

struct s_test_case
{
    t_test_func func;
    const char *description;
    const char *module;
    const char *name;
};

static s_test_case *get_tests(void)
{
    static s_test_case tests[4096];

    return (tests);
}

static void swap_test_cases(s_test_case *left, s_test_case *right)
{
    s_test_case temp;

    temp = *left;
    *left = *right;
    *right = temp;
    return ;
}

static void sort_tests(void)
{
    s_test_case *tests;
    int *test_count;
    int outer_index;
    int inner_index;

    tests = get_tests();
    test_count = get_test_count();
    outer_index = 0;
    while (outer_index < *test_count)
    {
        inner_index = outer_index + 1;
        while (inner_index < *test_count)
        {
            if (std::strcmp(tests[inner_index].module, tests[outer_index].module) < 0)
                swap_test_cases(&tests[outer_index], &tests[inner_index]);
            inner_index++;
        }
        outer_index++;
    }
    return ;
}

static const char *get_name_filter(void)
{
    const char *filter;

    filter = getenv("FT_TEST_NAME_FILTER");
    if (!filter)
        return (NULL);
    if (!filter[0])
        return (NULL);
    return (filter);
}

static int name_matches_filter(const char *filter, const char *name)
{
    const char *cursor;
    const char *start;
    size_t length;

    cursor = filter;
    while (*cursor)
    {
        while (*cursor == ' ' || *cursor == ',')
            cursor++;
        start = cursor;
        while (*cursor && *cursor != ',')
            cursor++;
        length = static_cast<size_t>(cursor - start);
        if (length > 0 && std::strncmp(start, name, length) == 0 && name[length] == '\0')
            return (1);
        if (*cursor == ',')
            cursor++;
    }
    return (0);
}

static int test_is_selected(const s_test_case *test)
{
    const char *name_filter;

    name_filter = get_name_filter();
    if (name_filter && !name_matches_filter(name_filter, test->name))
        return (0);
    return (1);
}

static int execute_test_function(const s_test_case *test)
{
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    try
    {
        result = test->func();
    }
    catch (const std::exception &exception)
    {
        FILE *log_file;

        ft_errno = FT_ERR_INTERNAL;
        log_file = fopen("test_failures.log", "a");
        if (log_file)
        {
            fprintf(log_file, "Exception in %s/%s: %s\n", test->module,
                test->name, exception.what());
            fclose(log_file);
        }
        return (0);
    }
    catch (...)
    {
        FILE *log_file;

        ft_errno = FT_ERR_INTERNAL;
        log_file = fopen("test_failures.log", "a");
        if (log_file)
        {
            fprintf(log_file, "Unknown exception in %s/%s\n", test->module,
                test->name);
            fclose(log_file);
        }
        return (0);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

int ft_register_test(t_test_func func, const char *description, const char *module, const char *name)
{
    s_test_case test_case;
    s_test_case *tests;
    int *test_count;

    test_case.func = func;
    test_case.description = description;
    test_case.module = module;
    test_case.name = name;
    tests = get_tests();
    test_count = get_test_count();
    if (*test_count >= get_test_capacity())
        return (1);
    tests[*test_count] = test_case;
    *test_count = *test_count + 1;
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
    s_test_case *tests;
    int *test_count;
    int total_tests;
    int selected_tests;
    int output_is_terminal;

    log_file = fopen("test_failures.log", "w");
    if (log_file)
        fclose(log_file);
    sort_tests();
    tests = get_tests();
    test_count = get_test_count();
    total_tests = *test_count;
    selected_tests = 0;
    index = 0;
    passed = 0;
    output_is_terminal = isatty(STDOUT_FILENO);
    while (index < total_tests)
    {
        if (!test_is_selected(&tests[index]))
        {
            index++;
            continue;
        }
        selected_tests++;
        if (output_is_terminal)
            printf("Running test %d \"%s\"", selected_tests, tests[index].description);
        else
            printf("Running test %d \"%s\"\n", selected_tests, tests[index].description);
        fflush(stdout);
        if (execute_test_function(&tests[index]))
        {
            if (output_is_terminal)
                printf("\r\033[K");
            printf("OK %d %s\n", selected_tests, tests[index].description);
            fflush(stdout);
            passed++;
        }
        else
        {
            if (output_is_terminal)
                printf("\r\033[K");
            printf("KO %d %s\n", selected_tests, tests[index].description);
            fflush(stdout);
        }
        index++;
    }
    if (selected_tests == 0)
    {
        printf("0/0 tests passed\n");
        fflush(stdout);
        return (1);
    }
    printf("%d/%d tests passed\n", passed, selected_tests);
    fflush(stdout);
    if (passed != selected_tests)
        return (1);
    return (0);
}
