#ifndef READLINE_INTERNAL_HPP
#define READLINE_INTERNAL_HPP

#include "../CPP_class/class_file.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <unistd.h>
#include <stdlib.h>

#ifndef DEBUG
# define DEBUG 0
#endif

#define MAX_HISTORY 1000
#define MAX_SUGGESTIONS 1000
#define INITIAL_BUFFER_SIZE 1024

extern struct termios orig_termios;

extern char *history[MAX_HISTORY];
extern int history_count;

extern char *suggestions[MAX_SUGGESTIONS];
extern int suggestion_count;

class pt_mutex;

struct terminal_dimensions {
    unsigned short rows = 0;
    unsigned short cols = 0;
    unsigned short x_pixels = 0;
    unsigned short y_pixels = 0;
    bool dimensions_valid = false;
    pt_mutex *mutex = ft_nullptr;
    bool thread_safe_enabled = false;
};

struct s_readline_state
{
    char        *buffer;
    int            bufsize;
    int            pos;
    int            prev_buffer_length;
    int            display_pos;
    int            prev_display_columns;
    int            history_index;
    int            in_completion_mode;
    int            current_match_count;
    int            current_match_index;
    int            word_start;
    char        *current_matches[MAX_SUGGESTIONS];
    ft_file        error_file;
    pt_mutex    *mutex = ft_nullptr;
    bool        thread_safe_enabled = false;
};

typedef struct s_readline_state readline_state_t;

typedef int (*t_rl_key_binding_callback)(readline_state_t *state, const char *prompt, void *user_data);

typedef int (*t_rl_completion_callback)(const char *buffer, int cursor_position,
    const char *prefix, void *user_data);

int        rl_initialize_state(readline_state_t *state);

void    rl_disable_raw_mode();
int        rl_enable_raw_mode();

int        rl_clear_line(const char *prompt, const char *buffer);
char    *rl_resize_buffer(char **buffer_pointer, int *current_size_pointer, int new_size);
int        rl_handle_escape_sequence(readline_state_t *state, const char *prompt);
int        rl_handle_backspace(readline_state_t *state, const char *prompt);
int        rl_handle_tab_completion(readline_state_t *state, const char *prompt);
int        rl_handle_printable_char(readline_state_t *state, char c, const char *prompt);
int        rl_update_display_metrics(readline_state_t *state);
int        rl_utf8_compute_columns(const char *string, int *columns);
int        rl_utf8_find_previous_grapheme(const char *buffer, int cursor_pos,
            int *start_byte, int *end_byte, int *display_width);
int        rl_utf8_find_next_grapheme(const char *buffer, int cursor_pos,
            int *start_byte, int *end_byte, int *display_width);

int        rl_read_key(void);
int        rl_get_terminal_width(void);
int        rl_read_escape_sequence(char seq[2]);
void    rl_update_history(const char *buffer);
void    rl_reset_completion_mode(readline_state_t *state);

int        rl_dispatch_custom_key(readline_state_t *state, const char *prompt, int key);

int        rl_bind_key(int key, t_rl_key_binding_callback callback, void *user_data);
int        rl_unbind_key(int key);

int        rl_state_insert_text(readline_state_t *state, const char *text);
int        rl_state_delete_previous_grapheme(readline_state_t *state);
int        rl_state_set_cursor(readline_state_t *state, int new_position);
int        rl_state_get_cursor(readline_state_t *state, int *out_position);
int        rl_state_get_buffer(readline_state_t *state, const char **out_buffer);
int        rl_state_refresh_display(readline_state_t *state, const char *prompt);

int        rl_set_completion_callback(t_rl_completion_callback callback, void *user_data);
int        rl_completion_add_candidate(const char *candidate);
void    rl_completion_reset_dynamic_matches(void);
int        rl_completion_prepare_candidates(const char *buffer, int cursor_position,
            const char *prefix, int prefix_length);
int        rl_completion_get_dynamic_count(void);
char    *rl_completion_get_dynamic_match(int index);

int        rl_history_set_backend(const char *backend_name, const char *location);
const char    *rl_history_get_backend(void);
int        rl_history_set_storage_path(const char *file_path);
int        rl_history_enable_auto_save(bool enabled);
int        rl_history_load(void);
int        rl_history_save(void);
void    rl_history_notify_updated(void);
int        rl_history_search(const char *query, int start_index,
            bool search_backward, int *match_index);

int        rl_state_prepare_thread_safety(readline_state_t *state);
void    rl_state_teardown_thread_safety(readline_state_t *state);
int        rl_state_lock(readline_state_t *state, bool *lock_acquired);
void    rl_state_unlock(readline_state_t *state, bool lock_acquired);

int        rl_terminal_dimensions_prepare_thread_safety(terminal_dimensions *dimensions);
void    rl_terminal_dimensions_teardown_thread_safety(terminal_dimensions *dimensions);
int        rl_terminal_dimensions_lock(terminal_dimensions *dimensions, bool *lock_acquired);
void    rl_terminal_dimensions_unlock(terminal_dimensions *dimensions, bool lock_acquired);
int        rl_terminal_dimensions_refresh(terminal_dimensions *dimensions);
int        rl_terminal_dimensions_get(terminal_dimensions *dimensions,
            unsigned short *rows, unsigned short *cols,
            unsigned short *x_pixels, unsigned short *y_pixels,
            bool *dimensions_valid);

#endif
