#include <fcntl.h>
#include <sys/stat.h>
#include "../sqlite_support.hpp"
#if SQLITE3_AVAILABLE
# include <sqlite3.h>
#endif
#include "../Advanced/advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_file.hpp"
#include "../Errno/errno.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../Basic/basic.hpp"
#include "../JSon/json.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "readline_internal.hpp"
#include "readline.hpp"

#define RL_MAX_KEY_BINDINGS 64

struct rl_key_binding_entry
{
    int key;
    t_rl_key_binding_callback callback;
    void *user_data;
};

static pt_recursive_mutex g_customization_mutex;
static bool g_customization_mutex_initialized = false;
static rl_key_binding_entry g_key_bindings[RL_MAX_KEY_BINDINGS];
static int g_key_binding_count = 0;
static t_rl_completion_callback g_completion_callback = ft_nullptr;
static void *g_completion_user_data = ft_nullptr;
static char *g_dynamic_suggestions[MAX_SUGGESTIONS];
static int g_dynamic_suggestion_count = 0;

struct rl_history_path_context
{
    char *path;
};

#if SQLITE3_AVAILABLE
struct rl_history_sqlite_context
{
    char *path;
    sqlite3 *database;
};

#endif
typedef struct s_rl_history_backend
{
    const char *name;
    int (*configure)(void **context_pointer, const char *location);
    void (*shutdown)(void *context_pointer);
    int (*load)(void *context_pointer);
    int (*save)(void *context_pointer);
}   rl_history_backend;

struct rl_history_backend_state
{
    const rl_history_backend *backend;
    void *backend_context;
    bool auto_save_enabled;
};

static rl_history_backend_state g_history_backend_state = {ft_nullptr, ft_nullptr, false};

static int rl_customization_lock_mutex(void)
{
    int initialize_error;

    if (g_customization_mutex_initialized == false)
    {
        initialize_error = g_customization_mutex.initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
        g_customization_mutex_initialized = true;
    }
    return (g_customization_mutex.lock());
}

static int rl_customization_unlock_mutex(void)
{
    if (g_customization_mutex_initialized == false)
        return (FT_ERR_SUCCESS);
    return (g_customization_mutex.unlock());
}

static int rl_customization_lock(bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    int mutex_error = rl_customization_lock_mutex();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

static int rl_customization_unlock(bool lock_acquired)
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    int mutex_error = rl_customization_unlock_mutex();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    return (FT_ERR_SUCCESS);
}

static int rl_history_assign_path(char **target_path, const char *location)
{
    char *new_path;

    if (target_path == ft_nullptr || location == ft_nullptr)
    {
        return (-1);
    }
    new_path = adv_strdup(location);
    if (new_path == ft_nullptr)
    {
        return (-1);
    }
    if (*target_path != ft_nullptr)
        cma_free(*target_path);
    *target_path = new_path;
    return (0);
}

static int rl_history_prepare_path_context(void **context_pointer, const char *location)
{
    rl_history_path_context *path_context;

    if (context_pointer == ft_nullptr || location == ft_nullptr)
    {
        return (-1);
    }
    path_context = static_cast<rl_history_path_context *>(*context_pointer);
    if (path_context == ft_nullptr)
    {
        path_context = static_cast<rl_history_path_context *>(cma_malloc(sizeof(*path_context)));
        if (path_context == ft_nullptr)
        {
            return (-1);
        }
        path_context->path = ft_nullptr;
        *context_pointer = path_context;
    }
    if (rl_history_assign_path(&path_context->path, location) != 0)
        return (-1);
    return (0);
}

static int rl_history_plain_configure(void **context_pointer, const char *location)
{
    if (rl_history_prepare_path_context(context_pointer, location) != 0)
        return (-1);
    return (0);
}

static void rl_history_plain_shutdown(void *context_pointer)
{
    rl_history_path_context *path_context;

    if (context_pointer == ft_nullptr)
        return ;
    path_context = static_cast<rl_history_path_context *>(context_pointer);
    if (path_context->path != ft_nullptr)
        cma_free(path_context->path);
    cma_free(path_context);
    return ;
}

