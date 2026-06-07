#include "debug.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../Compatebility/compatebility_stack_trace.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>

#if defined(__linux__) || defined(__APPLE__)
# include <execinfo.h>
# include <signal.h>
# include <unistd.h>
#endif

#define DBG_SIGNAL_STACK_SIZE 65536

static int32_t g_dbg_error = FT_ERR_SUCCESS;

#if defined(__linux__) || defined(__APPLE__)
static uint8_t g_dbg_signal_stack[DBG_SIGNAL_STACK_SIZE];
static stack_t g_dbg_previous_signal_stack;
static ft_bool g_dbg_has_previous_signal_stack = FT_FALSE;
static ft_bool g_dbg_enabled = FT_FALSE;
static void *g_dbg_frames[DBG_STACK_TRACE_MAX_FRAMES];
static struct sigaction g_dbg_previous_sigabrt;
static struct sigaction g_dbg_previous_sigbus;
static struct sigaction g_dbg_previous_sigfpe;
static struct sigaction g_dbg_previous_sigill;
static struct sigaction g_dbg_previous_sigsegv;
static struct sigaction g_dbg_previous_sigterm;
static ft_bool g_dbg_has_sigabrt = FT_FALSE;
static ft_bool g_dbg_has_sigbus = FT_FALSE;
static ft_bool g_dbg_has_sigfpe = FT_FALSE;
static ft_bool g_dbg_has_sigill = FT_FALSE;
static ft_bool g_dbg_has_sigsegv = FT_FALSE;
static ft_bool g_dbg_has_sigterm = FT_FALSE;
#else
static ft_bool g_dbg_enabled = FT_FALSE;
#endif

static int32_t dbg_set_error(int32_t error_code) noexcept
{
    g_dbg_error = error_code;
    return (error_code);
}

int32_t dbg_get_error(void) noexcept
{
    return (g_dbg_error);
}

const char *dbg_get_error_str(void) noexcept
{
    return (ft_strerror(g_dbg_error));
}

#if defined(__linux__) || defined(__APPLE__)
static void dbg_write_literal(int32_t file_descriptor,
    const char *message) noexcept
{
    ft_size_t message_length;

    if (message == ft_nullptr)
        return ;
    message_length = ft_strlen(message);
    if (message_length == 0)
        return ;
    (void)write(file_descriptor, message, message_length);
    return ;
}

static void dbg_write_signal_number(int32_t file_descriptor,
    int32_t signal_number) noexcept
{
    char buffer[16];
    ft_size_t digit_count;
    int32_t value;

    digit_count = 0;
    value = signal_number;
    if (value == 0)
    {
        buffer[digit_count] = '0';
        digit_count += 1;
    }
    while (value > 0 && digit_count < sizeof(buffer))
    {
        buffer[digit_count] = static_cast<char>('0' + (value % 10));
        value = value / 10;
        digit_count += 1;
    }
    while (digit_count > 0)
    {
        digit_count -= 1;
        (void)write(file_descriptor, &buffer[digit_count], 1);
    }
    return ;
}

static const char *dbg_signal_name(int32_t signal_number) noexcept
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

static void dbg_write_frame_index(int32_t file_descriptor,
    ft_size_t frame_index) noexcept
{
    char digit_buffer[32];
    ft_size_t digit_count;

    digit_count = 0;
    if (frame_index == 0)
    {
        digit_buffer[digit_count] = '0';
        digit_count += 1;
    }
    while (frame_index > 0 && digit_count < sizeof(digit_buffer))
    {
        digit_buffer[digit_count] = static_cast<char>('0' + (frame_index % 10));
        frame_index = frame_index / 10;
        digit_count += 1;
    }
    dbg_write_literal(file_descriptor, "    #");
    while (digit_count > 0)
    {
        digit_count -= 1;
        (void)write(file_descriptor, &digit_buffer[digit_count], 1);
    }
    (void)write(file_descriptor, " ", 1);
    return ;
}

static void dbg_write_symbolized_frame(int32_t file_descriptor, void *frame,
    ft_size_t frame_index) noexcept
{
    char symbol_buffer[1024];
    char location_buffer[1024];

    if (cmp_stack_trace_symbolize_address(frame, symbol_buffer,
            sizeof(symbol_buffer), location_buffer, sizeof(location_buffer))
        != FT_ERR_SUCCESS)
    {
        backtrace_symbols_fd(&frame, 1, file_descriptor);
        return ;
    }
    dbg_write_frame_index(file_descriptor, frame_index);
    dbg_write_literal(file_descriptor, symbol_buffer);
    dbg_write_literal(file_descriptor, "\n       ");
    dbg_write_literal(file_descriptor, location_buffer);
    dbg_write_literal(file_descriptor, "\n");
    return ;
}

