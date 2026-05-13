#include "demo_game.hpp"

#include <cmath>
#include <cstring>
#include <thread>

static const int64_t DEMO_TARGET_FRAME_MICROSECONDS = 1111;
static const int64_t DEMO_FPS_SAMPLE_MILLISECONDS = 250;
static const int64_t DEMO_FPS_AVERAGE_MILLISECONDS = 10000;
static const double  DEMO_NOMINAL_FRAME_SECONDS = 1.0 / 900.0;
static const double  DEMO_MAX_FRAME_SECONDS = 0.05;
static void demo_reset_name_entry(demo_game_state *game_state)
{
    game_state->pending_name[0] = 'A';
    game_state->pending_name[1] = 'A';
    game_state->pending_name[2] = 'A';
    game_state->pending_name[3] = '\0';
    game_state->pending_name_index = 0U;
    return ;
}

static void demo_prepare_level(demo_game_state *game_state, int32_t level_index)
{
    const demo_level *level;

    level = &g_demo_levels[level_index];
    game_state->current_level_index = level_index;
    game_state->player_position_x = level->start_position_x;
    game_state->player_position_y = level->start_position_y;
    game_state->player_floor_height = 0.0;
    game_state->stair_progress = 0.0;
    game_state->player_direction_x = level->start_direction_x;
    game_state->player_direction_y = level->start_direction_y;
    game_state->camera_plane_x = -level->start_direction_y * 0.66;
    game_state->camera_plane_y = level->start_direction_x * 0.66;
    game_state->mode = DEMO_MODE_PLAYING;
    return ;
}

static void demo_reset_run(demo_game_state *game_state)
{
    game_state->selected_menu_index = 0;
    game_state->pending_level_index = 0;
    game_state->total_elapsed_milliseconds = 0U;
    game_state->displayed_fps = 0U;
    game_state->average_fps_10_seconds = 0U;
    game_state->fps_frame_counter = 0U;
    game_state->fps_history_write_index = 0U;
    game_state->fps_history_count = 0U;
    game_state->run_start_time = std::chrono::steady_clock::now();
    game_state->fps_sample_start_time = game_state->run_start_time;
    demo_reset_name_entry(game_state);
    demo_prepare_level(game_state, 0);
    return ;
}

void demo_init_game_state(demo_game_state *game_state)
{
    game_state->mode = DEMO_MODE_MENU;
    game_state->selected_menu_index = 0;
    game_state->current_level_index = 0;
    game_state->pending_level_index = 0;
    game_state->player_position_x = 1.5;
    game_state->player_position_y = 1.5;
    game_state->player_floor_height = 0.0;
    game_state->stair_progress = 0.0;
    game_state->player_direction_x = 1.0;
    game_state->player_direction_y = 0.0;
    game_state->camera_plane_x = 0.0;
    game_state->camera_plane_y = 0.66;
    game_state->should_quit = FT_FALSE;
    game_state->total_elapsed_milliseconds = 0U;
    game_state->displayed_fps = 0U;
    game_state->average_fps_10_seconds = 0U;
    game_state->fps_frame_counter = 0U;
    game_state->fps_history_write_index = 0U;
    game_state->fps_history_count = 0U;
    game_state->run_start_time = std::chrono::steady_clock::now();
    game_state->fps_sample_start_time = game_state->run_start_time;
    demo_reset_name_entry(game_state);
    return ;
}

