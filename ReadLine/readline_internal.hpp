#ifndef READLINE_INTERNAL_HPP
#define READLINE_INTERNAL_HPP

#include "../CPP_class/file.hpp"
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

struct terminal_dimensions {
    unsigned short rows;
    unsigned short cols;
    unsigned short x_pixels;
    unsigned short y_pixels;
};

typedef struct {
    char	*buffer;
    int		bufsize;
    int		pos;
    int		prev_buffer_length;
    int		history_index;
    int		in_completion_mode;
    int		current_match_count;
    int		current_match_index;
    int		word_start;
    char	*current_matches[MAX_SUGGESTIONS];
	ft_file	error_file;
} readline_state_t;

//Initialize memory
int		rl_initialize_state(readline_state_t *state);

//Raw mode functions
void	rl_disable_raw_mode();
int		rl_enable_raw_mode();

//Buffer Management Functions
int		rl_clear_line(const char *prompt, const char *buffer);
char	*rl_resize_buffer(char *old_buffer, int current_size, int new_size);

//Input Handling Functions
int		rl_handle_escape_sequence(readline_state_t *state, const char *prompt);
int		rl_handle_backspace(readline_state_t *state, const char *prompt);
int		rl_handle_tab_completion(readline_state_t *state, const char *prompt);
int		rl_handle_printable_char(readline_state_t *state, char c, const char *prompt);

//Utilities
char	rl_read_key();
int		rl_get_terminal_width(void);
int		rl_read_escape_sequence(char seq[2]);
void	rl_update_history(const char *buffer);
void	rl_reset_completion_mode(readline_state_t *state);

#endif
