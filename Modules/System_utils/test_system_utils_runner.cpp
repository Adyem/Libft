#include "test_system_utils_runner.hpp"

#include "../CMA/CMA.hpp"
#include "../Sink/sink.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_lock_tracking.hpp"
#include "../SCMA/SCMA.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Networking/socket_handle.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <fcntl.h>
#if !defined(_WIN32) && !defined(_WIN64)
# include <sys/ioctl.h>
#endif
#include <unistd.h>
#include "../Basic/limits.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno.hpp"

int32_t cmp_readline_terminal_width(int32_t *width_out);

#ifndef FT_TEST_RUNNER_INITIAL_CAPACITY
# define FT_TEST_RUNNER_INITIAL_CAPACITY 4096
#endif

struct s_test_case
{
    t_test_func func;
    const char *description;
    const char *module;
    const char *name;
};

static int32_t *get_test_count(void)
{
    static int32_t test_count = 0;

    return (&test_count);
}

static int32_t *get_test_capacity(void)
{
    static int32_t test_capacity = 0;

    return (&test_capacity);
}

static s_test_case **get_test_storage(void)
{
    static s_test_case *tests = NULL;

    return (&tests);
}

static s_test_case *get_tests(void)
{
    return (*get_test_storage());
}

static void release_test_storage(void)
{
    s_test_case **tests;

    tests = get_test_storage();
    if (*tests != NULL)
    {
        std::free(*tests);
        *tests = NULL;
    }
    *get_test_count() = 0;
    *get_test_capacity() = 0;
    return ;
}

static int32_t ensure_test_capacity(int32_t required_capacity)
{
    s_test_case **tests;
    int32_t *test_capacity;
    int32_t new_capacity;
    ft_size_t allocation_size;
    void *new_tests;

    if (required_capacity <= 0)
        return (FT_ERR_SUCCESS);
    tests = get_test_storage();
    test_capacity = get_test_capacity();
    if (required_capacity <= *test_capacity)
        return (FT_ERR_SUCCESS);
    new_capacity = *test_capacity;
    if (new_capacity < 1)
        new_capacity = FT_TEST_RUNNER_INITIAL_CAPACITY;
    if (new_capacity < 1)
        new_capacity = 1;
    while (new_capacity < required_capacity)
    {
        if (new_capacity > 1073741823)
        {
            new_capacity = required_capacity;
            break ;
        }
        new_capacity = new_capacity * 2;
    }
    allocation_size = sizeof(s_test_case)
        * static_cast<ft_size_t>(new_capacity);
    new_tests = std::realloc(*tests, allocation_size);
    if (new_tests == NULL)
        return (FT_ERR_NO_MEMORY);
    *tests = static_cast<s_test_case *>(new_tests);
    *test_capacity = new_capacity;
    return (FT_ERR_SUCCESS);
}

static char *get_last_failure_message(void)
{
    static char last_failure_message[2048];

    return (last_failure_message);
}

static void clear_last_failure_message(void)
{
    char *last_failure_message;

    last_failure_message = get_last_failure_message();
    last_failure_message[0] = '\0';
    return ;
}

static void set_last_failure_message(const char *message)
{
    char *last_failure_message;

    if (message == NULL)
        return ;
    last_failure_message = get_last_failure_message();
    (void)std::snprintf(last_failure_message, 2048, "%s", message);
    return ;
}

#ifdef LIBFT_TEST_BUILD
static void report_allocator_leaks(void)
{
    cma_leak_summary cma_summary;
    scma_leak_summary scma_summary;

    if (cma_get_leak_summary(&cma_summary) == FT_ERR_SUCCESS)
    {
        if (cma_summary.live_block_count != 0 || cma_summary.ignored_block_count != 0)
            (void)cma_report_leaks();
    }
    if (scma_get_leak_summary(&scma_summary) == FT_ERR_SUCCESS)
    {
        if (scma_summary.live_block_count != 0 || scma_summary.ignored_block_count != 0)
            (void)scma_report_leaks();
    }
    return ;
}
#endif