static void demo_update_fps_counter(demo_game_state *game_state)
{
    std::chrono::steady_clock::time_point current_time;
    int64_t elapsed_milliseconds;
    int64_t history_age_milliseconds;
    uint32_t calculated_fps;
    uint32_t current_run_milliseconds;
    uint32_t cutoff_run_milliseconds;
    uint32_t oldest_index;

    current_time = std::chrono::steady_clock::now();
    game_state->fps_frame_counter = game_state->fps_frame_counter + 1U;
    elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time - game_state->run_start_time).count();
    if (elapsed_milliseconds < 0)
    {
        elapsed_milliseconds = 0;
    }
    current_run_milliseconds = static_cast<uint32_t>(elapsed_milliseconds);
    game_state->fps_history_milliseconds[game_state->fps_history_write_index]
        = current_run_milliseconds;
    game_state->fps_history_write_index
        = (game_state->fps_history_write_index + 1U)
        % demo_game_state::FPS_HISTORY_CAPACITY;
    if (game_state->fps_history_count < demo_game_state::FPS_HISTORY_CAPACITY)
    {
        game_state->fps_history_count = game_state->fps_history_count + 1U;
    }
    if (current_run_milliseconds > static_cast<uint32_t>(DEMO_FPS_AVERAGE_MILLISECONDS))
    {
        cutoff_run_milliseconds = current_run_milliseconds
            - static_cast<uint32_t>(DEMO_FPS_AVERAGE_MILLISECONDS);
    }
    else
    {
        cutoff_run_milliseconds = 0U;
    }
    while (game_state->fps_history_count > 0U)
    {
        oldest_index = (game_state->fps_history_write_index
            + demo_game_state::FPS_HISTORY_CAPACITY
            - game_state->fps_history_count)
            % demo_game_state::FPS_HISTORY_CAPACITY;
        if (game_state->fps_history_milliseconds[oldest_index]
            >= cutoff_run_milliseconds)
        {
            break ;
        }
        game_state->fps_history_count = game_state->fps_history_count - 1U;
    }
    if (game_state->fps_history_count == 0U)
    {
        game_state->average_fps_10_seconds = 0U;
    }
    else
    {
        oldest_index = (game_state->fps_history_write_index
            + demo_game_state::FPS_HISTORY_CAPACITY
            - game_state->fps_history_count)
            % demo_game_state::FPS_HISTORY_CAPACITY;
        history_age_milliseconds = static_cast<int64_t>(current_run_milliseconds)
            - static_cast<int64_t>(
                game_state->fps_history_milliseconds[oldest_index]);
        if (history_age_milliseconds <= 0)
        {
            game_state->average_fps_10_seconds = game_state->displayed_fps;
        }
        else
        {
            game_state->average_fps_10_seconds = static_cast<uint32_t>(
                (static_cast<uint64_t>(game_state->fps_history_count) * 1000U)
                / static_cast<uint64_t>(history_age_milliseconds));
        }
    }
    elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time - game_state->fps_sample_start_time).count();
    if (elapsed_milliseconds < DEMO_FPS_SAMPLE_MILLISECONDS)
    {
        return ;
    }
    if (elapsed_milliseconds <= 0)
    {
        elapsed_milliseconds = 1;
    }
    calculated_fps = static_cast<uint32_t>(
        (static_cast<uint64_t>(game_state->fps_frame_counter) * 1000U)
        / static_cast<uint64_t>(elapsed_milliseconds));
    game_state->displayed_fps = calculated_fps;
    game_state->fps_frame_counter = 0U;
    game_state->fps_sample_start_time = current_time;
    return ;
}

void demo_poll_input(demo_input_state *input_state)
{
    ft_dumb_controls_poll();
    input_state->move_forward_down = ft_dumb_control_is_down(FT_DUMB_CONTROL_UP);
    input_state->move_backward_down = ft_dumb_control_is_down(FT_DUMB_CONTROL_DOWN);
    input_state->turn_left_down = ft_dumb_control_is_down(FT_DUMB_CONTROL_LEFT);
    input_state->turn_right_down = ft_dumb_control_is_down(FT_DUMB_CONTROL_RIGHT);
    input_state->menu_up_pressed = ft_dumb_control_was_pressed(FT_DUMB_CONTROL_UP);
    input_state->menu_down_pressed = ft_dumb_control_was_pressed(FT_DUMB_CONTROL_DOWN);
    input_state->menu_left_pressed = ft_dumb_control_was_pressed(FT_DUMB_CONTROL_LEFT);
    input_state->menu_right_pressed = ft_dumb_control_was_pressed(FT_DUMB_CONTROL_RIGHT);
    input_state->confirm_pressed = ft_dumb_control_was_pressed(FT_DUMB_CONTROL_CONFIRM);
    input_state->back_pressed = ft_dumb_control_was_pressed(FT_DUMB_CONTROL_BACK);
    return ;
}

