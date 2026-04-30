#pragma once

#include "../DUMB/controls.hpp"
#include "../DUMB/render_window.hpp"
#include <chrono>

struct demo_input_state
{
    ft_bool move_forward_down;
    ft_bool move_backward_down;
    ft_bool turn_left_down;
    ft_bool turn_right_down;
    ft_bool menu_up_pressed;
    ft_bool menu_down_pressed;
    ft_bool menu_left_pressed;
    ft_bool menu_right_pressed;
    ft_bool confirm_pressed;
    ft_bool back_pressed;
};

struct demo_score_entry
{
    char        name[4];
    uint32_t    elapsed_milliseconds;
};

struct demo_leaderboard
{
    demo_score_entry entries[5];
    uint32_t         entry_count;
};

struct demo_level
{
    struct demo_light
    {
        double      position_x;
        double      position_y;
        double      radius;
        uint32_t    color;
        uint32_t    intensity;
    };

    const char  *name;
    const char  *tiles;
    int32_t     width;
    int32_t     height;
    double      start_position_x;
    double      start_position_y;
    double      start_direction_x;
    double      start_direction_y;
    const demo_light *lights;
    uint32_t    light_count;
};

struct demo_game_state
{
    static const uint32_t FPS_HISTORY_CAPACITY = 4096U;

    int32_t     mode;
    int32_t     selected_menu_index;
    int32_t     current_level_index;
    int32_t     pending_level_index;
    double      player_position_x;
    double      player_position_y;
    double      player_direction_x;
    double      player_direction_y;
    double      camera_plane_x;
    double      camera_plane_y;
    char        pending_name[4];
    uint32_t    pending_name_index;
    uint32_t    total_elapsed_milliseconds;
    uint32_t    displayed_fps;
    uint32_t    average_fps_10_seconds;
    uint32_t    fps_frame_counter;
    uint32_t    fps_history_write_index;
    uint32_t    fps_history_count;
    ft_bool     should_quit;
    std::chrono::steady_clock::time_point run_start_time;
    std::chrono::steady_clock::time_point fps_sample_start_time;
    uint32_t    fps_history_milliseconds[FPS_HISTORY_CAPACITY];
};

enum demo_mode
{
    DEMO_MODE_MENU = 0,
    DEMO_MODE_PLAYING = 1,
    DEMO_MODE_LEVEL_CLEAR = 2,
    DEMO_MODE_LEADERBOARD = 3,
    DEMO_MODE_NAME_ENTRY = 4
};

extern const demo_level g_demo_levels[];
extern const uint32_t   g_demo_level_count;

ft_bool demo_level_is_wall(const demo_level &level, int32_t tile_x, int32_t tile_y);
char    demo_level_tile_at(const demo_level &level, int32_t tile_x, int32_t tile_y);
int32_t demo_validate_levels(void);

void    demo_leaderboard_reset(demo_leaderboard *leaderboard);
void    demo_leaderboard_insert(demo_leaderboard *leaderboard, const char *name,
            uint32_t elapsed_milliseconds);
void    demo_leaderboard_save(const demo_leaderboard *leaderboard);
void    demo_leaderboard_load(demo_leaderboard *leaderboard);

void    demo_init_game_state(demo_game_state *game_state);
void    demo_poll_input(demo_input_state *input_state);
void    demo_update(demo_game_state *game_state, const demo_input_state *input_state,
            demo_leaderboard *leaderboard, double frame_delta_seconds);

void    demo_draw_frame(ft_render_window &render_window,
            const demo_game_state &game_state,
            const demo_leaderboard &leaderboard);
void    demo_choose_window_size(ft_render_window_desc *window_desc);

int32_t demo_run(void);
