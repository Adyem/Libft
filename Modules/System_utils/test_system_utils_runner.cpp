#include "test_system_utils_runner.hpp"

#include "../CMA/CMA.hpp"
#include "../Logger/logger.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_lock_tracking.hpp"
#include "../SCMA/SCMA.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#if defined(__linux__) || defined(__APPLE__)
# include <execinfo.h>
# include <signal.h>
#endif
#include "../Errno/errno.hpp"

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

#if defined(__linux__) || defined(__APPLE__)
static int32_t g_test_runner_crash_output_descriptor = STDERR_FILENO;

static const char *test_runner_signal_name(int32_t signal_number)
{
    if (signal_number == SIGABRT)
        return ("SIGABRT");
    if (signal_number == SIGBUS)
        return ("SIGBUS");
    if (signal_number == SIGFPE)
        return ("SIGFPE");
    if (signal_number == SIGILL)
        return ("SIGILL");
    if (signal_number == SIGSEGV)
        return ("SIGSEGV");
    if (signal_number == SIGTERM)
        return ("SIGTERM");
    return ("signal");
}

static int32_t test_runner_crash_output_descriptor(void)
{
    if (g_test_runner_crash_output_descriptor >= 0)
        return (g_test_runner_crash_output_descriptor);
    return (STDERR_FILENO);
}

static void test_runner_write_literal(int32_t file_descriptor,
    const char *message)
{
    ft_size_t message_length;

    if (message == NULL)
        return ;
    message_length = std::strlen(message);
    if (message_length == 0)
        return ;
    (void)write(file_descriptor, message, message_length);
    return ;
}

static void test_runner_crash_signal_handler(int signal_number,
    siginfo_t *signal_information, void *context_pointer)
{
    void *frames[64];
    int32_t frame_count;
    int32_t output_descriptor;
    struct sigaction default_action;

    (void)signal_information;
    (void)context_pointer;
    output_descriptor = test_runner_crash_output_descriptor();
    test_runner_write_literal(output_descriptor,
        "\nlibft tests: crash caught (");
    test_runner_write_literal(output_descriptor,
        test_runner_signal_name(signal_number));
    test_runner_write_literal(output_descriptor, ")\n");
    test_runner_write_literal(output_descriptor,
        "libft tests: stack trace:\n");
    frame_count = backtrace(frames, 64);
    if (frame_count > 0)
        backtrace_symbols_fd(frames, frame_count, output_descriptor);
    else
        test_runner_write_literal(output_descriptor,
            "    <stack trace unavailable>\n");
    std::memset(&default_action, 0, sizeof(default_action));
    default_action.sa_handler = SIG_DFL;
    (void)sigemptyset(&default_action.sa_mask);
    (void)sigaction(signal_number, &default_action, NULL);
    (void)raise(signal_number);
    _exit(128 + signal_number);
}

static void test_runner_enable_crash_stack_traces(void)
{
    struct sigaction action;

    std::memset(&action, 0, sizeof(action));
    action.sa_sigaction = test_runner_crash_signal_handler;
    action.sa_flags = SA_SIGINFO;
    (void)sigemptyset(&action.sa_mask);
    (void)sigaction(SIGABRT, &action, NULL);
    (void)sigaction(SIGBUS, &action, NULL);
    (void)sigaction(SIGFPE, &action, NULL);
    (void)sigaction(SIGILL, &action, NULL);
    (void)sigaction(SIGSEGV, &action, NULL);
    (void)sigaction(SIGTERM, &action, NULL);
# ifdef SIGIOT
    if (SIGIOT != SIGABRT)
        (void)sigaction(SIGIOT, &action, NULL);
# endif
    return ;
}

static void test_runner_set_crash_output_descriptor(int32_t file_descriptor)
{
    g_test_runner_crash_output_descriptor = file_descriptor;
    return ;
}
#endif

static void swap_test_cases(s_test_case *left, s_test_case *right)
{
    s_test_case temp;

    temp = *left;
    *left = *right;
    *right = temp;
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

static int32_t test_is_selected(const s_test_case *test)
{
    const char *name_filter;

    name_filter = get_name_filter();
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
    struct winsize terminal_size;

    if (isatty(STDOUT_FILENO) == 0)
        return (0);
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size) != 0)
        return (80);
    if (terminal_size.ws_col == 0)
        return (80);
    return (static_cast<int32_t>(terminal_size.ws_col));
}

static void print_running_test_line(int32_t test_number,
    const char *description)
{
    int32_t terminal_width;
    int32_t prefix_length;
    int32_t available_description_length;
    int32_t description_index;

    terminal_width = get_stdout_terminal_width();
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
    write_result = write(STDERR_FILENO, message, std::strlen(message));
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
    int32_t show_running_line;
    const char *failure_message;

    log_file = fopen("test_failures.log", "w");
    if (log_file)
        fclose(log_file);
    sort_tests();
    tests = get_tests();
    test_count = get_test_count();
    baseline_stdin_descriptor = dup(STDIN_FILENO);
    baseline_stdout_descriptor = dup(STDOUT_FILENO);
    baseline_stderr_descriptor = dup(STDERR_FILENO);
#if defined(__linux__) || defined(__APPLE__)
    test_runner_set_crash_output_descriptor(baseline_stderr_descriptor);
#endif
    total_tests = *test_count;
    selected_tests = 0;
    hide_successful_tests = hide_successful_tests_enabled();
    index = 0;
    passed = 0;
    while (index < total_tests)
    {
        tests = get_tests();
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
#if defined(__linux__) || defined(__APPLE__)
        test_runner_enable_crash_stack_traces();
#endif
        selected_tests++;
        current_test = tests[index];
        show_running_line = 1;
        if (hide_successful_tests != 0 && isatty(STDOUT_FILENO) == 0)
            show_running_line = 0;
        if (show_running_line != 0)
        {
            print_running_test_line(selected_tests, current_test.description);
        }
        if (execute_test_function(&current_test))
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