static int rl_history_plain_load(void *context_pointer)
{
    rl_history_path_context *path_context;
    ft_file history_file;
    int open_result;
    int file_descriptor;

    path_context = static_cast<rl_history_path_context *>(context_pointer);
    if (path_context == ft_nullptr || path_context->path == ft_nullptr)
    {
        return (-1);
    }
    open_result = history_file.open(path_context->path, O_RDONLY);
    if (open_result != 0)
    {
        int error_code = history_file.get_error();

        if (error_code == FT_ERR_NOT_FOUND)
        {
            return (0);
        }
        return (-1);
    }
    rl_clear_history();
    file_descriptor = history_file.get_fd();
    if (history_file.get_error() != FT_ERR_SUCCESS)
    {
        history_file.close();
        return (-1);
    }
    while (1)
    {
        char *line_buffer;

        line_buffer = get_next_line(file_descriptor, 1024);
        if (line_buffer == ft_nullptr)
            break ;
        size_t line_length;

        line_length = ft_strlen_size_t(line_buffer);
        if (line_length > 0 && line_buffer[line_length - 1] == '\n')
            line_buffer[line_length - 1] = '\0';
        rl_update_history(line_buffer);
        cma_free(line_buffer);
    }
    history_file.close();
    return (0);
}

static int rl_history_plain_save(void *context_pointer)
{
    rl_history_path_context *path_context;
    ft_file history_file;
    int open_result;
    int history_index;

    path_context = static_cast<rl_history_path_context *>(context_pointer);
    if (path_context == ft_nullptr || path_context->path == ft_nullptr)
    {
        return (-1);
    }
    open_result = history_file.open(path_context->path,
            O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (open_result != 0)
    {
        return (-1);
    }
    history_index = 0;
    while (history_index < history_count)
    {
        if (history[history_index] != ft_nullptr)
        {
        if (history_file.printf("%s\n", history[history_index]) < 0)
        {
            history_file.close();
            return (-1);
        }
        }
        history_index += 1;
    }
    history_file.close();
    return (0);
}

static int rl_history_json_configure(void **context_pointer, const char *location)
{
    if (rl_history_prepare_path_context(context_pointer, location) != 0)
        return (-1);
    return (0);
}

static void rl_history_json_shutdown(void *context_pointer)
{
    rl_history_plain_shutdown(context_pointer);
    return ;
}

static int rl_history_json_load(void *context_pointer)
{
    rl_history_path_context *path_context;
    json_group *group_head;
    json_group *history_group;
    json_item *item_pointer;

    path_context = static_cast<rl_history_path_context *>(context_pointer);
    if (path_context == ft_nullptr || path_context->path == ft_nullptr)
    {
        return (-1);
    }
    group_head = json_read_from_file(path_context->path);
    if (group_head == ft_nullptr)
        return (0);
    history_group = json_find_group(group_head, "history");
    if (history_group == ft_nullptr)
    {
        json_free_groups(group_head);
        return (0);
    }
    rl_clear_history();
    item_pointer = history_group->items;
    while (item_pointer != ft_nullptr)
    {
        if (item_pointer->value != ft_nullptr)
            rl_update_history(item_pointer->value);
        else
            rl_update_history("");
        item_pointer = item_pointer->next;
    }
    json_free_groups(group_head);
    return (0);
}

static int rl_history_json_save(void *context_pointer)
{
    rl_history_path_context *path_context;
    json_group *history_group;
    json_group *root_group;
    int history_index;

    path_context = static_cast<rl_history_path_context *>(context_pointer);
    if (path_context == ft_nullptr || path_context->path == ft_nullptr)
    {
        return (-1);
    }
    history_group = json_create_json_group("history");
    if (history_group == ft_nullptr)
    {
        return (-1);
    }
    history_index = 0;
    while (history_index < history_count)
    {
        const char *history_entry;
        char *key_string;
        json_item *item_pointer;

        history_entry = history[history_index];
        if (history_entry == ft_nullptr)
            history_entry = "";
        key_string = adv_itoa(history_index);
        if (key_string == ft_nullptr)
        {
            json_free_groups(history_group);
            return (-1);
        }
        item_pointer = json_create_item(key_string, history_entry);
        cma_free(key_string);
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(history_group);
            return (-1);
        }
        json_add_item_to_group(history_group, item_pointer);
        history_index += 1;
    }
    root_group = ft_nullptr;
    json_append_group(&root_group, history_group);
    int write_result = json_write_to_file(path_context->path, root_group);
    if (write_result != 0)
    {
        json_free_groups(root_group);
        return (-1);
    }
    json_free_groups(root_group);
    return (0);
}

