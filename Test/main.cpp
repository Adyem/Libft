#include "../Modules/System_utils/test_system_utils_runner.hpp"

#if defined(__linux__) || defined(__APPLE__)
# include <execinfo.h>
# include <signal.h>
# include <unistd.h>
# include <cstring>

static const char *test_crash_signal_name(int32_t signal_number) noexcept
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
#ifdef SIGIOT
    if (signal_number == SIGIOT)
        return ("SIGIOT");
#endif
    return ("signal");
}

static void test_write_literal(const char *message) noexcept
{
    ft_size_t message_length;

    if (message == ft_nullptr)
        return ;
    message_length = std::strlen(message);
    if (message_length == 0)
        return ;
    (void)write(STDERR_FILENO, message, message_length);
    return ;
}

static void test_print_stack_trace(void) noexcept
{
    void *frames[64];
    int32_t frame_count;

    frame_count = backtrace(frames, 64);
    if (frame_count <= 0)
    {
        test_write_literal("    <stack trace unavailable>\n");
        return ;
    }
    backtrace_symbols_fd(frames, frame_count, STDERR_FILENO);
    return ;
}

static void test_crash_signal_handler(int signal_number, siginfo_t *,
    void *)
{
    struct sigaction default_action;

    test_write_literal("\nlibft tests: crash caught (");
    test_write_literal(test_crash_signal_name(signal_number));
    test_write_literal(")\n");
    test_write_literal("libft tests: stack trace:\n");
    test_print_stack_trace();
    std::memset(&default_action, 0, sizeof(default_action));
    default_action.sa_handler = SIG_DFL;
    (void)sigemptyset(&default_action.sa_mask);
    (void)sigaction(signal_number, &default_action, ft_nullptr);
    (void)raise(signal_number);
    _exit(128 + signal_number);
}

static void test_enable_crash_stack_traces(void) noexcept
{
    struct sigaction action;

    std::memset(&action, 0, sizeof(action));
    action.sa_sigaction = test_crash_signal_handler;
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;
    (void)sigemptyset(&action.sa_mask);
    (void)sigaction(SIGABRT, &action, ft_nullptr);
    (void)sigaction(SIGBUS, &action, ft_nullptr);
    (void)sigaction(SIGFPE, &action, ft_nullptr);
    (void)sigaction(SIGILL, &action, ft_nullptr);
    (void)sigaction(SIGSEGV, &action, ft_nullptr);
    (void)sigaction(SIGTERM, &action, ft_nullptr);
#ifdef SIGIOT
    if (SIGIOT != SIGABRT)
        (void)sigaction(SIGIOT, &action, ft_nullptr);
#endif
    return ;
}
#endif

int main(void)
{
#if defined(__linux__) || defined(__APPLE__)
    test_enable_crash_stack_traces();
#endif
    return (ft_run_registered_tests());
}