static void dbg_write_symbolized_frames(int32_t file_descriptor,
    ft_size_t frame_count, ft_size_t skip_count) noexcept
{
    ft_size_t frame_index;
    ft_size_t output_index;

    frame_index = skip_count;
    output_index = 0;
    while (frame_index < frame_count)
    {
        dbg_write_symbolized_frame(file_descriptor, g_dbg_frames[frame_index],
            output_index);
        frame_index += 1;
        output_index += 1;
    }
    return ;
}
#endif

static void dbg_print_stack_trace_to_fd(int32_t file_descriptor,
    ft_size_t skip_count) noexcept
{
    int32_t frame_count;
    int32_t frame_capacity;

    frame_capacity = DBG_STACK_TRACE_MAX_FRAMES;
    frame_count = backtrace(g_dbg_frames, frame_capacity);
    if (frame_count <= 0)
    {
        dbg_write_literal(file_descriptor, "    <stack trace unavailable>\n");
        return ;
    }
#if defined(__linux__)
    if (skip_count < static_cast<ft_size_t>(frame_count))
    {
        dbg_write_symbolized_frames(file_descriptor,
            static_cast<ft_size_t>(frame_count), skip_count);
        return ;
    }
#endif
    if (skip_count > 0 && skip_count < static_cast<ft_size_t>(frame_count))
    {
        backtrace_symbols_fd(g_dbg_frames + skip_count,
            frame_count - static_cast<int32_t>(skip_count), file_descriptor);
        return ;
    }
    backtrace_symbols_fd(g_dbg_frames, frame_count, file_descriptor);
    return ;
}

static void dbg_crash_signal_handler(int signal_number,
    siginfo_t *signal_information, void *context_pointer)
{
    struct sigaction default_action;

    (void)signal_information;
    (void)context_pointer;
    dbg_write_literal(STDERR_FILENO, "\nlibft debug: crash caught (");
    dbg_write_literal(STDERR_FILENO, dbg_signal_name(signal_number));
    dbg_write_literal(STDERR_FILENO, " ");
    dbg_write_signal_number(STDERR_FILENO, signal_number);
    dbg_write_literal(STDERR_FILENO, ")\n");
    dbg_write_literal(STDERR_FILENO, "libft debug: stack trace:\n");
    dbg_print_stack_trace_to_fd(STDERR_FILENO, 2);
    std::memset(&default_action, 0, sizeof(default_action));
    default_action.sa_handler = SIG_DFL;
    (void)sigemptyset(&default_action.sa_mask);
    (void)sigaction(signal_number, &default_action, ft_nullptr);
    (void)raise(signal_number);
    _exit(128 + signal_number);
}

static int32_t dbg_install_signal_handler(int signal_number,
    struct sigaction *old_action, ft_bool *installed) noexcept
{
    struct sigaction action;

    std::memset(&action, 0, sizeof(action));
    action.sa_sigaction = dbg_crash_signal_handler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND;
    if (sigemptyset(&action.sa_mask) != 0)
        return (dbg_set_error(FT_ERR_SYSTEM));
    if (sigaction(signal_number, &action, old_action) != 0)
        return (dbg_set_error(FT_ERR_SYSTEM));
    *installed = FT_TRUE;
    return (dbg_set_error(FT_ERR_SUCCESS));
}

static void dbg_restore_signal_handler(int signal_number,
    const struct sigaction *old_action, ft_bool *installed) noexcept
{
    if (*installed == FT_FALSE)
        return ;
    (void)sigaction(signal_number, old_action, ft_nullptr);
    *installed = FT_FALSE;
    return ;
}

static int32_t dbg_prepare_signal_stack(void) noexcept
{
    stack_t signal_stack;

    std::memset(&signal_stack, 0, sizeof(signal_stack));
    signal_stack.ss_sp = g_dbg_signal_stack;
    signal_stack.ss_size = sizeof(g_dbg_signal_stack);
    signal_stack.ss_flags = 0;
    if (sigaltstack(&signal_stack, &g_dbg_previous_signal_stack) != 0)
        return (dbg_set_error(FT_ERR_SYSTEM));
    g_dbg_has_previous_signal_stack = FT_TRUE;
    return (dbg_set_error(FT_ERR_SUCCESS));
}