#if SQLITE3_AVAILABLE
static int rl_history_sqlite_configure(void **context_pointer, const char *location)
{
    rl_history_sqlite_context *sqlite_context;
    sqlite3 *database_handle;
    char *error_message;

    if (context_pointer == ft_nullptr || location == ft_nullptr)
    {
        return (-1);
    }
    sqlite_context = static_cast<rl_history_sqlite_context *>(*context_pointer);
    if (sqlite_context == ft_nullptr)
    {
        sqlite_context = static_cast<rl_history_sqlite_context *>(cma_malloc(sizeof(*sqlite_context)));
        if (sqlite_context == ft_nullptr)
        {
            return (-1);
        }
        sqlite_context->path = ft_nullptr;
        sqlite_context->database = ft_nullptr;
        *context_pointer = sqlite_context;
    }
    if (rl_history_assign_path(&sqlite_context->path, location) != 0)
        return (-1);
    if (sqlite_context->database != ft_nullptr)
    {
        sqlite3_close(sqlite_context->database);
        sqlite_context->database = ft_nullptr;
    }
    if (sqlite3_open(sqlite_context->path, &database_handle) != SQLITE_OK)
    {
        if (database_handle != ft_nullptr)
            sqlite3_close(database_handle);
        return (-1);
    }
    error_message = ft_nullptr;
    if (sqlite3_exec(database_handle,
            "CREATE TABLE IF NOT EXISTS readline_history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "entry TEXT NOT NULL);",
            ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message != ft_nullptr)
            sqlite3_free(error_message);
        sqlite3_close(database_handle);
        return (-1);
    }
    if (error_message != ft_nullptr)
        sqlite3_free(error_message);
    sqlite_context->database = database_handle;
    return (0);
}

static void rl_history_sqlite_shutdown(void *context_pointer)
{
    rl_history_sqlite_context *sqlite_context;

    if (context_pointer == ft_nullptr)
        return ;
    sqlite_context = static_cast<rl_history_sqlite_context *>(context_pointer);
    if (sqlite_context->database != ft_nullptr)
        sqlite3_close(sqlite_context->database);
    if (sqlite_context->path != ft_nullptr)
        cma_free(sqlite_context->path);
    cma_free(sqlite_context);
    return ;
}

static int rl_history_sqlite_load(void *context_pointer)
{
    rl_history_sqlite_context *sqlite_context;
    sqlite3_stmt *statement_handle;
    int prepare_result;

    sqlite_context = static_cast<rl_history_sqlite_context *>(context_pointer);
    if (sqlite_context == ft_nullptr || sqlite_context->database == ft_nullptr)
    {
        return (-1);
    }
    prepare_result = sqlite3_prepare_v2(sqlite_context->database,
            "SELECT entry FROM readline_history ORDER BY id ASC;",
            -1, &statement_handle, ft_nullptr);
    if (prepare_result != SQLITE_OK)
    {
        return (-1);
    }
    rl_clear_history();
    while (1)
    {
        int step_result;

        step_result = sqlite3_step(statement_handle);
        if (step_result == SQLITE_DONE)
            break ;
        if (step_result == SQLITE_ROW)
        {
            const unsigned char *text_pointer;
            const char *entry_text;

            text_pointer = sqlite3_column_text(statement_handle, 0);
            if (text_pointer == ft_nullptr)
                entry_text = "";
            else
                entry_text = reinterpret_cast<const char *>(text_pointer);
            rl_update_history(entry_text);
            continue ;
        }
        sqlite3_finalize(statement_handle);
        return (-1);
    }
    sqlite3_finalize(statement_handle);
    return (0);
}

