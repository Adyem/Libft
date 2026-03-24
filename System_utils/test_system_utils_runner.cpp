#include "test_system_utils_runner.hpp"

#include "../CMA/CMA.hpp"
#include "../Logger/logger.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_lock_tracking.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <fcntl.h>
#include <unistd.h>
#include "../Errno/errno.hpp"

static int32_t *get_test_count(void)
{
    static int32_t test_count = 0;

    return (&test_count);
}

static int32_t get_test_capacity(void)
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
    int32_t *test_count;
    int32_t outer_index;
    int32_t inner_index;

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

static int32_t name_matches_filter(const char *filter, const char *name)
{
    const char *cursor;
    const char *start;
    ft_size_t length;

    cursor = filter;
    while (*cursor)
    {
        while (*cursor == ' ' || *cursor == ',')
            cursor++;
        start = cursor;
        while (*cursor && *cursor != ',')
            cursor++;
        length = static_cast<ft_size_t>(cursor - start);
        if (length > 0 && std::strncmp(start, name, length) == 0 && name[length] == '\0')
            return (1);
        if (*cursor == ',')
            cursor++;
    }
    return (0);
}

static int32_t test_is_selected(const s_test_case *test)
{
    const char *name_filter;

    name_filter = get_name_filter();
    if (name_filter && !name_matches_filter(name_filter, test->name))
        return (0);
    return (1);
}

static void write_literal_to_stderr(const char *message)
{
    if (message == NULL)
        return ;
    (void)write(STDERR_FILENO, message, std::strlen(message));
    return ;
}

static void reset_mutex_failure_overrides(void)
{
#ifdef LIBFT_TEST_BUILD
    pt_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    pt_lock_tracking_notify_acquired_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    pt_lock_tracking_detect_cycle_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
#endif
    return ;
}

static void report_runner_failure(const char *message)
{
    if (message == NULL)
        return ;
    if (std::fprintf(stderr, "\r\033[2K\033[31mFAIL\033[0m %s\n", message) < 0)
    {
        write_literal_to_stderr("\r\033[2KFAIL ");
        write_literal_to_stderr(message);
        write_literal_to_stderr("\n");
    }
    else
        (void)std::fflush(stderr);
    return ;
}

static int32_t restore_descriptor_checked(int32_t saved_descriptor,
    int32_t target_descriptor, const char *message)
{
    if (saved_descriptor < 0)
        return (1);
    if (dup2(saved_descriptor, target_descriptor) < 0)
    {
        report_runner_failure(message);
        return (0);
    }
    return (1);
}

static int32_t close_descriptor_checked(int32_t descriptor, const char *message)
{
    if (descriptor < 0)
        return (1);
    if (close(descriptor) < 0)
    {
        report_runner_failure(message);
        return (0);
    }
    return (1);
}

