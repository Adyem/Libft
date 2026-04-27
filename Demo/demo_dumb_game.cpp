#include "../CMA/CMA.hpp"
#include "../DUMB/controls.hpp"
#include "../DUMB/dumb_io.hpp"
#include "../DUMB/render_window.hpp"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <thread>

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
    ft_bool previous_menu_up_down;
    ft_bool previous_menu_down_down;
    ft_bool previous_menu_left_down;
    ft_bool previous_menu_right_down;
    ft_bool previous_confirm_down;
    ft_bool previous_back_down;
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
    const char  *name;
    const char  *tiles;
    int32_t     width;
    int32_t     height;
    double      start_position_x;
    double      start_position_y;
    double      start_direction_x;
    double      start_direction_y;
};

struct demo_game_state
{
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
    ft_bool     should_quit;
    std::chrono::steady_clock::time_point run_start_time;
};

static const int32_t DEMO_MODE_MENU = 0;
static const int32_t DEMO_MODE_PLAYING = 1;
static const int32_t DEMO_MODE_LEVEL_CLEAR = 2;
static const int32_t DEMO_MODE_LEADERBOARD = 3;
static const int32_t DEMO_MODE_NAME_ENTRY = 4;

static const char g_demo_level_tiles_0[] =
    "111111111111"
    "100000000001"
    "101111011101"
    "101001010001"
    "101001011101"
    "101001000001"
    "101011111101"
    "101000100001"
    "101110101101"
    "1000001000E1"
    "101111111101"
    "111111111111";

static const char g_demo_level_tiles_1[] =
    "111111111111"
    "1000001000E1"
    "101110101101"
    "101010101001"
    "101010111101"
    "101010000001"
    "101011111101"
    "101000000101"
    "101111110101"
    "100000010001"
    "101111011101"
    "111111111111";

static const char g_demo_level_tiles_2[] =
    "111111111111"
    "100000000001"
    "101111111101"
    "100010000001"
    "111010111101"
    "100010100001"
    "101110101111"
    "1010001010E1"
    "101011101101"
    "100010000001"
    "101111111101"
    "111111111111";

static const demo_level g_demo_levels[] =
{
    {
        "ENTRY HALL",
        g_demo_level_tiles_0,
        12,
        12,
        1.5,
        1.5,
        1.0,
        0.0
    },
    {
        "BRICK LOOP",
        g_demo_level_tiles_1,
        12,
        12,
        1.5,
        10.5,
        1.0,
        0.0
    },
    {
        "LAST TURN",
        g_demo_level_tiles_2,
        12,
        12,
        1.5,
        1.5,
        1.0,
        0.0
    }
};

static uint32_t demo_make_color(uint32_t red, uint32_t green, uint32_t blue)
{
    return ((red << 16) | (green << 8) | blue);
}

static uint32_t demo_min_u32(uint32_t left_value, uint32_t right_value)
{
    if (left_value < right_value)
    {
        return (left_value);
    }
    return (right_value);
}

static int32_t demo_clamp_s32(int32_t value, int32_t minimum_value,
    int32_t maximum_value)
{
    if (value < minimum_value)
    {
        return (minimum_value);
    }
    if (value > maximum_value)
    {
        return (maximum_value);
    }
    return (value);
}

static uint32_t demo_shade_color(uint32_t color, uint32_t numerator,
    uint32_t denominator)
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    if (denominator == 0U)
    {
        return (color);
    }
    red = ((color >> 16) & 255U) * numerator / denominator;
    green = ((color >> 8) & 255U) * numerator / denominator;
    blue = (color & 255U) * numerator / denominator;
    return (demo_make_color(red, green, blue));
}