static void sort_tests(void)
{
    s_test_case *tests;
    int32_t *test_count;

    tests = get_tests();
    test_count = get_test_count();
    std::sort(tests, tests + *test_count,
        [](const s_test_case &left_test, const s_test_case &right_test) -> int32_t
        {
            return (std::strcmp(left_test.module, right_test.module) < 0);
        });
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

static int32_t filter_token_is_substring(const char *token,
    ft_size_t token_length, const char *name)
{
    ft_size_t name_index;

    if (token == NULL || name == NULL || token_length == 0)
        return (0);
    name_index = 0;
    while (name[name_index] != '\0')
    {
        if (std::strncmp(name + name_index, token, token_length) == 0)
            return (1);
        name_index++;
    }
    return (0);
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
        if (filter_token_is_substring(start, length, name) != 0)
            return (1);
        if (*cursor == ',')
            cursor++;
    }
    return (0);
}

static int32_t test_is_selected(const char *name_filter, const s_test_case *test)
{
    if (name_filter && !name_matches_filter(name_filter, test->name))
        return (0);
    return (1);
}

static int32_t env_value_is_enabled(const char *value)
{
    if (value == NULL)
        return (0);
    if (value[0] == '\0')
        return (0);
    if (std::strcmp(value, "0") == 0)
        return (0);
    if (std::strcmp(value, "false") == 0)
        return (0);
    if (std::strcmp(value, "FALSE") == 0)
        return (0);
    if (std::strcmp(value, "no") == 0)
        return (0);
    if (std::strcmp(value, "NO") == 0)
        return (0);
    if (std::strcmp(value, "off") == 0)
        return (0);
    if (std::strcmp(value, "OFF") == 0)
        return (0);
    return (1);
}

static int32_t hide_successful_tests_enabled(void)
{
    const char *hide_successful;

    hide_successful = getenv("FT_TEST_HIDE_SUCCESSFUL");
    if (env_value_is_enabled(hide_successful) != 0)
        return (1);
    hide_successful = getenv("FT_TEST_HIDE_SUCCESSES");
    if (env_value_is_enabled(hide_successful) != 0)
        return (1);
    return (0);
}

static int32_t get_stdout_terminal_width(void)
{
#if defined(_WIN32) || defined(_WIN64)
    int32_t terminal_width;

    if (cmp_readline_terminal_width(&terminal_width) != FT_ERR_SUCCESS)
        return (80);
    if (terminal_width <= 0)
        return (80);
    return (terminal_width);
#else
    struct winsize terminal_size;

    if (isatty(STDOUT_FILENO) == 0)
        return (0);
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size) != 0)
        return (80);
    if (terminal_size.ws_col == 0)
        return (80);
    return (static_cast<int32_t>(terminal_size.ws_col));
#endif
}

static void print_running_test_line(int32_t test_number,
    const char *description, int32_t terminal_width)
{
    int32_t prefix_length;
    int32_t available_description_length;
    int32_t description_index;

    if (terminal_width <= 0)
        return ;
    printf("\r\033[2KRunning test %d \"", test_number);
    prefix_length = std::snprintf(ft_nullptr, 0, "Running test %d \"",
        test_number);
    available_description_length = terminal_width - prefix_length - 2;
    if (available_description_length < 0)
        available_description_length = 0;
    description_index = 0;
    while (description != ft_nullptr
        && description[description_index] != '\0'
        && description_index < available_description_length)
    {
        putchar(description[description_index]);
        description_index++;
    }
    putchar('"');
    fflush(stdout);
    return ;
}