static void demo_finish_run(demo_game_state *game_state, demo_leaderboard *leaderboard)
{
    std::chrono::steady_clock::time_point finish_time;
    int64_t elapsed_milliseconds;

    finish_time = std::chrono::steady_clock::now();
    elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        finish_time - game_state->run_start_time).count();
    if (elapsed_milliseconds < 0)
    {
        elapsed_milliseconds = 0;
    }
    game_state->total_elapsed_milliseconds
        = static_cast<uint32_t>(elapsed_milliseconds);
    demo_leaderboard_insert(leaderboard, game_state->pending_name,
        game_state->total_elapsed_milliseconds);
    demo_leaderboard_save(leaderboard);
    game_state->mode = DEMO_MODE_LEADERBOARD;
    game_state->selected_menu_index = 0;
    return ;
}

static void demo_handle_menu(demo_game_state *game_state,
    const demo_input_state *input_state)
{
    if (input_state->menu_up_pressed == FT_TRUE)
    {
        game_state->selected_menu_index = game_state->selected_menu_index - 1;
        if (game_state->selected_menu_index < 0)
        {
            game_state->selected_menu_index = 2;
        }
    }
    if (input_state->menu_down_pressed == FT_TRUE)
    {
        game_state->selected_menu_index = game_state->selected_menu_index + 1;
        if (game_state->selected_menu_index > 2)
        {
            game_state->selected_menu_index = 0;
        }
    }
    if (input_state->confirm_pressed == FT_TRUE)
    {
        if (game_state->selected_menu_index == 0)
        {
            demo_reset_run(game_state);
        }
        else if (game_state->selected_menu_index == 1)
        {
            game_state->mode = DEMO_MODE_LEADERBOARD;
        }
        else
        {
            game_state->should_quit = FT_TRUE;
        }
    }
    return ;
}

static void demo_rotate_player(demo_game_state *game_state, double angle_radians)
{
    double previous_direction_x;
    double previous_plane_x;
    double cosine_angle;
    double sine_angle;

    cosine_angle = std::cos(angle_radians);
    sine_angle = std::sin(angle_radians);
    previous_direction_x = game_state->player_direction_x;
    previous_plane_x = game_state->camera_plane_x;
    game_state->player_direction_x = (game_state->player_direction_x * cosine_angle)
        - (game_state->player_direction_y * sine_angle);
    game_state->player_direction_y = (previous_direction_x * sine_angle)
        + (game_state->player_direction_y * cosine_angle);
    game_state->camera_plane_x = (game_state->camera_plane_x * cosine_angle)
        - (game_state->camera_plane_y * sine_angle);
    game_state->camera_plane_y = (previous_plane_x * sine_angle)
        + (game_state->camera_plane_y * cosine_angle);
    return ;
}

static void demo_try_move_player(demo_game_state *game_state, double delta_x,
    double delta_y)
{
    const demo_level *level;
    double          target_x;
    double          target_y;
    int32_t         tile_x;
    int32_t         tile_y;

    level = &g_demo_levels[game_state->current_level_index];
    target_x = game_state->player_position_x + delta_x;
    target_y = game_state->player_position_y + delta_y;
    tile_x = static_cast<int32_t>(target_x);
    tile_y = static_cast<int32_t>(game_state->player_position_y);
    if (demo_level_is_wall(*level, tile_x, tile_y) == FT_FALSE)
    {
        game_state->player_position_x = target_x;
    }
    tile_x = static_cast<int32_t>(game_state->player_position_x);
    tile_y = static_cast<int32_t>(target_y);
    if (demo_level_is_wall(*level, tile_x, tile_y) == FT_FALSE)
    {
        game_state->player_position_y = target_y;
    }
    return ;
}

static double demo_tile_floor_height(char tile_value, double position_x)
{
    double tile_fraction;
    double quarter_fraction;
    double stair_height;

    if (tile_value >= 'a' && tile_value <= 'l')
    {
        tile_fraction = position_x - std::floor(position_x);
        quarter_fraction = std::floor(tile_fraction * 4.0) / 4.0;
        stair_height = (static_cast<double>(tile_value - 'a') + quarter_fraction)
            / 12.0;
        if (stair_height < 0.0)
            return (0.0);
        if (stair_height > 1.0)
            return (1.0);
        return (stair_height);
    }
    if (tile_value == 'U' || tile_value == 'E')
        return (1.0);
    return (0.0);
}

