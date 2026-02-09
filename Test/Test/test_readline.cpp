#include <unistd.h>
#include <fcntl.h>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstring>
#include <thread>
#include "../../ReadLine/readline.hpp"
#include "../../ReadLine/readline_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Basic/basic.hpp"

static void test_readline_cleanup_state(readline_state_t *state)
{
    bool lock_acquired;

    if (state == ft_nullptr)
        return ;
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) == 0)
    {
        if (lock_acquired == true && state->buffer != ft_nullptr)
        {
            cma_free(state->buffer);
            state->buffer = ft_nullptr;
        }
        if (lock_acquired == true)
            rl_state_unlock(state, lock_acquired);
    }
    rl_state_teardown_thread_safety(state);
    return ;
}

static void test_readline_suppress_stderr(int *backup_descriptor)
{
    int null_descriptor;

    if (backup_descriptor == ft_nullptr)
        return ;
    *backup_descriptor = -1;
    null_descriptor = open("/dev/null", O_WRONLY);
    if (null_descriptor == -1)
        return ;
    *backup_descriptor = dup(STDERR_FILENO);
    if (*backup_descriptor == -1)
    {
        close(null_descriptor);
        return ;
    }
    if (dup2(null_descriptor, STDERR_FILENO) == -1)
    {
        close(null_descriptor);
        close(*backup_descriptor);
        *backup_descriptor = -1;
        return ;
    }
    close(null_descriptor);
    return ;
}

static void test_readline_restore_stderr(int backup_descriptor)
{
    if (backup_descriptor == -1)
        return ;
    dup2(backup_descriptor, STDERR_FILENO);
    close(backup_descriptor);
    return ;
}

static int test_readline_custom_key_callback(readline_state_t *state, const char *prompt, void *user_data)
{
    int *counter_pointer;

    (void)state;
    (void)prompt;
    counter_pointer = static_cast<int *>(user_data);
    if (counter_pointer != ft_nullptr)
        *counter_pointer += 1;
    return (0);
}

static int test_readline_completion_callback(const char *buffer, int cursor_position,
    const char *prefix, void *user_data)
{
    int *callback_counter_pointer;
    int add_result;

    (void)buffer;
    (void)cursor_position;
    (void)prefix;
    callback_counter_pointer = static_cast<int *>(user_data);
    if (callback_counter_pointer != ft_nullptr)
        *callback_counter_pointer += 1;
    add_result = rl_completion_add_candidate("custom-option");
    if (add_result != 0)
        return (-1);
    return (0);
}

