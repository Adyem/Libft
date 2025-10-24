#include <fcntl.h>
#include <sys/stat.h>
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_file.hpp"
#include "../Errno/errno.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/pthread.hpp"
#include "readline_internal.hpp"
#include "readline.hpp"

#define RL_MAX_KEY_BINDINGS 64

struct rl_key_binding_entry
{
    int key;
    t_rl_key_binding_callback callback;
    void *user_data;
};

static pt_mutex g_customization_mutex;
static rl_key_binding_entry g_key_bindings[RL_MAX_KEY_BINDINGS];
static int g_key_binding_count = 0;
static t_rl_completion_callback g_completion_callback = ft_nullptr;
static void *g_completion_user_data = ft_nullptr;
static char *g_dynamic_suggestions[MAX_SUGGESTIONS];
static int g_dynamic_suggestion_count = 0;
static char *g_history_storage_path = ft_nullptr;
static bool g_history_auto_save = false;

static int rl_customization_lock(bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    g_customization_mutex.lock(THREAD_ID);
    if (g_customization_mutex.get_error() != ER_SUCCESS)
    {
        ft_errno = g_customization_mutex.get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

static void rl_customization_unlock(bool lock_acquired)
{
    if (lock_acquired == false)
        return ;
    g_customization_mutex.unlock(THREAD_ID);
    if (g_customization_mutex.get_error() != ER_SUCCESS)
    {
        ft_errno = g_customization_mutex.get_error();
        return ;
    }
    return ;
}

static void rl_completion_reset_dynamic_matches_locked(void)
{
    int index;

    index = 0;
    while (index < g_dynamic_suggestion_count)
    {
        if (g_dynamic_suggestions[index] != ft_nullptr)
        {
            cma_free(g_dynamic_suggestions[index]);
            g_dynamic_suggestions[index] = ft_nullptr;
        }
        index += 1;
    }
    g_dynamic_suggestion_count = 0;
    return ;
}

int rl_bind_key(int key, t_rl_key_binding_callback callback, void *user_data)
{
    bool lock_acquired;
    int index;

    if (callback == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    index = 0;
    while (index < g_key_binding_count)
    {
        if (g_key_bindings[index].key == key)
        {
            g_key_bindings[index].callback = callback;
            g_key_bindings[index].user_data = user_data;
            rl_customization_unlock(lock_acquired);
            ft_errno = ER_SUCCESS;
            return (0);
        }
        index += 1;
    }
    if (g_key_binding_count >= RL_MAX_KEY_BINDINGS)
    {
        rl_customization_unlock(lock_acquired);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    g_key_bindings[g_key_binding_count].key = key;
    g_key_bindings[g_key_binding_count].callback = callback;
    g_key_bindings[g_key_binding_count].user_data = user_data;
    g_key_binding_count += 1;
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_unbind_key(int key)
{
    bool lock_acquired;
    int index;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    index = 0;
    while (index < g_key_binding_count)
    {
        if (g_key_bindings[index].key == key)
        {
            int move_index;

            move_index = index;
            while (move_index < g_key_binding_count - 1)
            {
                g_key_bindings[move_index] = g_key_bindings[move_index + 1];
                move_index += 1;
            }
            g_key_binding_count -= 1;
            rl_customization_unlock(lock_acquired);
            ft_errno = ER_SUCCESS;
            return (0);
        }
        index += 1;
    }
    rl_customization_unlock(lock_acquired);
    ft_errno = FT_ERR_NOT_FOUND;
    return (-1);
}

int rl_dispatch_custom_key(readline_state_t *state, const char *prompt, int key)
{
    bool lock_acquired;
    t_rl_key_binding_callback callback;
    void *user_data;
    int result;

    lock_acquired = false;
    callback = ft_nullptr;
    user_data = ft_nullptr;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    int index;

    index = 0;
    while (index < g_key_binding_count)
    {
        if (g_key_bindings[index].key == key)
        {
            callback = g_key_bindings[index].callback;
            user_data = g_key_bindings[index].user_data;
            break ;
        }
        index += 1;
    }
    rl_customization_unlock(lock_acquired);
    if (callback == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    result = callback(state, prompt, user_data);
    if (result != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (1);
}

int rl_state_insert_text(readline_state_t *state, const char *text)
{
    bool lock_acquired;
    int text_length;
    int suffix_length;
    long long total_length;
    int required_size;
    int result;

    if (state == ft_nullptr || text == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    text_length = ft_strlen(text);
    if (text_length == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    suffix_length = ft_strlen(&state->buffer[state->pos]);
    total_length = static_cast<long long>(state->pos) + static_cast<long long>(text_length) + static_cast<long long>(suffix_length);
    required_size = static_cast<int>(total_length) + 1;
    if (total_length > static_cast<long long>(FT_INT_MAX))
    {
        result = -1;
        ft_errno = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    if (required_size > state->bufsize)
    {
        int new_bufsize;
        char *resized_buffer;

        new_bufsize = state->bufsize;
        if (new_bufsize <= 0)
            new_bufsize = 1;
        while (required_size > new_bufsize)
        {
            if (new_bufsize > FT_INT_MAX / 2)
            {
                ft_errno = FT_ERR_OUT_OF_RANGE;
                result = -1;
                goto cleanup;
            }
            new_bufsize *= 2;
        }
        resized_buffer = rl_resize_buffer(state->buffer, state->bufsize, new_bufsize);
        if (resized_buffer == ft_nullptr)
        {
            result = -1;
            goto cleanup;
        }
        state->buffer = resized_buffer;
        state->bufsize = new_bufsize;
    }
    ft_memmove(&state->buffer[state->pos + text_length], &state->buffer[state->pos], static_cast<size_t>(suffix_length) + 1);
    ft_memcpy(&state->buffer[state->pos], text, static_cast<size_t>(text_length));
    state->pos += text_length;
    result = 0;
cleanup:
    rl_state_unlock(state, lock_acquired);
    if (result == 0)
        ft_errno = ER_SUCCESS;
    return (result);
}

int rl_state_delete_previous_grapheme(readline_state_t *state)
{
    bool lock_acquired;
    int result;

    if (state == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    result = 0;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    if (state->pos > 0)
    {
        int grapheme_start;
        int grapheme_end;
        int grapheme_columns;
        int tail_length;

        if (rl_utf8_find_previous_grapheme(state->buffer, state->pos, &grapheme_start, &grapheme_end, &grapheme_columns) != 0)
        {
            result = -1;
            goto cleanup;
        }
        tail_length = ft_strlen(&state->buffer[grapheme_end]) + 1;
        ft_memmove(&state->buffer[grapheme_start], &state->buffer[grapheme_end], tail_length);
        state->pos = grapheme_start;
    }
cleanup:
    rl_state_unlock(state, lock_acquired);
    if (result == 0)
        ft_errno = ER_SUCCESS;
    return (result);
}

int rl_state_set_cursor(readline_state_t *state, int new_position)
{
    bool lock_acquired;
    int buffer_length;

    if (state == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (new_position < 0)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    buffer_length = ft_strlen(state->buffer);
    if (new_position > buffer_length)
    {
        rl_state_unlock(state, lock_acquired);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    state->pos = new_position;
    rl_state_unlock(state, lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_state_get_cursor(readline_state_t *state, int *out_position)
{
    bool lock_acquired;

    if (state == ft_nullptr || out_position == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    *out_position = state->pos;
    rl_state_unlock(state, lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_state_get_buffer(readline_state_t *state, const char **out_buffer)
{
    bool lock_acquired;

    if (state == ft_nullptr || out_buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    *out_buffer = state->buffer;
    rl_state_unlock(state, lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_state_refresh_display(readline_state_t *state, const char *prompt)
{
    bool lock_acquired;
    int result;
    int columns_after_cursor;

    if (state == ft_nullptr || prompt == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    result = 0;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    if (rl_update_display_metrics(state) != 0)
    {
        result = -1;
        goto cleanup;
    }
    if (rl_clear_line(prompt, state->buffer) == -1)
    {
        result = -1;
        goto cleanup;
    }
    pf_printf("%s%s", prompt, state->buffer);
    columns_after_cursor = state->prev_display_columns - state->display_pos;
    if (columns_after_cursor > 0)
        pf_printf("\033[%dD", columns_after_cursor);
    fflush(stdout);
cleanup:
    rl_state_unlock(state, lock_acquired);
    if (result == 0)
        ft_errno = ER_SUCCESS;
    return (result);
}

int rl_set_completion_callback(t_rl_completion_callback callback, void *user_data)
{
    bool lock_acquired;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    g_completion_callback = callback;
    g_completion_user_data = user_data;
    rl_completion_reset_dynamic_matches_locked();
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_completion_add_candidate(const char *candidate)
{
    bool lock_acquired;
    char *duplicated;

    if (candidate == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    if (g_dynamic_suggestion_count >= MAX_SUGGESTIONS)
    {
        rl_customization_unlock(lock_acquired);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    duplicated = cma_strdup(candidate);
    if (duplicated == ft_nullptr)
    {
        rl_customization_unlock(lock_acquired);
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    g_dynamic_suggestions[g_dynamic_suggestion_count] = duplicated;
    g_dynamic_suggestion_count += 1;
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

void rl_completion_reset_dynamic_matches(void)
{
    bool lock_acquired;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return ;
    rl_completion_reset_dynamic_matches_locked();
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return ;
}

int rl_completion_prepare_candidates(const char *buffer, int cursor_position,
        const char *prefix, int prefix_length)
{
    bool lock_acquired;
    t_rl_completion_callback callback;
    void *user_data;
    int callback_result;

    (void)prefix_length;
    if (buffer == ft_nullptr || prefix == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    rl_completion_reset_dynamic_matches_locked();
    callback = g_completion_callback;
    user_data = g_completion_user_data;
    rl_customization_unlock(lock_acquired);
    if (callback == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    callback_result = callback(buffer, cursor_position, prefix, user_data);
    if (callback_result != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_completion_get_dynamic_count(void)
{
    bool lock_acquired;
    int count;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (0);
    count = g_dynamic_suggestion_count;
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (count);
}

char *rl_completion_get_dynamic_match(int index)
{
    bool lock_acquired;
    char *result;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (ft_nullptr);
    if (index < 0 || index >= g_dynamic_suggestion_count)
    {
        rl_customization_unlock(lock_acquired);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (ft_nullptr);
    }
    result = g_dynamic_suggestions[index];
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (result);
}

int rl_history_set_storage_path(const char *file_path)
{
    bool lock_acquired;
    char *new_path;

    lock_acquired = false;
    new_path = ft_nullptr;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    if (file_path != ft_nullptr)
    {
        new_path = cma_strdup(file_path);
        if (new_path == ft_nullptr)
        {
            rl_customization_unlock(lock_acquired);
            ft_errno = FT_ERR_NO_MEMORY;
            return (-1);
        }
    }
    if (g_history_storage_path != ft_nullptr)
        cma_free(g_history_storage_path);
    g_history_storage_path = new_path;
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_history_enable_auto_save(bool enabled)
{
    bool lock_acquired;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    g_history_auto_save = enabled;
    rl_customization_unlock(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_history_load(void)
{
    bool lock_acquired;
    char *path_copy;
    ft_file history_file;
    int open_result;
    int fd;
    char *line;

    lock_acquired = false;
    path_copy = ft_nullptr;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    if (g_history_storage_path == ft_nullptr)
    {
        rl_customization_unlock(lock_acquired);
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    path_copy = cma_strdup(g_history_storage_path);
    rl_customization_unlock(lock_acquired);
    if (path_copy == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    open_result = history_file.open(path_copy, O_RDONLY);
    if (open_result != 0)
    {
        int error_code;

        error_code = history_file.get_error();
        cma_free(path_copy);
        if (error_code == FT_ERR_NOT_FOUND)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        ft_errno = error_code;
        return (-1);
    }
    rl_clear_history();
    fd = history_file.get_fd();
    if (history_file.get_error() != ER_SUCCESS)
    {
        int error_code;

        error_code = history_file.get_error();
        history_file.close();
        cma_free(path_copy);
        ft_errno = error_code;
        return (-1);
    }
    while (1)
    {
        line = get_next_line(fd, 1024);
        if (line == ft_nullptr)
            break ;
        size_t length;

        length = ft_strlen_size_t(line);
        if (length > 0 && line[length - 1] == '\n')
            line[length - 1] = '\0';
        rl_update_history(line);
        cma_free(line);
    }
    history_file.close();
    cma_free(path_copy);
    ft_errno = ER_SUCCESS;
    return (0);
}

int rl_history_save(void)
{
    bool lock_acquired;
    char *path_copy;
    ft_file history_file;
    int open_result;
    int index;

    lock_acquired = false;
    path_copy = ft_nullptr;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    if (g_history_storage_path == ft_nullptr)
    {
        rl_customization_unlock(lock_acquired);
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    path_copy = cma_strdup(g_history_storage_path);
    rl_customization_unlock(lock_acquired);
    if (path_copy == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    open_result = history_file.open(path_copy, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (open_result != 0)
    {
        int error_code;

        error_code = history_file.get_error();
        cma_free(path_copy);
        ft_errno = error_code;
        return (-1);
    }
    index = 0;
    while (index < history_count)
    {
        if (history[index] != ft_nullptr)
        {
            if (history_file.printf("%s\n", history[index]) < 0)
            {
                int error_code;

                error_code = history_file.get_error();
                history_file.close();
                cma_free(path_copy);
                ft_errno = error_code;
                return (-1);
            }
        }
        index += 1;
    }
    history_file.close();
    cma_free(path_copy);
    ft_errno = ER_SUCCESS;
    return (0);
}

void rl_history_notify_updated(void)
{
    bool lock_acquired;
    bool auto_save_enabled;

    lock_acquired = false;
    auto_save_enabled = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return ;
    auto_save_enabled = g_history_auto_save;
    rl_customization_unlock(lock_acquired);
    if (!auto_save_enabled)
        return ;
    (void)rl_history_save();
    return ;
}