static int32_t execute_test_function(const s_test_case *test)
{
    int32_t saved_stdin_descriptor;
    int32_t saved_stdout_descriptor;
    int32_t saved_stderr_descriptor;
    int32_t null_descriptor;
    int32_t restore_ok;
    int32_t reset_error;
    int32_t result;

    saved_stdin_descriptor = dup(STDIN_FILENO);
    if (saved_stdin_descriptor < 0)
    {
        report_runner_failure("runner failed to dup stdin before test");
        return (0);
    }
    saved_stdout_descriptor = dup(STDOUT_FILENO);
    if (saved_stdout_descriptor < 0)
    {
        report_runner_failure("runner failed to dup stdout before test");
        (void)close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after dup stdout failure");
        return (0);
    }
    saved_stderr_descriptor = dup(STDERR_FILENO);
    if (saved_stderr_descriptor < 0)
    {
        report_runner_failure("runner failed to dup stderr before test");
        (void)close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after dup stderr failure");
        (void)close_descriptor_checked(saved_stdout_descriptor,
            "runner failed to close saved stdout after dup stderr failure");
        return (0);
    }
    null_descriptor = open("/dev/null", O_WRONLY);
    if (null_descriptor < 0)
    {
        report_runner_failure("runner failed to open /dev/null before test");
        (void)close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after /dev/null open failure");
        (void)close_descriptor_checked(saved_stdout_descriptor,
            "runner failed to close saved stdout after /dev/null open failure");
        (void)close_descriptor_checked(saved_stderr_descriptor,
            "runner failed to close saved stderr after /dev/null open failure");
        return (0);
    }
    if (dup2(null_descriptor, STDOUT_FILENO) < 0)
    {
        report_runner_failure("runner failed to redirect stdout before test");
        (void)close_descriptor_checked(null_descriptor,
            "runner failed to close /dev/null after stdout redirect failure");
        (void)close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after stdout redirect failure");
        (void)close_descriptor_checked(saved_stdout_descriptor,
            "runner failed to close saved stdout after stdout redirect failure");
        (void)close_descriptor_checked(saved_stderr_descriptor,
            "runner failed to close saved stderr after stdout redirect failure");
        return (0);
    }
    if (dup2(null_descriptor, STDERR_FILENO) < 0)
    {
        report_runner_failure("runner failed to redirect stderr before test");
        (void)close_descriptor_checked(null_descriptor,
            "runner failed to close /dev/null after stderr redirect failure");
        (void)close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after stderr redirect failure");
        (void)close_descriptor_checked(saved_stdout_descriptor,
            "runner failed to close saved stdout after stderr redirect failure");
        (void)close_descriptor_checked(saved_stderr_descriptor,
            "runner failed to close saved stderr after stderr redirect failure");
        return (0);
    }
    if (!close_descriptor_checked(null_descriptor,
            "runner failed to close /dev/null before test"))
    {
        (void)close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after /dev/null close failure");
        (void)close_descriptor_checked(saved_stdout_descriptor,
            "runner failed to close saved stdout after /dev/null close failure");
        (void)close_descriptor_checked(saved_stderr_descriptor,
            "runner failed to close saved stderr after /dev/null close failure");
        return (0);
    }
    reset_error = cma_set_alloc_limit(0);
    if (reset_error != FT_ERR_SUCCESS)
    {
        (void)restore_descriptor_checked(saved_stderr_descriptor, STDERR_FILENO,
            "runner failed to restore stderr after pre-test CMA reset failure");
        report_runner_failure("runner failed to reset CMA alloc limit before test");
        (void)restore_descriptor_checked(saved_stdin_descriptor, STDIN_FILENO,
            "runner failed to restore stdin after pre-test CMA reset failure");
        (void)restore_descriptor_checked(saved_stdout_descriptor, STDOUT_FILENO,
            "runner failed to restore stdout after pre-test CMA reset failure");
        (void)restore_descriptor_checked(saved_stderr_descriptor, STDERR_FILENO,
            "runner failed to restore stderr after pre-test CMA reset failure");
        (void)close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after pre-test CMA reset failure");
        (void)close_descriptor_checked(saved_stdout_descriptor,
            "runner failed to close saved stdout after pre-test CMA reset failure");
        (void)close_descriptor_checked(saved_stderr_descriptor,
            "runner failed to close saved stderr after pre-test CMA reset failure");
        return (0);
    }
    ft_log_close();
    reset_mutex_failure_overrides();
    try
    {
        result = test->func();
    }
    catch (const std::exception &exception)
    {
        FILE *log_file;

        log_file = fopen("test_failures.log", "a");
        if (log_file)
        {
            fprintf(log_file, "Exception in %s/%s: %s\n", test->module,
                test->name, exception.what());
            fclose(log_file);
        }
        result = 0;
    }
    catch (...)
    {
        FILE *log_file;

        log_file = fopen("test_failures.log", "a");
        if (log_file)
        {
            fprintf(log_file, "Unknown exception in %s/%s\n", test->module,
                test->name);
            fclose(log_file);
        }
        result = 0;
    }
    reset_error = cma_set_alloc_limit(0);
    if (reset_error != FT_ERR_SUCCESS)
    {
        (void)restore_descriptor_checked(saved_stderr_descriptor, STDERR_FILENO,
            "runner failed to restore stderr after post-test CMA reset failure");
        report_runner_failure("runner failed to reset CMA alloc limit after test");
        result = 0;
    }
    ft_log_close();
    reset_mutex_failure_overrides();
    restore_ok = restore_descriptor_checked(saved_stdin_descriptor, STDIN_FILENO,
        "runner failed to restore stdin after test");
    if (restore_ok == 0)
        result = 0;
    restore_ok = restore_descriptor_checked(saved_stdout_descriptor, STDOUT_FILENO,
        "runner failed to restore stdout after test");
    if (restore_ok == 0)
        result = 0;
    restore_ok = restore_descriptor_checked(saved_stderr_descriptor, STDERR_FILENO,
        "runner failed to restore stderr after test");
    if (restore_ok == 0)
        result = 0;
    if (!close_descriptor_checked(saved_stdin_descriptor,
            "runner failed to close saved stdin after test"))
        result = 0;
    if (!close_descriptor_checked(saved_stdout_descriptor,
            "runner failed to close saved stdout after test"))
        result = 0;
    if (!close_descriptor_checked(saved_stderr_descriptor,
            "runner failed to close saved stderr after test"))
        result = 0;
    return (result);
}