static const char *demo_get_glyph_pattern(char character)
{
    static const char glyph_space[] = "00000""00000""00000""00000""00000""00000""00000";
    static const char glyph_dash[] = "00000""00000""00000""11111""00000""00000""00000";
    static const char glyph_colon[] = "00000""00100""00100""00000""00100""00100""00000";
    static const char glyph_period[] = "00000""00000""00000""00000""00000""00110""00110";
    static const char glyph_0[] = "01110""10001""10011""10101""11001""10001""01110";
    static const char glyph_1[] = "00100""01100""00100""00100""00100""00100""01110";
    static const char glyph_2[] = "01110""10001""00001""00010""00100""01000""11111";
    static const char glyph_3[] = "11110""00001""00001""01110""00001""00001""11110";
    static const char glyph_4[] = "00010""00110""01010""10010""11111""00010""00010";
    static const char glyph_5[] = "11111""10000""10000""11110""00001""00001""11110";
    static const char glyph_6[] = "01110""10000""10000""11110""10001""10001""01110";
    static const char glyph_7[] = "11111""00001""00010""00100""01000""01000""01000";
    static const char glyph_8[] = "01110""10001""10001""01110""10001""10001""01110";
    static const char glyph_9[] = "01110""10001""10001""01111""00001""00001""01110";
    static const char glyph_a[] = "01110""10001""10001""11111""10001""10001""10001";
    static const char glyph_b[] = "11110""10001""10001""11110""10001""10001""11110";
    static const char glyph_c[] = "01110""10001""10000""10000""10000""10001""01110";
    static const char glyph_d[] = "11110""10001""10001""10001""10001""10001""11110";
    static const char glyph_e[] = "11111""10000""10000""11110""10000""10000""11111";
    static const char glyph_f[] = "11111""10000""10000""11110""10000""10000""10000";
    static const char glyph_g[] = "01110""10001""10000""10111""10001""10001""01110";
    static const char glyph_h[] = "10001""10001""10001""11111""10001""10001""10001";
    static const char glyph_i[] = "01110""00100""00100""00100""00100""00100""01110";
    static const char glyph_j[] = "00001""00001""00001""00001""10001""10001""01110";
    static const char glyph_k[] = "10001""10010""10100""11000""10100""10010""10001";
    static const char glyph_l[] = "10000""10000""10000""10000""10000""10000""11111";
    static const char glyph_m[] = "10001""11011""10101""10101""10001""10001""10001";
    static const char glyph_n[] = "10001""11001""10101""10011""10001""10001""10001";
    static const char glyph_o[] = "01110""10001""10001""10001""10001""10001""01110";
    static const char glyph_p[] = "11110""10001""10001""11110""10000""10000""10000";
    static const char glyph_q[] = "01110""10001""10001""10001""10101""10010""01101";
    static const char glyph_r[] = "11110""10001""10001""11110""10100""10010""10001";
    static const char glyph_s[] = "01111""10000""10000""01110""00001""00001""11110";
    static const char glyph_t[] = "11111""00100""00100""00100""00100""00100""00100";
    static const char glyph_u[] = "10001""10001""10001""10001""10001""10001""01110";
    static const char glyph_v[] = "10001""10001""10001""10001""10001""01010""00100";
    static const char glyph_w[] = "10001""10001""10001""10101""10101""10101""01010";
    static const char glyph_x[] = "10001""10001""01010""00100""01010""10001""10001";
    static const char glyph_y[] = "10001""10001""01010""00100""00100""00100""00100";
    static const char glyph_z[] = "11111""00001""00010""00100""01000""10000""11111";

    if (character == ' ')
    {
        return (glyph_space);
    }
    if (character == '-')
    {
        return (glyph_dash);
    }
    if (character == ':')
    {
        return (glyph_colon);
    }
    if (character == '.')
    {
        return (glyph_period);
    }
    if (character == '0')
    {
        return (glyph_0);
    }
    if (character == '1')
    {
        return (glyph_1);
    }
    if (character == '2')
    {
        return (glyph_2);
    }
    if (character == '3')
    {
        return (glyph_3);
    }
    if (character == '4')
    {
        return (glyph_4);
    }
    if (character == '5')
    {
        return (glyph_5);
    }
    if (character == '6')
    {
        return (glyph_6);
    }
    if (character == '7')
    {
        return (glyph_7);
    }
    if (character == '8')
    {
        return (glyph_8);
    }
    if (character == '9')
    {
        return (glyph_9);
    }
    if (character == 'A')
    {
        return (glyph_a);
    }
    if (character == 'B')
    {
        return (glyph_b);
    }
    if (character == 'C')
    {
        return (glyph_c);
    }
    if (character == 'D')
    {
        return (glyph_d);
    }
    if (character == 'E')
    {
        return (glyph_e);
    }
    if (character == 'F')
    {
        return (glyph_f);
    }
    if (character == 'G')
    {
        return (glyph_g);
    }
    if (character == 'H')
    {
        return (glyph_h);
    }
    if (character == 'I')
    {
        return (glyph_i);
    }
    if (character == 'J')
    {
        return (glyph_j);
    }
    if (character == 'K')
    {
        return (glyph_k);
    }
    if (character == 'L')
    {
        return (glyph_l);
    }
    if (character == 'M')
    {
        return (glyph_m);
    }
    if (character == 'N')
    {
        return (glyph_n);
    }
    if (character == 'O')
    {
        return (glyph_o);
    }
    if (character == 'P')
    {
        return (glyph_p);
    }
    if (character == 'Q')
    {
        return (glyph_q);
    }
    if (character == 'R')
    {
        return (glyph_r);
    }
    if (character == 'S')
    {
        return (glyph_s);
    }
    if (character == 'T')
    {
        return (glyph_t);
    }
    if (character == 'U')
    {
        return (glyph_u);
    }
    if (character == 'V')
    {
        return (glyph_v);
    }
    if (character == 'W')
    {
        return (glyph_w);
    }
    if (character == 'X')
    {
        return (glyph_x);
    }
    if (character == 'Y')
    {
        return (glyph_y);
    }
    if (character == 'Z')
    {
        return (glyph_z);
    }
    return (glyph_space);
}

static void demo_draw_rectangle(ft_render_framebuffer &framebuffer,
    int32_t coordinate_x, int32_t coordinate_y, int32_t width, int32_t height,
    uint32_t color)
{
    int32_t index_width;
    int32_t index_height;
    int32_t write_x;
    int32_t write_y;

    index_height = 0;
    while (index_height < height)
    {
        write_y = coordinate_y + index_height;
        if (write_y >= 0 && write_y < framebuffer.height)
        {
            index_width = 0;
            while (index_width < width)
            {
                write_x = coordinate_x + index_width;
                if (write_x >= 0 && write_x < framebuffer.width)
                {
                    framebuffer.pixels[(write_y * framebuffer.width) + write_x] = color;
                }
                index_width = index_width + 1;
            }
        }
        index_height = index_height + 1;
    }
    return ;
}