static void write_literal_to_stderr(const char *message)
{
    ssize_t    write_result;

    if (message == NULL)
        return ;
#if defined(_WIN32) || defined(_WIN64)
    write_result = write(STDERR_FILENO, message,
            static_cast<unsigned int>(std::strlen(message)));
#else
    write_result = write(STDERR_FILENO, message, std::strlen(message));
#endif
    if (write_result < 0)
        return ;
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

static int32_t restore_baseline_descriptors(int32_t baseline_stdin_descriptor,
    int32_t baseline_stdout_descriptor, int32_t baseline_stderr_descriptor)
{
    int32_t restore_ok;

    restore_ok = restore_descriptor_checked(baseline_stdin_descriptor,
        STDIN_FILENO, "runner failed to restore stdin after test");
    if (restore_ok == 0)
        return (0);
    restore_ok = restore_descriptor_checked(baseline_stdout_descriptor,
        STDOUT_FILENO, "runner failed to restore stdout after test");
    if (restore_ok == 0)
        return (0);
    restore_ok = restore_descriptor_checked(baseline_stderr_descriptor,
        STDERR_FILENO, "runner failed to restore stderr after test");
    if (restore_ok == 0)
        return (0);
    return (1);
}

static int32_t execute_test_function(const s_test_case *test,
    int32_t baseline_stdin_descriptor, int32_t baseline_stdout_descriptor,
    int32_t baseline_stderr_descriptor, int32_t null_descriptor)
{
    int32_t reset_error;
    int32_t result;
    if (dup2(null_descriptor, STDOUT_FILENO) < 0)
    {
        report_runner_failure("runner failed to redirect stdout before test");
        return (0);
    }
    if (dup2(null_descriptor, STDERR_FILENO) < 0)
    {
        report_runner_failure("runner failed to redirect stderr before test");
        (void)restore_baseline_descriptors(baseline_stdin_descriptor,
            baseline_stdout_descriptor, baseline_stderr_descriptor);
        return (0);
    }
    reset_error = cma_set_alloc_limit(0);
    if (reset_error != FT_ERR_SUCCESS)
    {
        (void)restore_baseline_descriptors(baseline_stdin_descriptor,
            baseline_stdout_descriptor, baseline_stderr_descriptor);
        report_runner_failure("runner failed to reset CMA alloc limit before test");
        return (0);
    }
    sink_clear();
    reset_mutex_failure_overrides();
    clear_last_failure_message();
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
        set_last_failure_message(exception.what());
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
        set_last_failure_message("unknown exception");
        result = 0;
    }
    reset_error = cma_set_alloc_limit(0);
    if (reset_error != FT_ERR_SUCCESS)
    {
        (void)restore_baseline_descriptors(baseline_stdin_descriptor,
            baseline_stdout_descriptor, baseline_stderr_descriptor);
        report_runner_failure("runner failed to reset CMA alloc limit after test");
        result = 0;
    }
    sink_clear();
    reset_mutex_failure_overrides();
    if (restore_baseline_descriptors(baseline_stdin_descriptor,
            baseline_stdout_descriptor, baseline_stderr_descriptor) == 0)
        result = 0;
    return (result);
}

#ifdef LIBFT_TEST_BUILD
int32_t ft_test_runner_reserve_capacity(int32_t required_capacity)
{
    return (ensure_test_capacity(required_capacity));
}

int32_t ft_test_runner_registered_count(void)
{
    return (*get_test_count());
}

int32_t ft_test_runner_registered_capacity(void)
{
    return (*get_test_capacity());
}
#endif

int32_t ft_register_test(t_test_func func, const char *description, const char *module, const char *name)
{
    s_test_case test_case;
    s_test_case *tests;
    int32_t *test_count;
    int32_t error_code;

    test_case.func = func;
    test_case.description = description;
    test_case.module = module;
    test_case.name = name;
    test_count = get_test_count();
    error_code = ensure_test_capacity(*test_count + 1);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)std::fprintf(stderr,
            "failed to grow test registry while registering %s\n", name);
        return (error_code);
    }
    tests = get_tests();
    tests[*test_count] = test_case;
    *test_count = *test_count + 1;
    return (FT_ERR_SUCCESS);
}

void ft_test_fail(const char *expression, const char *file, int32_t line)
{
    FILE *log_file;
    char failure_message[1024];

    (void)std::snprintf(failure_message, sizeof(failure_message),
        "%s:%d: %s", file, line, expression);
    set_last_failure_message(failure_message);

    log_file = fopen("test_failures.log", "a");
    if (log_file)
    {
        fprintf(log_file, "%s\n", failure_message);
        fclose(log_file);
    }
    return ;
}

void ft_test_fail_values(const char *expression, const char *file, int32_t line,
    const char *expected_value, const char *actual_value)
{
    FILE *log_file;
    char failure_message[2048];

    (void)std::snprintf(failure_message, sizeof(failure_message),
        "%s:%d: %s | expected: %s | actual: %s",
        file, line, expression, expected_value, actual_value);
    set_last_failure_message(failure_message);

    log_file = fopen("test_failures.log", "a");
    if (log_file)
    {
        fprintf(log_file, "%s\n", failure_message);
        fclose(log_file);
    }
    return ;
}

