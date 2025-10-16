#include "test_runner.hpp"

#include <cstdio>
#include <cstring>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>

static void ft_test_signal_handler(int signal_number)
{
    void *frames[64];
    int frame_count;

    frame_count = backtrace(frames, 64);
    backtrace_symbols_fd(frames, frame_count, STDERR_FILENO);
    signal(signal_number, SIG_DFL);
    raise(signal_number);
    return ;
}

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

int ft_register_test(t_test_func func, const char *description, const char *module)
{
    s_test_case test_case;
    s_test_case *tests;
    int *test_count;

    test_case.func = func;
    test_case.description = description;
    test_case.module = module;
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
    int output_is_terminal;

    log_file = fopen("test_failures.log", "w");
    if (log_file)
        fclose(log_file);
    signal(SIGSEGV, ft_test_signal_handler);
    signal(SIGABRT, ft_test_signal_handler);
    sort_tests();
    tests = get_tests();
    test_count = get_test_count();
    total_tests = *test_count;
    index = 0;
    passed = 0;
    output_is_terminal = isatty(STDOUT_FILENO);
    while (index < total_tests)
    {
        if (output_is_terminal)
            printf("Running test %d \"%s\"", index + 1, tests[index].description);
        else
            printf("Running test %d \"%s\"\n", index + 1, tests[index].description);
        fflush(stdout);
        if (tests[index].func())
        {
            if (output_is_terminal)
                printf("\r\033[K");
            printf("OK %d %s\n", index + 1, tests[index].description);
            fflush(stdout);
            passed++;
        }
        else
        {
            if (output_is_terminal)
                printf("\r\033[K");
            printf("KO %d %s\n", index + 1, tests[index].description);
            fflush(stdout);
        }
        index++;
    }
    printf("%d/%d tests passed\n", passed, total_tests);
    fflush(stdout);
    if (passed != total_tests)
        return (1);
    return (0);
}
