#ifndef READLINE_HPP
# define READLINE_HPP

#include "../Errno/errno.hpp"

struct s_readline_state;

typedef int32_t (*t_rl_key_binding_callback)(s_readline_state *state, const char *prompt, void *user_data);
typedef int32_t (*t_rl_completion_callback)(const char *buffer, int32_t cursor_position,
    const char *prefix, void *user_data);

void rl_clear_history();
void rl_add_suggestion(const char *word);
void rl_clear_suggestions();

int32_t rl_bind_key(int32_t key, t_rl_key_binding_callback callback, void *user_data);
int32_t rl_unbind_key(int32_t key);

int32_t rl_state_insert_text(s_readline_state *state, const char *text);
int32_t rl_state_delete_previous_grapheme(s_readline_state *state);
int32_t rl_state_set_cursor(s_readline_state *state, int32_t new_position);
int32_t rl_state_get_cursor(s_readline_state *state, int32_t *out_position);
int32_t rl_state_get_buffer(s_readline_state *state, const char **out_buffer);
int32_t rl_state_refresh_display(s_readline_state *state, const char *prompt);

int32_t rl_set_completion_callback(t_rl_completion_callback callback, void *user_data);
int32_t rl_completion_add_candidate(const char *candidate);

int32_t rl_history_set_backend(const char *backend_name, const char *location);
const char *rl_history_get_backend(void);
int32_t rl_history_set_storage_path(const char *file_path);
int32_t rl_history_enable_auto_save(ft_bool enabled);
int32_t rl_history_load(void);
int32_t rl_history_save(void);
int32_t rl_history_search(const char *query, int32_t start_index,
        ft_bool search_backward, int32_t *match_index);

char *rl_readline(const char *prompt) __attribute__((warn_unused_result));

#endif