int32_t ft_run_registered_tests(void)
{
    FILE *log_file;
    const char *name_filter;
    int32_t null_descriptor;
    int32_t baseline_stdin_descriptor;
    int32_t baseline_stdout_descriptor;
    int32_t baseline_stderr_descriptor;
    int32_t index;
    int32_t passed;
    s_test_case *tests;
    s_test_case current_test;
    int32_t *test_count;
    int32_t total_tests;
    int32_t selected_tests;
    int32_t hide_successful_tests;
    int32_t terminal_width;
    int32_t show_running_line;
    const char *failure_message;
    ft_bool socket_runtime_acquired;

    log_file = fopen("test_failures.log", "w");
    if (log_file)
        fclose(log_file);
    sort_tests();
    tests = get_tests();
    test_count = get_test_count();
    name_filter = get_name_filter();
    baseline_stdin_descriptor = dup(STDIN_FILENO);
    baseline_stdout_descriptor = dup(STDOUT_FILENO);
    baseline_stderr_descriptor = dup(STDERR_FILENO);
    socket_runtime_acquired = FT_FALSE;
#ifdef _WIN32
    if (ft_socket_runtime_acquire() != FT_ERR_SUCCESS)
    {
        report_runner_failure("runner failed to initialize socket runtime before test loop");
        if (baseline_stdin_descriptor >= 0)
            (void)close(baseline_stdin_descriptor);
        if (baseline_stdout_descriptor >= 0)
            (void)close(baseline_stdout_descriptor);
        if (baseline_stderr_descriptor >= 0)
            (void)close(baseline_stderr_descriptor);
        return (1);
    }
    socket_runtime_acquired = FT_TRUE;
#endif
    null_descriptor = open(cmp_service_null_device_path(), O_WRONLY);
    if (null_descriptor < 0)
    {
        report_runner_failure("runner failed to open null device before test loop");
#ifdef _WIN32
        if (socket_runtime_acquired == FT_TRUE)
            ft_socket_runtime_release();
#endif
        if (baseline_stdin_descriptor >= 0)
            (void)close(baseline_stdin_descriptor);
        if (baseline_stdout_descriptor >= 0)
            (void)close(baseline_stdout_descriptor);
        if (baseline_stderr_descriptor >= 0)
            (void)close(baseline_stderr_descriptor);
        return (1);
    }
    total_tests = *test_count;
    selected_tests = 0;
    hide_successful_tests = hide_successful_tests_enabled();
    terminal_width = get_stdout_terminal_width();
    index = 0;
    passed = 0;
    while (index < total_tests)
    {
        tests = get_tests();
        if (!test_is_selected(name_filter, &tests[index]))
        {
            index++;
            continue ;
        }
        selected_tests++;
        current_test = tests[index];
        show_running_line = 1;
        if (hide_successful_tests != 0 && terminal_width <= 0)
            show_running_line = 0;
        if (show_running_line != 0)
        {
            print_running_test_line(selected_tests, current_test.description,
                terminal_width);
        }
        if (execute_test_function(&current_test, baseline_stdin_descriptor,
                baseline_stdout_descriptor, baseline_stderr_descriptor,
                null_descriptor))
        {
            if (hide_successful_tests != 0)
            {
                if (show_running_line != 0)
                    printf("\r\033[2K");
            }
            else
                printf("\r\033[2K\033[32mSUCCESS\033[0m %d %s\n",
                    selected_tests, current_test.description);
            fflush(stdout);
            passed++;
        }
        else
        {
            if (show_running_line != 0)
                printf("\r\033[2K");
            printf("\033[31mFAIL\033[0m %d %s\n", selected_tests, current_test.description);
            failure_message = get_last_failure_message();
            if (failure_message[0] != '\0')
                printf("    %s\n", failure_message);
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
    if (null_descriptor >= 0)
        (void)close(null_descriptor);
#ifdef _WIN32
    if (socket_runtime_acquired == FT_TRUE)
        ft_socket_runtime_release();
#endif
    release_test_storage();
    if (selected_tests == 0)
    {
        printf("0/0 tests passed\n");
        fflush(stdout);
        return (1);
    }
    printf("%d/%d tests passed\n", passed, selected_tests);
    fflush(stdout);
#ifdef LIBFT_TEST_BUILD
    report_allocator_leaks();
#endif
    if (passed != selected_tests)
        return (1);
    return (0);
}