static void dbg_restore_signal_stack(void) noexcept
{
    if (g_dbg_has_previous_signal_stack == FT_FALSE)
        return ;
    (void)sigaltstack(&g_dbg_previous_signal_stack, ft_nullptr);
    g_dbg_has_previous_signal_stack = FT_FALSE;
    return ;
}

static void dbg_warm_stack_trace_backend(void) noexcept
{
    void *frames[1];

    (void)backtrace(frames, 1);
    return ;
}

int32_t dbg_enable_crash_stack_traces(void) noexcept
{
#if defined(__linux__) || defined(__APPLE__)
    int32_t result;

    if (g_dbg_enabled == FT_TRUE)
        return (dbg_set_error(FT_ERR_SUCCESS));
    dbg_warm_stack_trace_backend();
    result = dbg_prepare_signal_stack();
    if (result != FT_ERR_SUCCESS)
        return (result);
    result = dbg_install_signal_handler(SIGABRT, &g_dbg_previous_sigabrt,
            &g_dbg_has_sigabrt);
    if (result == FT_ERR_SUCCESS)
        result = dbg_install_signal_handler(SIGBUS, &g_dbg_previous_sigbus,
                &g_dbg_has_sigbus);
    if (result == FT_ERR_SUCCESS)
        result = dbg_install_signal_handler(SIGFPE, &g_dbg_previous_sigfpe,
                &g_dbg_has_sigfpe);
    if (result == FT_ERR_SUCCESS)
        result = dbg_install_signal_handler(SIGILL, &g_dbg_previous_sigill,
                &g_dbg_has_sigill);
    if (result == FT_ERR_SUCCESS)
        result = dbg_install_signal_handler(SIGSEGV, &g_dbg_previous_sigsegv,
                &g_dbg_has_sigsegv);
    if (result == FT_ERR_SUCCESS)
        result = dbg_install_signal_handler(SIGTERM, &g_dbg_previous_sigterm,
                &g_dbg_has_sigterm);
    if (result != FT_ERR_SUCCESS)
    {
        dbg_disable_crash_stack_traces();
        return (result);
    }
    g_dbg_enabled = FT_TRUE;
    return (dbg_set_error(FT_ERR_SUCCESS));
#else
    g_dbg_enabled = FT_FALSE;
    return (dbg_set_error(FT_ERR_INVALID_OPERATION));
#endif
}

void dbg_disable_crash_stack_traces(void) noexcept
{
#if defined(__linux__) || defined(__APPLE__)
    dbg_restore_signal_handler(SIGTERM, &g_dbg_previous_sigterm,
        &g_dbg_has_sigterm);
    dbg_restore_signal_handler(SIGSEGV, &g_dbg_previous_sigsegv,
        &g_dbg_has_sigsegv);
    dbg_restore_signal_handler(SIGILL, &g_dbg_previous_sigill,
        &g_dbg_has_sigill);
    dbg_restore_signal_handler(SIGFPE, &g_dbg_previous_sigfpe,
        &g_dbg_has_sigfpe);
    dbg_restore_signal_handler(SIGBUS, &g_dbg_previous_sigbus,
        &g_dbg_has_sigbus);
    dbg_restore_signal_handler(SIGABRT, &g_dbg_previous_sigabrt,
        &g_dbg_has_sigabrt);
    dbg_restore_signal_stack();
#endif
    g_dbg_enabled = FT_FALSE;
    dbg_set_error(FT_ERR_SUCCESS);
    return ;
}

ft_bool dbg_crash_stack_traces_enabled(void) noexcept
{
    return (g_dbg_enabled);
}

void dbg_print_stack_trace(void) noexcept
{
#if defined(__linux__) || defined(__APPLE__)
    dbg_write_literal(STDERR_FILENO, "libft debug: stack trace:\n");
    dbg_print_stack_trace_to_fd(STDERR_FILENO, 1);
#endif
    return ;
}

int32_t dbg_symbolize_address(const void *address, char *symbol_buffer,
    ft_size_t symbol_buffer_size, char *location_buffer,
    ft_size_t location_buffer_size) noexcept
{
    return (dbg_set_error(cmp_stack_trace_symbolize_address(address,
        symbol_buffer, symbol_buffer_size, location_buffer,
        location_buffer_size)));
}
