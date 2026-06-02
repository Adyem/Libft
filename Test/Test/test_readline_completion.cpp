#include "../test_internal.hpp"
#include <cstring>
#include "../../Modules/ReadLine/readline.hpp"
#include "../../Modules/ReadLine/readline_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Advanced/advanced.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

static void test_readline_cleanup_state(readline_state_t *state)
{
    ft_bool lock_acquired;
    int32_t lock_result;

    if (state == ft_nullptr)
        return ;
    lock_acquired = FT_FALSE;
    lock_result = rl_state_lock(state, &lock_acquired);
    if (lock_result == 0 && state->buffer != ft_nullptr)
    {
        cma_free(state->buffer);
        state->buffer = ft_nullptr;
    }
    if (lock_result == 0)
    {
        if (lock_acquired == FT_TRUE)
            rl_state_unlock(state, lock_acquired);
    }
    rl_state_teardown_thread_safety(state);
    return ;
}

static int test_readline_completion_callback(const char *buffer, int position,
    const char *prefix, void *user_data)
{
    int *callback_counter_pointer;
    int add_result;

    (void)buffer;
    (void)position;
    (void)prefix;
    callback_counter_pointer = static_cast<int *>(user_data);
    if (callback_counter_pointer != ft_nullptr)
        *callback_counter_pointer += 1;
    add_result = rl_completion_add_candidate("custom-option");
    if (add_result != 0)
        return (-1);
    return (0);
}

FT_TEST(test_readline_tab_completion_rejects_long_prefix)
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
    state.buffer_size = buffer_length + 1;
    state.position = buffer_length;
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
        state.current_match_scores[index] = 0;
        index++;
    }
    suggestion_count = 0;
    result = rl_handle_tab_completion(&state, "> ");
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, result);
    FT_ASSERT_EQ(0, state.in_completion_mode);
    FT_ASSERT_EQ(buffer_length, state.position);
    FT_ASSERT_EQ(0, std::strcmp(buffer, state.buffer));
    cma_free(buffer);
    return (1);
}

FT_TEST(test_readline_tab_completion_preserves_suffix)
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
    state.buffer_size = buffer_capacity;
    state.position = 7;
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
        state.current_match_scores[index] = 0;
        index++;
    }
    result = rl_handle_tab_completion(&state, "> ");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(1, state.in_completion_mode);
    FT_ASSERT_EQ(0, std::strcmp("say hello there", state.buffer));
    FT_ASSERT_EQ(9, state.position);
    FT_ASSERT_EQ(ft_strlen(state.buffer), state.prev_buffer_length);
    rl_clear_suggestions();
    cma_free(buffer);
    return (1);
}

FT_TEST(test_readline_tab_completion_supports_fuzzy_matches)
{
    readline_state_t state;
    int initialize_result;
    int completion_result;
    char *suggestions_backup[MAX_SUGGESTIONS];
    int32_t suggestion_count_backup;
    int32_t backup_index;

    backup_index = 0;
    suggestion_count_backup = suggestion_count;
    while (backup_index < MAX_SUGGESTIONS)
    {
        suggestions_backup[backup_index] = suggestions[backup_index];
        backup_index += 1;
    }
    rl_clear_suggestions();
    rl_add_suggestion("hello");
    ft_bzero(&state, sizeof(state));
    initialize_result = rl_initialize_state(&state);
    if (initialize_result != FT_ERR_SUCCESS)
    {
        backup_index = 0;
        while (backup_index < MAX_SUGGESTIONS)
        {
            suggestions[backup_index] = suggestions_backup[backup_index];
            backup_index += 1;
        }
        suggestion_count = suggestion_count_backup;
        return (0);
    }
    ft_strlcpy(state.buffer, "hlo", static_cast<size_t>(state.buffer_size));
    state.position = 3;
    state.in_completion_mode = 0;
    completion_result = rl_handle_tab_completion(&state, "> ");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, completion_result);
    FT_ASSERT_EQ(0, std::strcmp(state.buffer, "hello"));
    FT_ASSERT_EQ(1, state.in_completion_mode);
    rl_clear_suggestions();
    backup_index = 0;
    while (backup_index < MAX_SUGGESTIONS)
    {
        suggestions[backup_index] = suggestions_backup[backup_index];
        backup_index += 1;
    }
    suggestion_count = suggestion_count_backup;
    rl_disable_raw_mode();
    test_readline_cleanup_state(&state);
    return (1);
}

FT_TEST(test_readline_completion_callbacks_register)
{
    int set_result;
    int prepare_result;
    int callback_counter;
    int suggestion_count_local;
    int dynamic_count;
    char *match_pointer;

    rl_completion_reset_dynamic_matches();
    callback_counter = 0;
    set_result = rl_set_completion_callback(test_readline_completion_callback, &callback_counter);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_result);
    prepare_result = rl_completion_prepare_candidates("input", 5, "input", 5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, prepare_result);
    FT_ASSERT_EQ(1, callback_counter);
    dynamic_count = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rl_completion_get_dynamic_count(&dynamic_count));
    suggestion_count_local = dynamic_count;
    FT_ASSERT_EQ(1, suggestion_count_local);
    match_pointer = rl_completion_get_dynamic_match(0);
    FT_ASSERT(match_pointer != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(match_pointer, "custom-option"));
    rl_completion_reset_dynamic_matches();
    dynamic_count = -1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rl_completion_get_dynamic_count(&dynamic_count));
    FT_ASSERT_EQ(0, dynamic_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rl_set_completion_callback(ft_nullptr, ft_nullptr));
    return (1);
}