static void demo_draw_glyph(ft_render_framebuffer &framebuffer, int32_t coordinate_x,
    int32_t coordinate_y, char character, int32_t scale, uint32_t color)
{
    const char  *pattern;
    int32_t     row_index;
    int32_t     column_index;
    int32_t     pattern_index;

    pattern = demo_get_glyph_pattern(character);
    row_index = 0;
    while (row_index < 7)
    {
        column_index = 0;
        while (column_index < 5)
        {
            pattern_index = (row_index * 5) + column_index;
            if (pattern[pattern_index] == '1')
            {
                demo_draw_rectangle(framebuffer,
                    coordinate_x + (column_index * scale),
                    coordinate_y + (row_index * scale),
                    scale, scale, color);
            }
            column_index = column_index + 1;
        }
        row_index = row_index + 1;
    }
    return ;
}

static void demo_draw_text(ft_render_framebuffer &framebuffer, int32_t coordinate_x,
    int32_t coordinate_y, const char *text, int32_t scale, uint32_t color)
{
    ft_size_t index_character;
    ft_size_t text_length;

    if (text == ft_nullptr)
    {
        return ;
    }
    text_length = std::strlen(text);
    index_character = 0;
    while (index_character < text_length)
    {
        demo_draw_glyph(framebuffer,
            coordinate_x + static_cast<int32_t>(index_character) * (6 * scale),
            coordinate_y, text[index_character], scale, color);
        index_character = index_character + 1;
    }
    return ;
}

static void demo_format_time(char *buffer, ft_size_t buffer_size,
    uint32_t elapsed_milliseconds)
{
    uint32_t total_seconds;
    uint32_t minutes;
    uint32_t seconds;
    uint32_t centiseconds;

    total_seconds = elapsed_milliseconds / 1000U;
    minutes = total_seconds / 60U;
    seconds = total_seconds % 60U;
    centiseconds = (elapsed_milliseconds % 1000U) / 10U;
    std::snprintf(buffer, buffer_size, "%02u:%02u.%02u", minutes, seconds,
        centiseconds);
    return ;
}

static ft_bool demo_level_is_wall(const demo_level &level, int32_t tile_x,
    int32_t tile_y)
{
    int32_t tile_index;

    if (tile_x < 0 || tile_y < 0 || tile_x >= level.width || tile_y >= level.height)
    {
        return (FT_TRUE);
    }
    tile_index = (tile_y * level.width) + tile_x;
    if (level.tiles[tile_index] == '1')
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static char demo_level_tile_at(const demo_level &level, int32_t tile_x, int32_t tile_y)
{
    int32_t tile_index;

    if (tile_x < 0 || tile_y < 0 || tile_x >= level.width || tile_y >= level.height)
    {
        return ('1');
    }
    tile_index = (tile_y * level.width) + tile_x;
    return (level.tiles[tile_index]);
}

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
    game_state->run_start_time = std::chrono::steady_clock::now();
    demo_reset_name_entry(game_state);
    demo_prepare_level(game_state, 0);
    return ;
}

static void demo_init_game_state(demo_game_state *game_state)
{
    game_state->mode = DEMO_MODE_MENU;
    game_state->selected_menu_index = 0;
    game_state->current_level_index = 0;
    game_state->pending_level_index = 0;
    game_state->player_position_x = 1.5;
    game_state->player_position_y = 1.5;
    game_state->player_direction_x = 1.0;
    game_state->player_direction_y = 0.0;
    game_state->camera_plane_x = 0.0;
    game_state->camera_plane_y = 0.66;
    game_state->should_quit = FT_FALSE;
    game_state->total_elapsed_milliseconds = 0U;
    game_state->run_start_time = std::chrono::steady_clock::now();
    demo_reset_name_entry(game_state);
    return ;
}

static void demo_poll_input(demo_input_state *input_state)
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

static void demo_leaderboard_reset(demo_leaderboard *leaderboard)
{
    uint32_t index_entry;

    leaderboard->entry_count = 0U;
    index_entry = 0U;
    while (index_entry < 5U)
    {
        leaderboard->entries[index_entry].name[0] = 'A';
        leaderboard->entries[index_entry].name[1] = 'A';
        leaderboard->entries[index_entry].name[2] = 'A';
        leaderboard->entries[index_entry].name[3] = '\0';
        leaderboard->entries[index_entry].elapsed_milliseconds = 0U;
        index_entry = index_entry + 1U;
    }
    return ;
}