static int rl_history_sqlite_save(void *context_pointer)
{
    rl_history_sqlite_context *sqlite_context;
    char *error_message;
    sqlite3_stmt *insert_statement;
    int history_index;

    sqlite_context = static_cast<rl_history_sqlite_context *>(context_pointer);
    if (sqlite_context == ft_nullptr || sqlite_context->database == ft_nullptr)
    {
        return (-1);
    }
    error_message = ft_nullptr;
    if (sqlite3_exec(sqlite_context->database, "BEGIN IMMEDIATE TRANSACTION;",
            ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message != ft_nullptr)
            sqlite3_free(error_message);
        return (-1);
    }
    if (error_message != ft_nullptr)
        sqlite3_free(error_message);
    error_message = ft_nullptr;
    if (sqlite3_exec(sqlite_context->database,
            "DELETE FROM readline_history;",
            ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message != ft_nullptr)
            sqlite3_free(error_message);
        sqlite3_exec(sqlite_context->database, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        return (-1);
    }
    if (error_message != ft_nullptr)
        sqlite3_free(error_message);
    if (sqlite3_prepare_v2(sqlite_context->database,
            "INSERT INTO readline_history(entry) VALUES (?1);",
            -1, &insert_statement, ft_nullptr) != SQLITE_OK)
    {
        sqlite3_exec(sqlite_context->database, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        return (-1);
    }
    history_index = 0;
    while (history_index < history_count)
    {
        const char *history_entry;
        int bind_result;
        int step_result;

        history_entry = history[history_index];
        if (history_entry == ft_nullptr)
            history_entry = "";
        sqlite3_reset(insert_statement);
        sqlite3_clear_bindings(insert_statement);
        bind_result = sqlite3_bind_text(insert_statement, 1, history_entry,
                -1, SQLITE_TRANSIENT);
        if (bind_result != SQLITE_OK)
        {
            sqlite3_finalize(insert_statement);
            sqlite3_exec(sqlite_context->database, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
            return (-1);
        }
        step_result = sqlite3_step(insert_statement);
        if (step_result != SQLITE_DONE)
        {
            sqlite3_finalize(insert_statement);
            sqlite3_exec(sqlite_context->database, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
            return (-1);
        }
        history_index += 1;
    }
    sqlite3_finalize(insert_statement);
    if (sqlite3_exec(sqlite_context->database, "COMMIT;",
            ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message != ft_nullptr)
            sqlite3_free(error_message);
        sqlite3_exec(sqlite_context->database, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        return (-1);
    }
    if (error_message != ft_nullptr)
        sqlite3_free(error_message);
    return (0);
}

static rl_history_backend g_history_sqlite_backend = {
    "sqlite",
    rl_history_sqlite_configure,
    rl_history_sqlite_shutdown,
    rl_history_sqlite_load,
    rl_history_sqlite_save
};
#endif

static rl_history_backend g_history_plain_backend = {
    "plain-text",
    rl_history_plain_configure,
    rl_history_plain_shutdown,
    rl_history_plain_load,
    rl_history_plain_save
};

static rl_history_backend g_history_json_backend = {
    "json",
    rl_history_json_configure,
    rl_history_json_shutdown,
    rl_history_json_load,
    rl_history_json_save
};

static const rl_history_backend *g_history_backend_catalog[] = {
    &g_history_plain_backend,
    &g_history_json_backend,
#if SQLITE3_AVAILABLE
    &g_history_sqlite_backend
#endif
};

static const rl_history_backend *rl_history_find_backend(const char *backend_name)
{
    size_t backend_index;

    if (backend_name == ft_nullptr)
    {
        return (ft_nullptr);
    }
    backend_index = 0;
    while (backend_index < sizeof(g_history_backend_catalog) / sizeof(g_history_backend_catalog[0]))
    {
        const rl_history_backend *candidate_backend;

        candidate_backend = g_history_backend_catalog[backend_index];
        if (candidate_backend != ft_nullptr
            && candidate_backend->name != ft_nullptr
            && ft_strcmp(candidate_backend->name, backend_name) == 0)
        {
            return (candidate_backend);
        }
        backend_index += 1;
    }
    return (ft_nullptr);
}

static int rl_history_configure_backend_locked(const rl_history_backend *backend,
        const char *location)
{
    const rl_history_backend *existing_backend;
    void *existing_context;
    void *working_context;

    if (backend == ft_nullptr)
    {
        return (-1);
    }
    existing_backend = g_history_backend_state.backend;
    existing_context = g_history_backend_state.backend_context;
    working_context = existing_context;
    if (backend != existing_backend)
        working_context = ft_nullptr;
    if (backend->configure != ft_nullptr)
    {
        if (backend->configure(&working_context, location) != 0)
        {
            if (backend != existing_backend && working_context != ft_nullptr
                && backend->shutdown != ft_nullptr)
                backend->shutdown(working_context);
            return (-1);
        }
    }
    if (backend != existing_backend)
    {
        if (existing_backend != ft_nullptr && existing_backend->shutdown != ft_nullptr)
            existing_backend->shutdown(existing_context);
    }
    g_history_backend_state.backend = backend;
    g_history_backend_state.backend_context = working_context;
    return (0);
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
            return (0);
        }
        index += 1;
    }
    if (g_key_binding_count >= RL_MAX_KEY_BINDINGS)
    {
        rl_customization_unlock(lock_acquired);
        return (-1);
    }
    g_key_bindings[g_key_binding_count].key = key;
    g_key_bindings[g_key_binding_count].callback = callback;
    g_key_bindings[g_key_binding_count].user_data = user_data;
    g_key_binding_count += 1;
    rl_customization_unlock(lock_acquired);
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
            return (0);
        }
        index += 1;
    }
    rl_customization_unlock(lock_acquired);
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
        return (0);
    }
    result = callback(state, prompt, user_data);
    if (result != 0)
        return (-1);
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
        return (-1);
    }
    text_length = ft_strlen(text);
    if (text_length == 0)
    {
        return (0);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    suffix_length = ft_strlen(&state->buffer[state->pos]);
    total_length = static_cast<long long>(state->pos) + static_cast<long long>(text_length) + static_cast<long long>(suffix_length);
    required_size = static_cast<int>(total_length) + 1;
    if (total_length > static_cast<long long>(FT_INT32_MAX))
    {
        result = -1;
        goto cleanup;
    }
    if (required_size > state->bufsize)
    {
        int new_bufsize;

        new_bufsize = state->bufsize;
        if (new_bufsize <= 0)
            new_bufsize = 1;
        while (required_size > new_bufsize)
        {
            if (new_bufsize > FT_INT32_MAX / 2)
            {
                result = -1;
                goto cleanup;
            }
            new_bufsize *= 2;
        }
        int resize_error = rl_resize_buffer(&state->buffer, &state->bufsize, new_bufsize);
        if (resize_error != FT_ERR_SUCCESS)
        {
            result = -1;
            goto cleanup;
        }
    }
    ft_memmove(&state->buffer[state->pos + text_length], &state->buffer[state->pos], static_cast<size_t>(suffix_length) + 1);
    ft_memcpy(&state->buffer[state->pos], text, static_cast<size_t>(text_length));
    state->pos += text_length;
    result = 0;
cleanup:
    rl_state_unlock(state, lock_acquired);
    return (result);
}