FT_TEST(test_readline_clear_line_null_prompt, "rl_clear_line rejects null prompts")
{
    int clear_result;
    const char *buffer;

    buffer = "";
    ft_errno = FT_ERR_SUCCESSS;
    clear_result = rl_clear_line(ft_nullptr, buffer);
    FT_ASSERT_EQ(-1, clear_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_readline_clear_line_null_buffer, "rl_clear_line rejects null buffers")
{
    int clear_result;
    const char *prompt;

    prompt = "> ";
    ft_errno = FT_ERR_SUCCESSS;
    clear_result = rl_clear_line(prompt, ft_nullptr);
    FT_ASSERT_EQ(-1, clear_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_readline_initialize_state_null_pointer, "rl_initialize_state rejects null state pointers")
{
    int init_result;

    ft_errno = FT_ERR_SUCCESSS;
    init_result = rl_initialize_state(ft_nullptr);
    FT_ASSERT_EQ(1, init_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_readline_initialize_state_allocation_failure, "rl_initialize_state reports allocation failures")
{
    readline_state_t state;
    int init_result;

    ft_errno = FT_ERR_SUCCESSS;
    cma_set_alloc_limit(1);
    init_result = rl_initialize_state(&state);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(1, init_result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_readline_initialize_state_success, "rl_initialize_state sets up the state on success")
{
    readline_state_t state;
    int init_result;
    char *allocated_buffer;
    int buffer_size;
    int buffer_position;
    int buffer_history_index;
    int initialize_errno;

    ft_errno = FT_ERR_SUCCESSS;
    init_result = rl_initialize_state(&state);
    allocated_buffer = state.buffer;
    buffer_size = state.bufsize;
    buffer_position = state.pos;
    buffer_history_index = state.history_index;
    initialize_errno = ft_errno;
    rl_disable_raw_mode();
    if (allocated_buffer != ft_nullptr)
        cma_free(allocated_buffer);
    FT_ASSERT_EQ(0, init_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, initialize_errno);
    FT_ASSERT(allocated_buffer != ft_nullptr);
    FT_ASSERT_EQ(INITIAL_BUFFER_SIZE, buffer_size);
    FT_ASSERT_EQ(0, buffer_position);
    FT_ASSERT_EQ(history_count, buffer_history_index);
    return (1);
}

FT_TEST(test_readline_history_recall_resizes_buffer, "history recall grows the active buffer")
{
    readline_state_t state;
    const char *prompt = "> ";
    char *history_entry;
    ssize_t written;
    int stdin_backup;
    int pipe_descriptors[2];
    int result;
    char sequence[2];

    result = 1;
    pipe_descriptors[0] = -1;
    pipe_descriptors[1] = -1;
    stdin_backup = -1;
    sequence[0] = '[';
    sequence[1] = 'A';
    history_entry = cma_strdup("this is a history entry that exceeds the initial buffer size");
    if (!history_entry)
        return (0);
    history[0] = history_entry;
    history_count = 1;
    state.buffer = static_cast<char *>(cma_calloc(4, sizeof(char)));
    if (!state.buffer)
    {
        result = 0;
        goto cleanup;
    }
    state.bufsize = 4;
    state.pos = 0;
    state.prev_buffer_length = 0;
    state.display_pos = 0;
    state.prev_display_columns = 0;
    state.history_index = history_count;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    std::memset(state.current_matches, 0, sizeof(state.current_matches));
    stdin_backup = dup(STDIN_FILENO);
    if (stdin_backup < 0)
    {
        result = 0;
        goto cleanup;
    }
    if (pipe(pipe_descriptors) != 0)
    {
        result = 0;
        goto cleanup;
    }
    if (dup2(pipe_descriptors[0], STDIN_FILENO) == -1)
    {
        result = 0;
        goto cleanup;
    }
    written = write(pipe_descriptors[1], sequence, sizeof(sequence));
    if (written != static_cast<ssize_t>(sizeof(sequence)))
    {
        result = 0;
        goto cleanup;
    }
    if (rl_handle_escape_sequence(&state, prompt) == -1)
    {
        result = 0;
        goto cleanup;
    }
    if (std::strcmp(state.buffer, history_entry) != 0)
    {
        result = 0;
        goto cleanup;
    }
    if (state.bufsize < static_cast<int>(std::strlen(history_entry) + 1))
    {
        result = 0;
        goto cleanup;
    }
cleanup:
    if (pipe_descriptors[0] != -1)
        close(pipe_descriptors[0]);
    if (pipe_descriptors[1] != -1)
        close(pipe_descriptors[1]);
    if (stdin_backup != -1)
    {
        dup2(stdin_backup, STDIN_FILENO);
        close(stdin_backup);
    }
    if (state.buffer)
        cma_free(state.buffer);
    if (history_entry)
        cma_free(history_entry);
    history[0] = ft_nullptr;
    history_count = 0;
    if (result == 0)
        return (0);
    return (1);
}

FT_TEST(test_readline_printable_char_preserves_buffer_on_resize_failure, "rl_handle_printable_char keeps buffer on resize failure")
{
    readline_state_t state = {};
    const char *prompt;
    char *initial_buffer;
    int handle_result;
    int stderr_backup_descriptor;

    prompt = "> ";
    initial_buffer = static_cast<char *>(cma_malloc(4));
    if (initial_buffer == ft_nullptr)
        return (0);
    initial_buffer[0] = 'x';
    initial_buffer[1] = '\0';
    state.buffer = initial_buffer;
    state.bufsize = 2;
    state.pos = state.bufsize - 1;
    state.prev_buffer_length = 1;
    state.display_pos = 1;
    state.prev_display_columns = 1;
    state.history_index = 0;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    ft_errno = FT_ERR_SUCCESSS;
    stderr_backup_descriptor = -1;
    test_readline_suppress_stderr(&stderr_backup_descriptor);
    cma_set_alloc_limit(1);
    handle_result = rl_handle_printable_char(&state, 'a', prompt);
    cma_set_alloc_limit(0);
    test_readline_restore_stderr(stderr_backup_descriptor);
    FT_ASSERT_EQ(-1, handle_result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    FT_ASSERT_EQ(initial_buffer, state.buffer);
    FT_ASSERT_EQ(2, state.bufsize);
    FT_ASSERT_EQ(1, state.pos);
    FT_ASSERT_EQ('x', state.buffer[0]);
    FT_ASSERT_EQ('\0', state.buffer[1]);
    cma_free(initial_buffer);
    return (1);
}

FT_TEST(test_readline_tab_completion_rejects_long_prefix, "rl_handle_tab_completion refuses overly long prefixes")
{
    readline_state_t state;
    char *buffer;
    int index;
    int result;
    int buffer_length;

    buffer_length = INITIAL_BUFFER_SIZE + 8;
    buffer = static_cast<char *>(cma_malloc(buffer_length + 1));
    if (buffer == ft_nullptr)
        return (0);
    index = 0;
    while (index < buffer_length)
    {
        buffer[index] = 'a';
        index++;
    }
    buffer[buffer_length] = '\0';
    state.buffer = buffer;
    state.bufsize = buffer_length + 1;
    state.pos = buffer_length;
    state.prev_buffer_length = buffer_length;
    state.display_pos = buffer_length;
    state.prev_display_columns = buffer_length;
    state.history_index = 0;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    index = 0;
    while (index < MAX_SUGGESTIONS)
    {
        state.current_matches[index] = ft_nullptr;
        index++;
    }
    suggestion_count = 0;
    ft_errno = FT_ERR_SUCCESSS;
    result = rl_handle_tab_completion(&state, "> ");
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    FT_ASSERT_EQ(0, state.in_completion_mode);
    FT_ASSERT_EQ(buffer_length, state.pos);
    FT_ASSERT_EQ(0, std::strcmp(buffer, state.buffer));
    cma_free(buffer);
    return (1);
}

FT_TEST(test_readline_tab_completion_preserves_suffix, "rl_handle_tab_completion keeps trailing text intact")
{
    readline_state_t state;
    char *buffer;
    const char *initial_line;
    int index;
    int result;
    int buffer_capacity;

    rl_clear_suggestions();
    rl_add_suggestion("hello");
    initial_line = "say hel there";
    buffer_capacity = 64;
    buffer = static_cast<char *>(cma_malloc(buffer_capacity));
    if (buffer == ft_nullptr)
    {
        rl_clear_suggestions();
        return (0);
    }
    ft_strlcpy(buffer, initial_line, buffer_capacity);
    state.buffer = buffer;
    state.bufsize = buffer_capacity;
    state.pos = 7;
    state.prev_buffer_length = ft_strlen(buffer);
    state.display_pos = 7;
    state.prev_display_columns = 7;
    state.history_index = 0;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    index = 0;
    while (index < MAX_SUGGESTIONS)
    {
        state.current_matches[index] = ft_nullptr;
        index++;
    }
    ft_errno = FT_ERR_SUCCESSS;
    result = rl_handle_tab_completion(&state, "> ");
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(1, state.in_completion_mode);
    FT_ASSERT_EQ(0, std::strcmp("say hello there", state.buffer));
    FT_ASSERT_EQ(9, state.pos);
    FT_ASSERT_EQ(ft_strlen(state.buffer), state.prev_buffer_length);
    rl_clear_suggestions();
    cma_free(buffer);
    return (1);
}

FT_TEST(test_readline_utf8_backspace_removes_grapheme, "rl_handle_backspace erases entire utf8 grapheme")
{
    readline_state_t state;
    const char *prompt;
    char *buffer;
    int handle_result;
    const char initial_sequence[] = { 'A', static_cast<char>(0xC3), static_cast<char>(0xA9), 'B', '\0' };
    const char expected_first[] = { 'A', static_cast<char>(0xC3), static_cast<char>(0xA9), '\0' };
    const char expected_second[] = { 'A', '\0' };

    prompt = "> ";
    buffer = cma_strdup(initial_sequence);
    if (buffer == ft_nullptr)
        return (0);
    state.buffer = buffer;
    state.bufsize = ft_strlen(buffer) + 1;
    state.pos = ft_strlen(buffer);
    state.prev_buffer_length = ft_strlen(buffer);
    state.display_pos = 3;
    state.prev_display_columns = 3;
    state.history_index = 0;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    ft_errno = FT_ERR_SUCCESSS;
    handle_result = rl_handle_backspace(&state, prompt);
    FT_ASSERT_EQ(0, handle_result);
    FT_ASSERT(std::memcmp(state.buffer, expected_first, sizeof(expected_first)) == 0);
    FT_ASSERT_EQ(ft_strlen(state.buffer), state.pos);
    FT_ASSERT_EQ(2, state.prev_display_columns);
    handle_result = rl_handle_backspace(&state, prompt);
    FT_ASSERT_EQ(0, handle_result);
    FT_ASSERT(std::memcmp(state.buffer, expected_second, sizeof(expected_second)) == 0);
    FT_ASSERT_EQ(ft_strlen(state.buffer), state.pos);
    FT_ASSERT_EQ(1, state.prev_display_columns);
    cma_free(buffer);
    return (1);
}

FT_TEST(test_readline_utf8_printable_updates_display, "rl_handle_printable_char tracks utf8 display width")
{
    readline_state_t state;
    const char *prompt;
    char *buffer;

    prompt = "> ";
    buffer = static_cast<char *>(cma_calloc(16, sizeof(char)));
    if (buffer == ft_nullptr)
        return (0);
    state.buffer = buffer;
    state.bufsize = 16;
    state.pos = 0;
    state.prev_buffer_length = 0;
    state.display_pos = 0;
    state.prev_display_columns = 0;
    state.history_index = 0;
    state.in_completion_mode = 0;
    state.current_match_count = 0;
    state.current_match_index = 0;
    state.word_start = 0;
    FT_ASSERT_EQ(0, rl_handle_printable_char(&state, static_cast<char>(0xC3), prompt));
    FT_ASSERT_EQ(1, state.pos);
    FT_ASSERT(state.prev_display_columns >= 1);
    FT_ASSERT_EQ(0, rl_handle_printable_char(&state, static_cast<char>(0xA9), prompt));
    FT_ASSERT_EQ(2, state.pos);
    FT_ASSERT_EQ(1, state.prev_display_columns);
    FT_ASSERT_EQ(0, rl_handle_printable_char(&state, '!', prompt));
    FT_ASSERT_EQ(3, state.pos);
    FT_ASSERT_EQ(2, state.prev_display_columns);
    cma_free(buffer);
    return (1);
}

FT_TEST(test_readline_state_thread_safety_prepare_null,
    "rl_state_prepare_thread_safety validates null arguments")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, rl_state_prepare_thread_safety(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_readline_state_thread_safety_lock_without_prepare,
    "rl_state_lock allows access when thread safety disabled")
{
    readline_state_t state;
    bool lock_acquired;

    ft_bzero(&state, sizeof(state));
    state.thread_safe_enabled = false;
    state.mutex = ft_nullptr;
    lock_acquired = true;
    ft_errno = FT_ERR_SOCKET_ACCEPT_FAILED;
    FT_ASSERT_EQ(0, rl_state_lock(&state, &lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_readline_state_thread_safety_unlock_without_lock,
    "rl_state_unlock resets errno when lock not held")
{
    readline_state_t state;

    ft_bzero(&state, sizeof(state));
    state.thread_safe_enabled = false;
    state.mutex = ft_nullptr;
    ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
    rl_state_unlock(&state, false);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_readline_state_thread_safety_lifecycle,
    "rl_state thread safety guards serialize access and reset errno")
{
    readline_state_t state;
    bool main_lock_acquired;
    bool relock_acquired;
    std::atomic<bool> worker_ready;
    std::atomic<bool> worker_has_lock;
    std::atomic<int> worker_result;
    std::thread worker;

    ft_bzero(&state, sizeof(state));
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, rl_state_prepare_thread_safety(&state));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT(state.thread_safe_enabled);
    FT_ASSERT(state.mutex != ft_nullptr);

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, rl_state_lock(&state, &main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    worker_ready.store(false);
    worker_has_lock.store(false);
    worker_result.store(0);
    worker = std::thread([&state, &worker_ready, &worker_has_lock, &worker_result]() {
        bool worker_lock_acquired;

        worker_lock_acquired = false;
        worker_ready.store(true);
        if (rl_state_lock(&state, &worker_lock_acquired) != 0)
        {
            worker_result.store(-1);
            return ;
        }
        if (worker_lock_acquired == true)
            worker_has_lock.store(true);
        worker_result.store(1);
        rl_state_unlock(&state, worker_lock_acquired);
    });
    while (worker_ready.load() == false)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    rl_state_unlock(&state, main_lock_acquired);
    worker.join();

    FT_ASSERT_EQ(1, worker_result.load());
    FT_ASSERT(worker_has_lock.load());

    relock_acquired = false;
    FT_ASSERT_EQ(0, rl_state_lock(&state, &relock_acquired));
    FT_ASSERT(relock_acquired);
    ft_errno = FT_ERR_NO_MEMORY;
    rl_state_unlock(&state, relock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    rl_state_teardown_thread_safety(&state);
    FT_ASSERT(state.thread_safe_enabled == false);
    FT_ASSERT(state.mutex == ft_nullptr);
    return (1);
}

FT_TEST(test_readline_terminal_dimensions_thread_safety_prepare_null,
    "rl_terminal_dimensions_prepare_thread_safety validates null arguments")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, rl_terminal_dimensions_prepare_thread_safety(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_readline_terminal_dimensions_thread_safety_lock_without_prepare,
    "rl_terminal_dimensions_lock allows access when thread safety disabled")
{
    terminal_dimensions dimensions;
    bool lock_acquired;

    ft_bzero(&dimensions, sizeof(dimensions));
    dimensions.thread_safe_enabled = false;
    dimensions.mutex = ft_nullptr;
    lock_acquired = true;
    ft_errno = FT_ERR_SOCKET_LISTEN_FAILED;
    FT_ASSERT_EQ(0, rl_terminal_dimensions_lock(&dimensions, &lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_readline_terminal_dimensions_thread_safety_unlock_without_lock,
    "rl_terminal_dimensions_unlock resets errno when lock not held")
{
    terminal_dimensions dimensions;

    ft_bzero(&dimensions, sizeof(dimensions));
    dimensions.thread_safe_enabled = false;
    dimensions.mutex = ft_nullptr;
    ft_errno = FT_ERR_SOCKET_ACCEPT_FAILED;
    rl_terminal_dimensions_unlock(&dimensions, false);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_readline_terminal_dimensions_thread_safety_lifecycle,
    "terminal_dimensions thread safety guards serialize access and reset errno")
{
    terminal_dimensions dimensions;
    bool main_lock_acquired;
    bool relock_acquired;
    std::atomic<bool> worker_ready;
    std::atomic<bool> worker_has_lock;
    std::atomic<int> worker_result;
    std::thread worker;

    ft_bzero(&dimensions, sizeof(dimensions));
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, rl_terminal_dimensions_prepare_thread_safety(&dimensions));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT(dimensions.thread_safe_enabled);
    FT_ASSERT(dimensions.mutex != ft_nullptr);

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, rl_terminal_dimensions_lock(&dimensions, &main_lock_acquired));
    FT_ASSERT(main_lock_acquired);

    worker_ready.store(false);
    worker_has_lock.store(false);
    worker_result.store(0);
    worker = std::thread([&dimensions, &worker_ready, &worker_has_lock, &worker_result]() {
        bool worker_lock_acquired;

        worker_lock_acquired = false;
        worker_ready.store(true);
        if (rl_terminal_dimensions_lock(&dimensions, &worker_lock_acquired) != 0)
        {
            worker_result.store(-1);
            return ;
        }
        if (worker_lock_acquired == true)
            worker_has_lock.store(true);
        worker_result.store(1);
        rl_terminal_dimensions_unlock(&dimensions, worker_lock_acquired);
    });
    while (worker_ready.load() == false)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    rl_terminal_dimensions_unlock(&dimensions, main_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    worker.join();

    FT_ASSERT_EQ(1, worker_result.load());
    FT_ASSERT(worker_has_lock.load());

    relock_acquired = false;
    FT_ASSERT_EQ(0, rl_terminal_dimensions_lock(&dimensions, &relock_acquired));
    FT_ASSERT(relock_acquired);
    ft_errno = FT_ERR_IO;
    rl_terminal_dimensions_unlock(&dimensions, relock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    rl_terminal_dimensions_teardown_thread_safety(&dimensions);
    FT_ASSERT(dimensions.thread_safe_enabled == false);
    FT_ASSERT(dimensions.mutex == ft_nullptr);
    return (1);
}

FT_TEST(test_readline_custom_key_bindings_dispatch, "custom key bindings dispatch registered callbacks")
{
    readline_state_t state;
    int initialize_result;
    int bind_result;
    int dispatch_result;
    int unbind_result;
    int callback_counter;

    ft_bzero(&state, sizeof(state));
    callback_counter = 0;
    initialize_result = rl_initialize_state(&state);
    if (initialize_result != 0)
        return (0);
    bind_result = rl_bind_key('x', test_readline_custom_key_callback, &callback_counter);
    rl_disable_raw_mode();
    FT_ASSERT_EQ(0, bind_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    dispatch_result = rl_dispatch_custom_key(&state, "> ", 'x');
    FT_ASSERT_EQ(1, dispatch_result);
    FT_ASSERT_EQ(1, callback_counter);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    callback_counter = 0;
    unbind_result = rl_unbind_key('x');
    FT_ASSERT_EQ(0, unbind_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    dispatch_result = rl_dispatch_custom_key(&state, "> ", 'x');
    FT_ASSERT_EQ(0, dispatch_result);
    FT_ASSERT_EQ(0, callback_counter);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    test_readline_cleanup_state(&state);
    return (1);
}

FT_TEST(test_readline_state_insert_and_delete_text, "state helpers insert and delete graphemes")
{
    readline_state_t state;
    int insert_result;
    int delete_result;
    const char *buffer_pointer;
    int cursor_position;

    ft_bzero(&state, sizeof(state));
    state.buffer = static_cast<char *>(cma_calloc(4, sizeof(char)));
    if (state.buffer == ft_nullptr)
        return (0);
    state.bufsize = 4;
    state.pos = 0;
    insert_result = rl_state_insert_text(&state, "hello");
    FT_ASSERT_EQ(0, insert_result);
    FT_ASSERT(state.bufsize >= 6);
    FT_ASSERT_EQ(5, state.pos);
    FT_ASSERT_EQ(0, ft_strcmp(state.buffer, "hello"));
    state.pos = ft_strlen("h\xC3\xA9");
    ft_strlcpy(state.buffer, "h\xC3\xA9", static_cast<size_t>(state.bufsize));
    delete_result = rl_state_delete_previous_grapheme(&state);
    FT_ASSERT_EQ(0, delete_result);
    FT_ASSERT_EQ(1, state.pos);
    FT_ASSERT_EQ(0, ft_strcmp(state.buffer, "h"));
    delete_result = rl_state_delete_previous_grapheme(&state);
    FT_ASSERT_EQ(0, delete_result);
    FT_ASSERT_EQ(0, state.pos);
    buffer_pointer = ft_nullptr;
    FT_ASSERT_EQ(0, rl_state_get_buffer(&state, &buffer_pointer));
    FT_ASSERT(buffer_pointer == state.buffer);
    cursor_position = -1;
    FT_ASSERT_EQ(0, rl_state_get_cursor(&state, &cursor_position));
    FT_ASSERT_EQ(0, cursor_position);
    FT_ASSERT_EQ(0, rl_state_set_cursor(&state, 0));
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, rl_state_set_cursor(&state, INT_MAX));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    cma_free(state.buffer);
    state.buffer = ft_nullptr;
    return (1);
}

FT_TEST(test_readline_completion_callbacks_register, "completion hooks invoke callbacks and store candidates")
{
    int set_result;
    int prepare_result;
    int callback_counter;
    int suggestion_count;
    char *match_pointer;

    rl_completion_reset_dynamic_matches();
    callback_counter = 0;
    set_result = rl_set_completion_callback(test_readline_completion_callback, &callback_counter);
    FT_ASSERT_EQ(0, set_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    prepare_result = rl_completion_prepare_candidates("input", 5, "input", 5);
    FT_ASSERT_EQ(0, prepare_result);
    FT_ASSERT_EQ(1, callback_counter);
    suggestion_count = rl_completion_get_dynamic_count();
    FT_ASSERT_EQ(1, suggestion_count);
    match_pointer = rl_completion_get_dynamic_match(0);
    FT_ASSERT(match_pointer != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(match_pointer, "custom-option"));
    rl_completion_reset_dynamic_matches();
    FT_ASSERT_EQ(0, rl_completion_get_dynamic_count());
    rl_set_completion_callback(ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