static void demo_leaderboard_insert(demo_leaderboard *leaderboard, const char *name,
    uint32_t elapsed_milliseconds)
{
    demo_score_entry new_entry;
    uint32_t index_entry;
    uint32_t insert_index;
    uint32_t move_index;

    new_entry.name[0] = name[0];
    new_entry.name[1] = name[1];
    new_entry.name[2] = name[2];
    new_entry.name[3] = '\0';
    new_entry.elapsed_milliseconds = elapsed_milliseconds;
    insert_index = leaderboard->entry_count;
    index_entry = 0U;
    while (index_entry < leaderboard->entry_count)
    {
        if (elapsed_milliseconds < leaderboard->entries[index_entry].elapsed_milliseconds)
        {
            insert_index = index_entry;
            break ;
        }
        index_entry = index_entry + 1U;
    }
    if (leaderboard->entry_count < 5U)
    {
        leaderboard->entry_count = leaderboard->entry_count + 1U;
    }
    move_index = leaderboard->entry_count;
    while (move_index > insert_index + 1U)
    {
        leaderboard->entries[move_index - 1U] = leaderboard->entries[move_index - 2U];
        move_index = move_index - 1U;
    }
    if (insert_index < 5U)
    {
        leaderboard->entries[insert_index] = new_entry;
    }
    return ;
}

static void demo_leaderboard_save(const demo_leaderboard *leaderboard)
{
    char        buffer[256];
    uint32_t    index_entry;
    int32_t     write_index;
    int32_t     line_length;

    std::memset(buffer, 0, sizeof(buffer));
    write_index = 0;
    index_entry = 0U;
    while (index_entry < leaderboard->entry_count)
    {
        line_length = std::snprintf(buffer + write_index,
            sizeof(buffer) - static_cast<ft_size_t>(write_index),
            "%c%c%c %u\n",
            leaderboard->entries[index_entry].name[0],
            leaderboard->entries[index_entry].name[1],
            leaderboard->entries[index_entry].name[2],
            leaderboard->entries[index_entry].elapsed_milliseconds);
        if (line_length <= 0)
        {
            return ;
        }
        write_index = write_index + line_length;
        if (write_index >= static_cast<int32_t>(sizeof(buffer)))
        {
            break ;
        }
        index_entry = index_entry + 1U;
    }
    (void)ft_write_file("Demo/demo_leaderboard.txt", buffer,
        static_cast<ft_size_t>(write_index));
    return ;
}

static uint32_t demo_parse_u32(const char *buffer, ft_size_t *index_pointer,
    ft_size_t size)
{
    uint32_t value;

    value = 0U;
    while (*index_pointer < size && buffer[*index_pointer] >= '0'
        && buffer[*index_pointer] <= '9')
    {
        value = (value * 10U)
            + static_cast<uint32_t>(buffer[*index_pointer] - '0');
        *index_pointer = *index_pointer + 1U;
    }
    return (value);
}