int32_t ft_register_test(t_test_func func, const char *description, const char *module, const char *name)
{
    s_test_case test_case;
    s_test_case *tests;
    int32_t *test_count;

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

void ft_test_fail(const char *expression, const char *file, int32_t line)
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

void ft_test_fail_values(const char *expression, const char *file, int32_t line,
    const char *expected_value, const char *actual_value)
{
    FILE *log_file;

    log_file = fopen("test_failures.log", "a");
    if (log_file)
    {
        fprintf(log_file, "%s:%d: %s | expected: %s | actual: %s\n",
            file, line, expression, expected_value, actual_value);
        fclose(log_file);
    }
    return ;
}

int32_t ft_run_registered_tests(void)
{
    FILE *log_file;
    int32_t baseline_stdin_descriptor;
    int32_t baseline_stdout_descriptor;
    int32_t baseline_stderr_descriptor;
    int32_t index;
    int32_t passed;
    s_test_case *tests;
    int32_t *test_count;
    int32_t total_tests;
    int32_t selected_tests;

    log_file = fopen("test_failures.log", "w");
    if (log_file)
        fclose(log_file);
    sort_tests();
    tests = get_tests();
    test_count = get_test_count();
    baseline_stdin_descriptor = dup(STDIN_FILENO);
    baseline_stdout_descriptor = dup(STDOUT_FILENO);
    baseline_stderr_descriptor = dup(STDERR_FILENO);
    total_tests = *test_count;
    selected_tests = 0;
    index = 0;
    passed = 0;
    while (index < total_tests)
    {
        if (!test_is_selected(&tests[index]))
        {
            index++;
            continue ;
        }
        if (baseline_stdin_descriptor >= 0)
            (void)dup2(baseline_stdin_descriptor, STDIN_FILENO);
        if (baseline_stdout_descriptor >= 0)
            (void)dup2(baseline_stdout_descriptor, STDOUT_FILENO);
        if (baseline_stderr_descriptor >= 0)
            (void)dup2(baseline_stderr_descriptor, STDERR_FILENO);
        selected_tests++;
        printf("Running test %d \"%s\"", selected_tests,
            tests[index].description);
        fflush(stdout);
        if (execute_test_function(&tests[index]))
        {
            printf("\r\033[2K\033[32mSUCCESS\033[0m %d %s\n", selected_tests, tests[index].description);
            fflush(stdout);
            passed++;
        }
        else
        {
            printf("\r\033[2K\033[31mFAIL\033[0m %d %s\n", selected_tests, tests[index].description);
            fflush(stdout);
        }
        index++;
    }
    if (baseline_stdin_descriptor >= 0)
        (void)close(baseline_stdin_descriptor);
    if (baseline_stdout_descriptor >= 0)
        (void)close(baseline_stdout_descriptor);
    if (baseline_stderr_descriptor >= 0)
        (void)close(baseline_stderr_descriptor);
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
