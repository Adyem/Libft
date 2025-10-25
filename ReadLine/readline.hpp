#ifndef READLINE_HPP
# define READLINE_HPP

struct s_readline_state;

typedef int (*t_rl_key_binding_callback)(s_readline_state *state, const char *prompt, void *user_data);
typedef int (*t_rl_completion_callback)(const char *buffer, int cursor_position,
    const char *prefix, void *user_data);

void rl_clear_history();
void rl_add_suggestion(const char *word);
void rl_clear_suggestions();

int rl_bind_key(int key, t_rl_key_binding_callback callback, void *user_data);
int rl_unbind_key(int key);

int rl_state_insert_text(s_readline_state *state, const char *text);
int rl_state_delete_previous_grapheme(s_readline_state *state);
int rl_state_set_cursor(s_readline_state *state, int new_position);
int rl_state_get_cursor(s_readline_state *state, int *out_position);
int rl_state_get_buffer(s_readline_state *state, const char **out_buffer);
int rl_state_refresh_display(s_readline_state *state, const char *prompt);

int rl_set_completion_callback(t_rl_completion_callback callback, void *user_data);
int rl_completion_add_candidate(const char *candidate);

int rl_history_set_backend(const char *backend_name, const char *location);
const char *rl_history_get_backend(void);
int rl_history_set_storage_path(const char *file_path);
int rl_history_enable_auto_save(bool enabled);
int rl_history_load(void);
int rl_history_save(void);
int rl_history_search(const char *query, int start_index,
        bool search_backward, int *match_index);

char *rl_readline(const char *prompt) __attribute__((warn_unused_result));

#endif