int rl_state_delete_previous_grapheme(readline_state_t *state)
{
    bool lock_acquired;
    int result;

    if (state == ft_nullptr)
    {
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
    return (result);
}

int rl_state_set_cursor(readline_state_t *state, int new_position)
{
    bool lock_acquired;
    int buffer_length;

    if (state == ft_nullptr)
    {
        return (-1);
    }
    if (new_position < 0)
    {
        return (-1);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    buffer_length = ft_strlen(state->buffer);
    if (new_position > buffer_length)
    {
        rl_state_unlock(state, lock_acquired);
        return (-1);
    }
    state->pos = new_position;
    rl_state_unlock(state, lock_acquired);
    return (0);
}

int rl_state_get_cursor(readline_state_t *state, int *out_position)
{
    bool lock_acquired;

    if (state == ft_nullptr || out_position == ft_nullptr)
    {
        return (-1);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    *out_position = state->pos;
    rl_state_unlock(state, lock_acquired);
    return (0);
}

int rl_state_get_buffer(readline_state_t *state, const char **out_buffer)
{
    bool lock_acquired;

    if (state == ft_nullptr || out_buffer == ft_nullptr)
    {
        return (-1);
    }
    lock_acquired = false;
    if (rl_state_lock(state, &lock_acquired) != 0)
        return (-1);
    *out_buffer = state->buffer;
    rl_state_unlock(state, lock_acquired);
    return (0);
}

int rl_state_refresh_display(readline_state_t *state, const char *prompt)
{
    bool lock_acquired;
    int result;
    int columns_after_cursor;

    if (state == ft_nullptr || prompt == ft_nullptr)
    {
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
    return (0);
}

int rl_completion_add_candidate(const char *candidate)
{
    bool lock_acquired;
    char *duplicated;

    if (candidate == ft_nullptr)
    {
        return (-1);
    }
    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    if (g_dynamic_suggestion_count >= MAX_SUGGESTIONS)
    {
        rl_customization_unlock(lock_acquired);
        return (-1);
    }
    duplicated = adv_strdup(candidate);
    if (duplicated == ft_nullptr)
    {
        rl_customization_unlock(lock_acquired);
        return (-1);
    }
    g_dynamic_suggestions[g_dynamic_suggestion_count] = duplicated;
    g_dynamic_suggestion_count += 1;
    rl_customization_unlock(lock_acquired);
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
        return (0);
    }
    callback_result = callback(buffer, cursor_position, prefix, user_data);
    if (callback_result != 0)
        return (-1);
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
        return (ft_nullptr);
    }
    result = g_dynamic_suggestions[index];
    rl_customization_unlock(lock_acquired);
    return (result);
}

int rl_history_set_backend(const char *backend_name, const char *location)
{
    bool lock_acquired;
    const rl_history_backend *backend_pointer;
    int configure_result;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    backend_pointer = rl_history_find_backend(backend_name);
    if (backend_pointer == ft_nullptr)
    {
        rl_customization_unlock(lock_acquired);
        return (-1);
    }
    configure_result = rl_history_configure_backend_locked(backend_pointer, location);
    rl_customization_unlock(lock_acquired);
    if (configure_result != 0)
        return (-1);
    return (0);
}

const char *rl_history_get_backend(void)
{
    bool lock_acquired;
    const char *backend_name;

    lock_acquired = false;
    backend_name = ft_nullptr;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (ft_nullptr);
    if (g_history_backend_state.backend == ft_nullptr
        || g_history_backend_state.backend->name == ft_nullptr)
    {
        rl_customization_unlock(lock_acquired);
        return (ft_nullptr);
    }
    backend_name = g_history_backend_state.backend->name;
    rl_customization_unlock(lock_acquired);
    return (backend_name);
}

int rl_history_set_storage_path(const char *file_path)
{
    return (rl_history_set_backend("plain-text", file_path));
}

int rl_history_enable_auto_save(bool enabled)
{
    bool lock_acquired;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    g_history_backend_state.auto_save_enabled = enabled;
    rl_customization_unlock(lock_acquired);
    return (0);
}

int rl_history_load(void)
{
    bool lock_acquired;
    const rl_history_backend *backend_pointer;
    void *backend_context;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    backend_pointer = g_history_backend_state.backend;
    backend_context = g_history_backend_state.backend_context;
    rl_customization_unlock(lock_acquired);
    if (backend_pointer == ft_nullptr || backend_pointer->load == ft_nullptr)
    {
        return (-1);
    }
    if (backend_pointer->load(backend_context) != 0)
        return (-1);
    return (0);
}

int rl_history_save(void)
{
    bool lock_acquired;
    const rl_history_backend *backend_pointer;
    void *backend_context;

    lock_acquired = false;
    if (rl_customization_lock(&lock_acquired) != 0)
        return (-1);
    backend_pointer = g_history_backend_state.backend;
    backend_context = g_history_backend_state.backend_context;
    rl_customization_unlock(lock_acquired);
    if (backend_pointer == ft_nullptr || backend_pointer->save == ft_nullptr)
    {
        return (-1);
    }
    if (backend_pointer->save(backend_context) != 0)
        return (-1);
    return (0);
}

void rl_history_notify_updated(void)
{
    bool lock_acquired;
    bool auto_save_enabled;
    const rl_history_backend *backend_pointer;

    lock_acquired = false;
    auto_save_enabled = false;
    backend_pointer = ft_nullptr;
    if (rl_customization_lock(&lock_acquired) != 0)
        return ;
    auto_save_enabled = g_history_backend_state.auto_save_enabled;
    backend_pointer = g_history_backend_state.backend;
    rl_customization_unlock(lock_acquired);
    if (!auto_save_enabled || backend_pointer == ft_nullptr
        || backend_pointer->save == ft_nullptr)
        return ;
    (void)rl_history_save();
    return ;
}