static void demo_update_player_stair_height(demo_game_state *game_state,
    const demo_level &level)
{
    char current_tile;

    current_tile = demo_level_tile_at(level,
        static_cast<int32_t>(game_state->player_position_x),
        static_cast<int32_t>(game_state->player_position_y));
    game_state->player_floor_height = demo_tile_floor_height(current_tile,
        game_state->player_position_x);
    game_state->stair_progress = game_state->player_floor_height;
    return ;
}

static void demo_finish_floor_if_stair_complete(demo_game_state *game_state)
{
    (void)game_state;
    return ;
}

static void demo_handle_playing(demo_game_state *game_state,
    const demo_input_state *input_state, double frame_delta_seconds)
{
    double move_speed;
    double turn_speed;
    double move_distance;
    double turn_angle;
    const demo_level *level;

    move_speed = 5.4;
    turn_speed = 3.6;
    move_distance = move_speed * frame_delta_seconds;
    turn_angle = turn_speed * frame_delta_seconds;
    if (input_state->turn_left_down == FT_TRUE)
    {
        demo_rotate_player(game_state, -turn_angle);
    }
    if (input_state->turn_right_down == FT_TRUE)
    {
        demo_rotate_player(game_state, turn_angle);
    }
    if (input_state->move_forward_down == FT_TRUE)
    {
        demo_try_move_player(game_state,
            game_state->player_direction_x * move_distance,
            game_state->player_direction_y * move_distance);
    }
    if (input_state->move_backward_down == FT_TRUE)
    {
        demo_try_move_player(game_state,
            -game_state->player_direction_x * move_distance,
            -game_state->player_direction_y * move_distance);
    }
    if (input_state->back_pressed == FT_TRUE)
    {
        game_state->mode = DEMO_MODE_MENU;
        return ;
    }
    level = &g_demo_levels[game_state->current_level_index];
    demo_update_player_stair_height(game_state, *level);
    demo_finish_floor_if_stair_complete(game_state);
    return ;
}

static void demo_handle_level_clear(demo_game_state *game_state,
    const demo_input_state *input_state)
{
    if (input_state->confirm_pressed == FT_TRUE)
    {
        demo_prepare_level(game_state, game_state->pending_level_index);
    }
    if (input_state->back_pressed == FT_TRUE)
    {
        game_state->mode = DEMO_MODE_MENU;
    }
    return ;
}

static void demo_handle_leaderboard(demo_game_state *game_state,
    const demo_input_state *input_state)
{
    if (input_state->confirm_pressed == FT_TRUE || input_state->back_pressed == FT_TRUE)
    {
        game_state->mode = DEMO_MODE_MENU;
    }
    return ;
}

static void demo_handle_name_entry(demo_game_state *game_state,
    const demo_input_state *input_state, demo_leaderboard *leaderboard)
{
    char *selected_letter;

    selected_letter = &game_state->pending_name[game_state->pending_name_index];
    if (input_state->menu_left_pressed == FT_TRUE
        && game_state->pending_name_index > 0U)
    {
        game_state->pending_name_index = game_state->pending_name_index - 1U;
    }
    if (input_state->menu_right_pressed == FT_TRUE
        && game_state->pending_name_index < 2U)
    {
        game_state->pending_name_index = game_state->pending_name_index + 1U;
    }
    selected_letter = &game_state->pending_name[game_state->pending_name_index];
    if (input_state->menu_up_pressed == FT_TRUE)
    {
        *selected_letter = static_cast<char>(*selected_letter + 1);
        if (*selected_letter > 'Z')
        {
            *selected_letter = 'A';
        }
    }
    if (input_state->menu_down_pressed == FT_TRUE)
    {
        *selected_letter = static_cast<char>(*selected_letter - 1);
        if (*selected_letter < 'A')
        {
            *selected_letter = 'Z';
        }
    }
    if (input_state->confirm_pressed == FT_TRUE)
    {
        if (game_state->pending_name_index < 2U)
        {
            game_state->pending_name_index = game_state->pending_name_index + 1U;
        }
        else
        {
            demo_finish_run(game_state, leaderboard);
        }
    }
    if (input_state->back_pressed == FT_TRUE)
    {
        game_state->mode = DEMO_MODE_MENU;
    }
    return ;
}