static void demo_leaderboard_load(demo_leaderboard *leaderboard)
{
    char        *raw_buffer;
    ft_size_t   buffer_size;
    ft_size_t   index_buffer;
    char        name_buffer[4];
    uint32_t    elapsed_milliseconds;
    int32_t     read_error;

    demo_leaderboard_reset(leaderboard);
    raw_buffer = ft_nullptr;
    buffer_size = 0U;
    read_error = ft_read_file("Demo/demo_leaderboard.txt", &raw_buffer, &buffer_size);
    if (read_error != FT_ERR_SUCCESS || raw_buffer == ft_nullptr)
    {
        return ;
    }
    index_buffer = 0U;
    while (index_buffer + 4U < buffer_size && leaderboard->entry_count < 5U)
    {
        if ((raw_buffer[index_buffer] < 'A' || raw_buffer[index_buffer] > 'Z')
            || (raw_buffer[index_buffer + 1U] < 'A' || raw_buffer[index_buffer + 1U] > 'Z')
            || (raw_buffer[index_buffer + 2U] < 'A' || raw_buffer[index_buffer + 2U] > 'Z'))
        {
            while (index_buffer < buffer_size && raw_buffer[index_buffer] != '\n')
            {
                index_buffer = index_buffer + 1U;
            }
            if (index_buffer < buffer_size)
            {
                index_buffer = index_buffer + 1U;
            }
            continue ;
        }
        name_buffer[0] = raw_buffer[index_buffer];
        name_buffer[1] = raw_buffer[index_buffer + 1U];
        name_buffer[2] = raw_buffer[index_buffer + 2U];
        name_buffer[3] = '\0';
        index_buffer = index_buffer + 3U;
        if (index_buffer >= buffer_size || raw_buffer[index_buffer] != ' ')
        {
            break ;
        }
        index_buffer = index_buffer + 1U;
        elapsed_milliseconds = demo_parse_u32(raw_buffer, &index_buffer, buffer_size);
        demo_leaderboard_insert(leaderboard, name_buffer, elapsed_milliseconds);
        while (index_buffer < buffer_size && raw_buffer[index_buffer] != '\n')
        {
            index_buffer = index_buffer + 1U;
        }
        if (index_buffer < buffer_size)
        {
            index_buffer = index_buffer + 1U;
        }
    }
    cma_free(raw_buffer);
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

static void demo_handle_playing(demo_game_state *game_state,
    const demo_input_state *input_state)
{
    double move_speed;
    double turn_speed;
    const demo_level *level;
    char current_tile;

    move_speed = 0.09;
    turn_speed = 0.06;
    if (input_state->turn_left_down == FT_TRUE)
    {
        demo_rotate_player(game_state, -turn_speed);
    }
    if (input_state->turn_right_down == FT_TRUE)
    {
        demo_rotate_player(game_state, turn_speed);
    }
    if (input_state->move_forward_down == FT_TRUE)
    {
        demo_try_move_player(game_state,
            game_state->player_direction_x * move_speed,
            game_state->player_direction_y * move_speed);
    }
    if (input_state->move_backward_down == FT_TRUE)
    {
        demo_try_move_player(game_state,
            -game_state->player_direction_x * move_speed,
            -game_state->player_direction_y * move_speed);
    }
    if (input_state->back_pressed == FT_TRUE)
    {
        game_state->mode = DEMO_MODE_MENU;
        return ;
    }
    level = &g_demo_levels[game_state->current_level_index];
    current_tile = demo_level_tile_at(*level,
        static_cast<int32_t>(game_state->player_position_x),
        static_cast<int32_t>(game_state->player_position_y));
    if (current_tile == 'E')
    {
        game_state->pending_level_index = game_state->current_level_index + 1;
        if (game_state->pending_level_index >= 3)
        {
            game_state->mode = DEMO_MODE_NAME_ENTRY;
            demo_reset_name_entry(game_state);
        }
        else
        {
            game_state->mode = DEMO_MODE_LEVEL_CLEAR;
        }
    }
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

static void demo_update(demo_game_state *game_state, const demo_input_state *input_state,
    demo_leaderboard *leaderboard)
{
    if (game_state->mode == DEMO_MODE_MENU)
    {
        demo_handle_menu(game_state, input_state);
        return ;
    }
    if (game_state->mode == DEMO_MODE_PLAYING)
    {
        demo_handle_playing(game_state, input_state);
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

static void demo_draw_background_gradient(ft_render_framebuffer &framebuffer)
{
    int32_t index_y;
    int32_t index_x;
    uint32_t top_color;
    uint32_t bottom_color;
    uint32_t blend_top;
    uint32_t blend_bottom;
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    index_y = 0;
    while (index_y < framebuffer.height)
    {
        blend_top = static_cast<uint32_t>(framebuffer.height - index_y);
        blend_bottom = static_cast<uint32_t>(index_y);
        if (index_y < framebuffer.height / 2)
        {
            top_color = demo_make_color(18U, 25U, 48U);
            bottom_color = demo_make_color(70U, 115U, 165U);
        }
        else
        {
            top_color = demo_make_color(50U, 38U, 28U);
            bottom_color = demo_make_color(16U, 12U, 10U);
        }
        red = ((((top_color >> 16) & 255U) * blend_top)
            + (((bottom_color >> 16) & 255U) * blend_bottom))
            / static_cast<uint32_t>(framebuffer.height);
        green = ((((top_color >> 8) & 255U) * blend_top)
            + (((bottom_color >> 8) & 255U) * blend_bottom))
            / static_cast<uint32_t>(framebuffer.height);
        blue = (((top_color & 255U) * blend_top)
            + ((bottom_color & 255U) * blend_bottom))
            / static_cast<uint32_t>(framebuffer.height);
        index_x = 0;
        while (index_x < framebuffer.width)
        {
            framebuffer.pixels[(index_y * framebuffer.width) + index_x]
                = demo_make_color(red, green, blue);
            index_x = index_x + 1;
        }
        index_y = index_y + 1;
    }
    return ;
}

static void demo_draw_wall_column(ft_render_framebuffer &framebuffer, int32_t screen_x,
    int32_t start_y, int32_t end_y, uint32_t color)
{
    int32_t draw_y;

    draw_y = start_y;
    while (draw_y <= end_y)
    {
        if (draw_y >= 0 && draw_y < framebuffer.height)
        {
            framebuffer.pixels[(draw_y * framebuffer.width) + screen_x] = color;
        }
        draw_y = draw_y + 1;
    }
    return ;
}

static void demo_draw_3d_view(ft_render_framebuffer &framebuffer,
    const demo_game_state &game_state)
{
    const demo_level *level;
    int32_t screen_x;

    level = &g_demo_levels[game_state.current_level_index];
    demo_draw_background_gradient(framebuffer);
    screen_x = 0;
    while (screen_x < framebuffer.width)
    {
        double camera_x;
        double ray_direction_x;
        double ray_direction_y;
        int32_t map_x;
        int32_t map_y;
        double delta_distance_x;
        double delta_distance_y;
        double side_distance_x;
        double side_distance_y;
        int32_t step_x;
        int32_t step_y;
        ft_bool hit_wall;
        int32_t wall_side;
        double perpendicular_wall_distance;
        int32_t line_height;
        int32_t draw_start_y;
        int32_t draw_end_y;
        uint32_t wall_color;
        uint32_t distance_shade;

        camera_x = (2.0 * static_cast<double>(screen_x)
            / static_cast<double>(framebuffer.width)) - 1.0;
        ray_direction_x = game_state.player_direction_x
            + (game_state.camera_plane_x * camera_x);
        ray_direction_y = game_state.player_direction_y
            + (game_state.camera_plane_y * camera_x);
        map_x = static_cast<int32_t>(game_state.player_position_x);
        map_y = static_cast<int32_t>(game_state.player_position_y);
        if (std::fabs(ray_direction_x) < 0.0001)
        {
            delta_distance_x = 1.0e30;
        }
        else
        {
            delta_distance_x = std::fabs(1.0 / ray_direction_x);
        }
        if (std::fabs(ray_direction_y) < 0.0001)
        {
            delta_distance_y = 1.0e30;
        }
        else
        {
            delta_distance_y = std::fabs(1.0 / ray_direction_y);
        }
        if (ray_direction_x < 0.0)
        {
            step_x = -1;
            side_distance_x = (game_state.player_position_x
                - static_cast<double>(map_x)) * delta_distance_x;
        }
        else
        {
            step_x = 1;
            side_distance_x = (static_cast<double>(map_x + 1)
                - game_state.player_position_x) * delta_distance_x;
        }
        if (ray_direction_y < 0.0)
        {
            step_y = -1;
            side_distance_y = (game_state.player_position_y
                - static_cast<double>(map_y)) * delta_distance_y;
        }
        else
        {
            step_y = 1;
            side_distance_y = (static_cast<double>(map_y + 1)
                - game_state.player_position_y) * delta_distance_y;
        }
        hit_wall = FT_FALSE;
        wall_side = 0;
        while (hit_wall == FT_FALSE)
        {
            if (side_distance_x < side_distance_y)
            {
                side_distance_x = side_distance_x + delta_distance_x;
                map_x = map_x + step_x;
                wall_side = 0;
            }
            else
            {
                side_distance_y = side_distance_y + delta_distance_y;
                map_y = map_y + step_y;
                wall_side = 1;
            }
            if (demo_level_is_wall(*level, map_x, map_y) == FT_TRUE)
            {
                hit_wall = FT_TRUE;
            }
        }
        if (wall_side == 0)
        {
            perpendicular_wall_distance = (static_cast<double>(map_x)
                - game_state.player_position_x
                + static_cast<double>(1 - step_x) / 2.0) / ray_direction_x;
        }
        else
        {
            perpendicular_wall_distance = (static_cast<double>(map_y)
                - game_state.player_position_y
                + static_cast<double>(1 - step_y) / 2.0) / ray_direction_y;
        }
        if (perpendicular_wall_distance < 0.0001)
        {
            perpendicular_wall_distance = 0.0001;
        }
        line_height = static_cast<int32_t>(
            static_cast<double>(framebuffer.height) / perpendicular_wall_distance);
        draw_start_y = demo_clamp_s32((-line_height / 2) + (framebuffer.height / 2),
            0, framebuffer.height - 1);
        draw_end_y = demo_clamp_s32((line_height / 2) + (framebuffer.height / 2),
            0, framebuffer.height - 1);
        wall_color = demo_make_color(212U, 180U, 96U);
        if (wall_side == 1)
        {
            wall_color = demo_shade_color(wall_color, 3U, 4U);
        }
        distance_shade = demo_min_u32(
            static_cast<uint32_t>(perpendicular_wall_distance * 32.0), 180U);
        wall_color = demo_shade_color(wall_color, 220U - distance_shade, 220U);
        demo_draw_wall_column(framebuffer, screen_x, draw_start_y, draw_end_y, wall_color);
        screen_x = screen_x + 1;
    }
    return ;
}

static void demo_draw_minimap(ft_render_framebuffer &framebuffer,
    const demo_game_state &game_state)
{
    const demo_level *level;
    int32_t tile_x;
    int32_t tile_y;
    int32_t tile_size;
    int32_t map_origin_x;
    int32_t map_origin_y;
    char tile_value;
    int32_t player_x;
    int32_t player_y;

    level = &g_demo_levels[game_state.current_level_index];
    tile_size = 8;
    map_origin_x = 12;
    map_origin_y = 12;
    demo_draw_rectangle(framebuffer, map_origin_x - 4, map_origin_y - 4,
        (level->width * tile_size) + 8, (level->height * tile_size) + 8,
        demo_make_color(10U, 10U, 16U));
    tile_y = 0;
    while (tile_y < level->height)
    {
        tile_x = 0;
        while (tile_x < level->width)
        {
            tile_value = demo_level_tile_at(*level, tile_x, tile_y);
            if (tile_value == '1')
            {
                demo_draw_rectangle(framebuffer,
                    map_origin_x + (tile_x * tile_size),
                    map_origin_y + (tile_y * tile_size),
                    tile_size - 1, tile_size - 1, demo_make_color(82U, 66U, 52U));
            }
            else if (tile_value == 'E')
            {
                demo_draw_rectangle(framebuffer,
                    map_origin_x + (tile_x * tile_size),
                    map_origin_y + (tile_y * tile_size),
                    tile_size - 1, tile_size - 1, demo_make_color(50U, 180U, 110U));
            }
            else
            {
                demo_draw_rectangle(framebuffer,
                    map_origin_x + (tile_x * tile_size),
                    map_origin_y + (tile_y * tile_size),
                    tile_size - 1, tile_size - 1, demo_make_color(26U, 28U, 38U));
            }
            tile_x = tile_x + 1;
        }
        tile_y = tile_y + 1;
    }
    player_x = map_origin_x
        + static_cast<int32_t>(game_state.player_position_x * static_cast<double>(tile_size));
    player_y = map_origin_y
        + static_cast<int32_t>(game_state.player_position_y * static_cast<double>(tile_size));
    demo_draw_rectangle(framebuffer, player_x - 2, player_y - 2, 5, 5,
        demo_make_color(255U, 80U, 64U));
    demo_draw_rectangle(framebuffer, player_x, player_y,
        static_cast<int32_t>(game_state.player_direction_x * 6.0),
        static_cast<int32_t>(game_state.player_direction_y * 6.0),
        demo_make_color(255U, 255U, 255U));
    return ;
}

static void demo_draw_playing_hud(ft_render_framebuffer &framebuffer,
    const demo_game_state &game_state)
{
    char time_buffer[32];
    char level_buffer[64];
    int64_t elapsed_milliseconds;

    elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - game_state.run_start_time).count();
    if (elapsed_milliseconds < 0)
    {
        elapsed_milliseconds = 0;
    }
    demo_format_time(time_buffer, sizeof(time_buffer),
        static_cast<uint32_t>(elapsed_milliseconds));
    std::snprintf(level_buffer, sizeof(level_buffer), "LEVEL %d  %s",
        game_state.current_level_index + 1,
        g_demo_levels[game_state.current_level_index].name);
    demo_draw_text(framebuffer, 12, framebuffer.height - 50, level_buffer, 2,
        demo_make_color(255U, 232U, 180U));
    demo_draw_text(framebuffer, 12, framebuffer.height - 28, time_buffer, 2,
        demo_make_color(210U, 230U, 255U));
    demo_draw_text(framebuffer, framebuffer.width - 170, framebuffer.height - 28,
        "W S MOVE", 2, demo_make_color(220U, 220U, 220U));
    demo_draw_text(framebuffer, framebuffer.width - 170, framebuffer.height - 50,
        "A D TURN", 2, demo_make_color(220U, 220U, 220U));
    return ;
}

static void demo_draw_menu(ft_render_framebuffer &framebuffer,
    const demo_game_state &game_state)
{
    const char *options[3];
    int32_t option_index;
    uint32_t color;

    options[0] = "START RUN";
    options[1] = "LEADERBOARD";
    options[2] = "QUIT";
    demo_draw_background_gradient(framebuffer);
    demo_draw_text(framebuffer, 86, 46, "DUMB MAZE 3D", 4,
        demo_make_color(255U, 220U, 120U));
    demo_draw_text(framebuffer, 110, 96, "SOFTWARE CRAWLER", 2,
        demo_make_color(205U, 220U, 255U));
    option_index = 0;
    while (option_index < 3)
    {
        color = demo_make_color(200U, 200U, 200U);
        if (option_index == game_state.selected_menu_index)
        {
            demo_draw_rectangle(framebuffer, 132, 146 + (option_index * 42), 272, 28,
                demo_make_color(60U, 78U, 120U));
            color = demo_make_color(255U, 240U, 180U);
        }
        demo_draw_text(framebuffer, 150, 150 + (option_index * 42),
            options[option_index], 2, color);
        option_index = option_index + 1;
    }
    demo_draw_text(framebuffer, 118, framebuffer.height - 46,
        "ARROWS OR WASD THEN ENTER", 2, demo_make_color(190U, 200U, 220U));
    return ;
}

static void demo_draw_level_clear(ft_render_framebuffer &framebuffer,
    const demo_game_state &game_state)
{
    char level_buffer[64];

    demo_draw_background_gradient(framebuffer);
    demo_draw_text(framebuffer, 122, 82, "LEVEL CLEAR", 4,
        demo_make_color(255U, 226U, 122U));
    std::snprintf(level_buffer, sizeof(level_buffer), "NEXT %s",
        g_demo_levels[game_state.pending_level_index].name);
    demo_draw_text(framebuffer, 160, 148, level_buffer, 2,
        demo_make_color(220U, 235U, 255U));
    demo_draw_text(framebuffer, 136, 208, "ENTER FOR NEXT FLOOR", 2,
        demo_make_color(255U, 255U, 255U));
    demo_draw_text(framebuffer, 170, 236, "ESC FOR MENU", 2,
        demo_make_color(190U, 200U, 220U));
    return ;
}

static void demo_draw_leaderboard(ft_render_framebuffer &framebuffer,
    const demo_leaderboard &leaderboard)
{
    uint32_t index_entry;
    char line_buffer[64];
    char time_buffer[32];

    demo_draw_background_gradient(framebuffer);
    demo_draw_text(framebuffer, 112, 44, "LEADERBOARD", 4,
        demo_make_color(255U, 220U, 120U));
    if (leaderboard.entry_count == 0U)
    {
        demo_draw_text(framebuffer, 176, 146, "NO RUNS YET", 2,
            demo_make_color(220U, 230U, 255U));
    }
    index_entry = 0U;
    while (index_entry < leaderboard.entry_count)
    {
        demo_format_time(time_buffer, sizeof(time_buffer),
            leaderboard.entries[index_entry].elapsed_milliseconds);
        std::snprintf(line_buffer, sizeof(line_buffer), "%u  %s  %s",
            index_entry + 1U, leaderboard.entries[index_entry].name, time_buffer);
        demo_draw_text(framebuffer, 150, 116 + static_cast<int32_t>(index_entry * 30U),
            line_buffer, 2, demo_make_color(240U, 240U, 240U));
        index_entry = index_entry + 1U;
    }
    demo_draw_text(framebuffer, 116, framebuffer.height - 44,
        "ENTER OR ESC TO RETURN", 2, demo_make_color(190U, 200U, 220U));
    return ;
}

static void demo_draw_name_entry(ft_render_framebuffer &framebuffer,
    const demo_game_state &game_state)
{
    char time_buffer[32];
    char letter_buffer[2];
    uint32_t index_letter;
    int32_t position_x;

    demo_draw_background_gradient(framebuffer);
    demo_draw_text(framebuffer, 88, 44, "YOU CLEARED THE MAZE", 3,
        demo_make_color(255U, 220U, 120U));
    demo_format_time(time_buffer, sizeof(time_buffer),
        game_state.total_elapsed_milliseconds);
    demo_draw_text(framebuffer, 198, 96, time_buffer, 3,
        demo_make_color(220U, 235U, 255U));
    demo_draw_text(framebuffer, 160, 146, "ENTER YOUR TAG", 2,
        demo_make_color(255U, 255U, 255U));
    index_letter = 0U;
    while (index_letter < 3U)
    {
        position_x = 220 + static_cast<int32_t>(index_letter * 48U);
        if (index_letter == game_state.pending_name_index)
        {
            demo_draw_rectangle(framebuffer, position_x - 8, 178, 36, 56,
                demo_make_color(66U, 86U, 130U));
        }
        letter_buffer[0] = game_state.pending_name[index_letter];
        letter_buffer[1] = '\0';
        demo_draw_text(framebuffer, position_x, 190, letter_buffer, 4,
            demo_make_color(255U, 236U, 166U));
        index_letter = index_letter + 1U;
    }
    demo_draw_text(framebuffer, 92, framebuffer.height - 58,
        "UP DOWN CHANGE  LEFT RIGHT SELECT", 2,
        demo_make_color(200U, 210U, 225U));
    demo_draw_text(framebuffer, 160, framebuffer.height - 32,
        "ENTER TO SAVE", 2, demo_make_color(200U, 210U, 225U));
    return ;
}

static void demo_draw_frame(ft_render_window &render_window,
    const demo_game_state &game_state, const demo_leaderboard &leaderboard)
{
    ft_render_framebuffer &framebuffer = render_window.framebuffer();

    if (game_state.mode == DEMO_MODE_MENU)
    {
        demo_draw_menu(framebuffer, game_state);
        return ;
    }
    if (game_state.mode == DEMO_MODE_PLAYING)
    {
        demo_draw_3d_view(framebuffer, game_state);
        demo_draw_minimap(framebuffer, game_state);
        demo_draw_playing_hud(framebuffer, game_state);
        return ;
    }
    if (game_state.mode == DEMO_MODE_LEVEL_CLEAR)
    {
        demo_draw_level_clear(framebuffer, game_state);
        return ;
    }
    if (game_state.mode == DEMO_MODE_LEADERBOARD)
    {
        demo_draw_leaderboard(framebuffer, leaderboard);
        return ;
    }
    if (game_state.mode == DEMO_MODE_NAME_ENTRY)
    {
        demo_draw_name_entry(framebuffer, game_state);
        return ;
    }
    demo_draw_background_gradient(framebuffer);
    return ;
}

static void demo_choose_window_size(ft_render_window_desc *window_desc)
{
    ft_render_screen_size screen_size;

    screen_size = ft_render_get_primary_screen_size();
    window_desc->width = 1280;
    window_desc->height = 720;
    if (screen_size.width > 0 && screen_size.height > 0)
    {
        if (screen_size.width < 1280)
        {
            window_desc->width = (screen_size.width * 3) / 4;
        }
        if (screen_size.height < 720)
        {
            window_desc->height = (screen_size.height * 3) / 4;
        }
    }
    if (window_desc->width < 320)
    {
        window_desc->width = 320;
    }
    if (window_desc->height < 240)
    {
        window_desc->height = 240;
    }
    window_desc->title = "libft DUMB maze 3D";
    window_desc->flags = FT_RENDER_WINDOW_FLAG_NONE;
    return ;
}

static int32_t demo_run(void)
{
    ft_render_window                      render_window;
    ft_render_window_desc                 window_desc;
    demo_game_state                       game_state;
    demo_input_state                      input_state;
    demo_leaderboard                      leaderboard;
    int32_t                               error_code;

    std::memset(&input_state, 0, sizeof(input_state));
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
    while (render_window.should_close() == FT_FALSE
        && game_state.should_quit == FT_FALSE)
    {
        error_code = render_window.poll_events();
        if (error_code != FT_ERR_SUCCESS)
        {
            break ;
        }
        demo_poll_input(&input_state);
        demo_update(&game_state, &input_state, &leaderboard);
        demo_draw_frame(render_window, game_state, leaderboard);
        error_code = render_window.present();
        if (error_code != FT_ERR_SUCCESS)
        {
            break ;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
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

int main(void)
{
    int32_t error_code;

    std::fprintf(stdout, "Launching DUMB maze 3D. Close the window to exit.\n");
    error_code = demo_run();
    if (error_code != FT_ERR_SUCCESS)
    {
        std::fprintf(stderr, "DUMB maze 3D failed with error code %d\n", error_code);
        return (1);
    }
    return (0);
}