void demo_update(demo_game_state *game_state, const demo_input_state *input_state,
    demo_leaderboard *leaderboard, double frame_delta_seconds)
{
    if (game_state->mode == DEMO_MODE_MENU)
    {
        demo_handle_menu(game_state, input_state);
        return ;
    }
    if (game_state->mode == DEMO_MODE_PLAYING)
    {
        demo_handle_playing(game_state, input_state, frame_delta_seconds);
        return ;
    }
    if (game_state->mode == DEMO_MODE_LEVEL_CLEAR)
    {
        demo_handle_level_clear(game_state, input_state);
        return ;
    }
    if (game_state->mode == DEMO_MODE_LEADERBOARD)
    {
        demo_handle_leaderboard(game_state, input_state);
        return ;
    }
    if (game_state->mode == DEMO_MODE_NAME_ENTRY)
    {
        demo_handle_name_entry(game_state, input_state, leaderboard);
        return ;
    }
    return ;
}

int32_t demo_run(void)
{
    ft_render_window                      render_window;
    ft_render_window_desc                 window_desc;
    demo_game_state                       game_state;
    demo_input_state                      input_state;
    demo_leaderboard                      leaderboard;
    std::chrono::steady_clock::time_point next_frame_time;
    std::chrono::steady_clock::time_point previous_frame_time;
    std::chrono::steady_clock::time_point current_frame_time;
    int64_t                               frame_delta_microseconds;
    double                                frame_delta_seconds;
    int32_t                               error_code;

    std::memset(&input_state, 0, sizeof(input_state));
    error_code = demo_validate_levels();
    if (error_code != FT_ERR_SUCCESS)
    {
        return (error_code);
    }
    demo_init_game_state(&game_state);
    demo_leaderboard_load(&leaderboard);
    error_code = render_window.initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        return (error_code);
    }
    demo_choose_window_size(&window_desc);
    error_code = render_window.initialize(window_desc);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)render_window.destroy();
        return (error_code);
    }
    previous_frame_time = std::chrono::steady_clock::now();
    next_frame_time = previous_frame_time;
    while (render_window.should_close() == FT_FALSE
        && game_state.should_quit == FT_FALSE)
    {
        std::this_thread::sleep_until(next_frame_time);
        current_frame_time = std::chrono::steady_clock::now();
        frame_delta_microseconds = std::chrono::duration_cast<
            std::chrono::microseconds>(current_frame_time - previous_frame_time).count();
        previous_frame_time = current_frame_time;
        if (frame_delta_microseconds < 0)
        {
            frame_delta_microseconds = 0;
        }
        frame_delta_seconds = static_cast<double>(frame_delta_microseconds)
            / 1000000.0;
        if (frame_delta_seconds <= 0.0)
        {
            frame_delta_seconds = DEMO_NOMINAL_FRAME_SECONDS;
        }
        if (frame_delta_seconds > DEMO_MAX_FRAME_SECONDS)
        {
            frame_delta_seconds = DEMO_MAX_FRAME_SECONDS;
        }
        error_code = render_window.poll_events();
        if (error_code != FT_ERR_SUCCESS)
        {
            break ;
        }
        demo_poll_input(&input_state);
        demo_update(&game_state, &input_state, &leaderboard,
            frame_delta_seconds);
        demo_update_fps_counter(&game_state);
        demo_draw_frame(render_window, game_state, leaderboard);
        error_code = render_window.present();
        if (error_code != FT_ERR_SUCCESS)
        {
            break ;
        }
        next_frame_time = next_frame_time + std::chrono::microseconds(
            DEMO_TARGET_FRAME_MICROSECONDS);
        if (current_frame_time > next_frame_time)
        {
            next_frame_time = current_frame_time;
        }
    }
    if (error_code == FT_ERR_SUCCESS)
    {
        error_code = render_window.destroy();
    }
    else
    {
        (void)render_window.destroy();
    }
    return (error_code);
}
