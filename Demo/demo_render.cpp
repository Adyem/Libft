#include "demo_game.hpp"

#include "../Modules/CPP_class/class_nullptr.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

struct demo_image
{
    int32_t     width;
    int32_t     height;
    uint32_t    *pixels;
};

static const int32_t DEMO_RAYCAST_COLUMN_STEP = 3;

static int32_t                g_demo_cached_background_width = 0;
static int32_t                g_demo_cached_background_height = 0;
static std::vector<uint32_t>  g_demo_cached_background_pixels;
static int32_t                g_demo_cached_minimap_level_index = -1;
static std::vector<uint32_t>  g_demo_cached_minimap_pixels;

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

static uint32_t demo_blend_channel(uint32_t base_value, uint32_t light_value,
    uint32_t blend_amount)
{
    if (blend_amount > 255U)
    {
        blend_amount = 255U;
    }
    return (((base_value * (255U - blend_amount))
        + (light_value * blend_amount)) / 255U);
}

static uint32_t demo_apply_light_to_color(uint32_t base_color,
    uint32_t light_color, uint32_t light_amount)
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    red = demo_blend_channel((base_color >> 16) & 255U,
        (light_color >> 16) & 255U, light_amount);
    green = demo_blend_channel((base_color >> 8) & 255U,
        (light_color >> 8) & 255U, light_amount);
    blue = demo_blend_channel(base_color & 255U, light_color & 255U,
        light_amount);
    return (demo_make_color(red, green, blue));
}

static uint32_t demo_apply_level_lighting(const demo_level &level,
    uint32_t base_color, double hit_position_x, double hit_position_y)
{
    uint32_t light_index;
    double distance_x;
    double distance_y;
    double distance_to_light;
    double falloff;
    uint32_t strongest_light;
    uint32_t strongest_light_color;
    uint32_t brightness_amount;

    strongest_light = 0U;
    strongest_light_color = demo_make_color(255U, 255U, 255U);
    light_index = 0U;
    while (light_index < level.light_count)
    {
        distance_x = hit_position_x - level.lights[light_index].position_x;
        distance_y = hit_position_y - level.lights[light_index].position_y;
        distance_to_light = std::sqrt((distance_x * distance_x)
                + (distance_y * distance_y));
        if (distance_to_light < level.lights[light_index].radius)
        {
            falloff = 1.0 - (distance_to_light / level.lights[light_index].radius);
            brightness_amount = static_cast<uint32_t>(
                static_cast<double>(level.lights[light_index].intensity)
                * falloff * falloff);
            if (brightness_amount > strongest_light)
            {
                strongest_light = brightness_amount;
                strongest_light_color = level.lights[light_index].color;
            }
        }
        light_index = light_index + 1U;
    }
    if (strongest_light == 0U)
    {
        return (demo_shade_color(base_color, 3U, 5U));
    }
    if (strongest_light > 220U)
    {
        strongest_light = 220U;
    }
    return (demo_apply_light_to_color(base_color, strongest_light_color,
        strongest_light));
}

static demo_image demo_prepare_frame_image(ft_render_framebuffer &framebuffer)
{
    demo_image image;

    image.width = framebuffer.width;
    image.height = framebuffer.height;
    image.pixels = framebuffer.pixels;
    return (image);
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
    if (character >= '0' && character <= '9')
    {
        static const char *glyph_digits[] =
        {
            glyph_0, glyph_1, glyph_2, glyph_3, glyph_4,
            glyph_5, glyph_6, glyph_7, glyph_8, glyph_9
        };
        return (glyph_digits[character - '0']);
    }
    if (character >= 'A' && character <= 'Z')
    {
        static const char *glyph_letters[] =
        {
            glyph_a, glyph_b, glyph_c, glyph_d, glyph_e, glyph_f, glyph_g,
            glyph_h, glyph_i, glyph_j, glyph_k, glyph_l, glyph_m, glyph_n,
            glyph_o, glyph_p, glyph_q, glyph_r, glyph_s, glyph_t, glyph_u,
            glyph_v, glyph_w, glyph_x, glyph_y, glyph_z
        };
        return (glyph_letters[character - 'A']);
    }
    return (glyph_space);
}

static void demo_draw_rectangle(demo_image &image, int32_t coordinate_x,
    int32_t coordinate_y, int32_t width, int32_t height, uint32_t color)
{
    int32_t index_width;
    int32_t index_height;
    int32_t write_x;
    int32_t write_y;

    index_height = 0;
    while (index_height < height)
    {
        write_y = coordinate_y + index_height;
        if (write_y >= 0 && write_y < image.height)
        {
            index_width = 0;
            while (index_width < width)
            {
                write_x = coordinate_x + index_width;
                if (write_x >= 0 && write_x < image.width)
                {
                    image.pixels[static_cast<ft_size_t>(
                        (write_y * image.width) + write_x)] = color;
                }
                index_width = index_width + 1;
            }
        }
        index_height = index_height + 1;
    }
    return ;
}

static void demo_draw_light_marker(demo_image &image, int32_t center_x,
    int32_t center_y, uint32_t color)
{
    demo_draw_rectangle(image, center_x - 6, center_y - 6, 13, 13,
        demo_make_color(8U, 8U, 12U));
    demo_draw_rectangle(image, center_x - 5, center_y - 5, 11, 11,
        demo_make_color(24U, 24U, 30U));
    demo_draw_rectangle(image, center_x - 4, center_y - 4, 9, 9, color);
    demo_draw_rectangle(image, center_x - 1, center_y - 6, 3, 13,
        demo_make_color(255U, 248U, 228U));
    demo_draw_rectangle(image, center_x - 6, center_y - 1, 13, 3,
        demo_make_color(255U, 248U, 228U));
    demo_draw_rectangle(image, center_x - 2, center_y - 2, 5, 5,
        demo_make_color(255U, 255U, 255U));
    return ;
}

static void demo_draw_glyph(demo_image &image, int32_t coordinate_x,
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
                demo_draw_rectangle(image,
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

static void demo_draw_text(demo_image &image, int32_t coordinate_x,
    int32_t coordinate_y, const char *text, int32_t scale, uint32_t color)
{
    ft_size_t index_character;
    ft_size_t text_length;

    if (text == ft_nullptr)
    {
        return ;
    }
    text_length = std::strlen(text);
    index_character = 0U;
    while (index_character < text_length)
    {
        demo_draw_glyph(image,
            coordinate_x + static_cast<int32_t>(index_character) * (6 * scale),
            coordinate_y, text[index_character], scale, color);
        index_character = index_character + 1U;
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

static void demo_draw_background_gradient(demo_image &image)
{
    ft_size_t copied_size;
    int32_t   index_y;
    int32_t   index_x;
    uint32_t  top_color;
    uint32_t  bottom_color;
    uint32_t  blend_top;
    uint32_t  blend_bottom;
    uint32_t  red;
    uint32_t  green;
    uint32_t  blue;

    if (g_demo_cached_background_width != image.width
        || g_demo_cached_background_height != image.height)
    {
        g_demo_cached_background_width = image.width;
        g_demo_cached_background_height = image.height;
        g_demo_cached_background_pixels.resize(static_cast<ft_size_t>(
            image.width * image.height), 0U);
        index_y = 0;
        while (index_y < image.height)
        {
            blend_top = static_cast<uint32_t>(image.height - index_y);
            blend_bottom = static_cast<uint32_t>(index_y);
            if (index_y < image.height / 2)
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
                / static_cast<uint32_t>(image.height);
            green = ((((top_color >> 8) & 255U) * blend_top)
                + (((bottom_color >> 8) & 255U) * blend_bottom))
                / static_cast<uint32_t>(image.height);
            blue = (((top_color & 255U) * blend_top)
                + ((bottom_color & 255U) * blend_bottom))
                / static_cast<uint32_t>(image.height);
            index_x = 0;
            while (index_x < image.width)
            {
                g_demo_cached_background_pixels[static_cast<ft_size_t>(
                    (index_y * image.width) + index_x)]
                    = demo_make_color(red, green, blue);
                index_x = index_x + 1;
            }
            index_y = index_y + 1;
        }
    }
    copied_size = static_cast<ft_size_t>(image.width * image.height)
        * sizeof(uint32_t);
    std::memcpy(image.pixels, g_demo_cached_background_pixels.data(), copied_size);
    return ;
}

static void demo_draw_wall_column(demo_image &image, int32_t start_x,
    int32_t end_x, int32_t start_y, int32_t end_y, uint32_t color)
{
    int32_t draw_x;
    int32_t draw_y;

    draw_x = start_x;
    while (draw_x <= end_x)
    {
        draw_y = start_y;
        while (draw_y <= end_y)
        {
            if (draw_y >= 0 && draw_y < image.height)
            {
                image.pixels[static_cast<ft_size_t>(
                    (draw_y * image.width) + draw_x)] = color;
            }
            draw_y = draw_y + 1;
        }
        draw_x = draw_x + 1;
    }
    return ;
}

static void demo_draw_line(demo_image &image, int32_t start_x, int32_t start_y,
    int32_t end_x, int32_t end_y, uint32_t color)
{
    int32_t delta_x;
    int32_t delta_y;
    int32_t step_x;
    int32_t step_y;
    int32_t error_value;
    int32_t doubled_error;

    delta_x = std::abs(end_x - start_x);
    delta_y = std::abs(end_y - start_y);
    if (start_x < end_x)
    {
        step_x = 1;
    }
    else
    {
        step_x = -1;
    }
    if (start_y < end_y)
    {
        step_y = 1;
    }
    else
    {
        step_y = -1;
    }
    error_value = delta_x - delta_y;
    while (1)
    {
        if (start_x >= 0 && start_x < image.width
            && start_y >= 0 && start_y < image.height)
        {
            image.pixels[static_cast<ft_size_t>(
                (start_y * image.width) + start_x)] = color;
        }
        if (start_x == end_x && start_y == end_y)
        {
            break ;
        }
        doubled_error = error_value * 2;
        if (doubled_error > -delta_y)
        {
            error_value = error_value - delta_y;
            start_x = start_x + step_x;
        }
        if (doubled_error < delta_x)
        {
            error_value = error_value + delta_x;
            start_y = start_y + step_y;
        }
    }
    return ;
}

static void demo_draw_light_sprite(demo_image &image, int32_t center_x,
    int32_t center_y, int32_t sprite_radius, uint32_t light_color,
    double sprite_depth, const std::vector<double> &wall_depth_by_column)
{
    int32_t draw_x;
    int32_t draw_y;
    double normalized_x;
    double normalized_y;
    double distance_squared;
    uint32_t blend_amount;
    uint32_t base_color;
    uint32_t draw_color;
    ft_size_t pixel_index;

    draw_y = center_y - sprite_radius;
    while (draw_y <= center_y + sprite_radius)
    {
        if (draw_y >= 0 && draw_y < image.height)
        {
            draw_x = center_x - sprite_radius;
            while (draw_x <= center_x + sprite_radius)
            {
                if (draw_x >= 0 && draw_x < image.width
                    && sprite_depth < wall_depth_by_column[static_cast<ft_size_t>(
                            draw_x)])
                {
                    normalized_x = static_cast<double>(draw_x - center_x)
                        / static_cast<double>(sprite_radius);
                    normalized_y = static_cast<double>(draw_y - center_y)
                        / static_cast<double>(sprite_radius);
                    distance_squared = (normalized_x * normalized_x)
                        + (normalized_y * normalized_y);
                    if (distance_squared < 1.35)
                    {
                        draw_color = light_color;
                        if (distance_squared < 0.18)
                        {
                            blend_amount = 255U;
                            draw_color = demo_make_color(255U, 248U, 232U);
                        }
                        else
                        {
                            blend_amount = static_cast<uint32_t>(
                                190.0 * (1.35 - distance_squared) / 1.35);
                        }
                        pixel_index = static_cast<ft_size_t>(
                            (draw_y * image.width) + draw_x);
                        base_color = image.pixels[pixel_index];
                        image.pixels[pixel_index] = demo_apply_light_to_color(
                            base_color, draw_color, blend_amount);
                    }
                }
                draw_x = draw_x + 1;
            }
        }
        draw_y = draw_y + 1;
    }
    return ;
}

static void demo_draw_level_light_sprites(demo_image &image,
    const demo_game_state &game_state, const demo_level &level,
    const std::vector<double> &wall_depth_by_column)
{
    uint32_t light_index;
    double relative_x;
    double relative_y;
    double inverse_determinant;
    double transform_x;
    double transform_y;
    int32_t sprite_screen_x;
    int32_t sprite_size;
    int32_t sprite_radius;

    inverse_determinant = 1.0 / ((game_state.camera_plane_x
            * game_state.player_direction_y)
            - (game_state.player_direction_x * game_state.camera_plane_y));
    light_index = 0U;
    while (light_index < level.light_count)
    {
        relative_x = level.lights[light_index].position_x
            - game_state.player_position_x;
        relative_y = level.lights[light_index].position_y
            - game_state.player_position_y;
        transform_x = inverse_determinant * ((game_state.player_direction_y
                * relative_x) - (game_state.player_direction_x * relative_y));
        transform_y = inverse_determinant * ((-game_state.camera_plane_y
                * relative_x) + (game_state.camera_plane_x * relative_y));
        if (transform_y > 0.15)
        {
            sprite_screen_x = static_cast<int32_t>(
                (static_cast<double>(image.width) / 2.0)
                * (1.0 + (transform_x / transform_y)));
            sprite_size = static_cast<int32_t>(
                static_cast<double>(image.height) / transform_y);
            sprite_radius = demo_clamp_s32(sprite_size / 2, 6, 36);
            demo_draw_light_sprite(image, sprite_screen_x, image.height / 2,
                sprite_radius, level.lights[light_index].color, transform_y,
                wall_depth_by_column);
        }
        light_index = light_index + 1U;
    }
    return ;
}

static void demo_draw_3d_view(demo_image &image, const demo_game_state &game_state)
{
    const demo_level *level;
    int32_t screen_x;
    std::vector<double> wall_depth_by_column;

    level = &g_demo_levels[game_state.current_level_index];
    demo_draw_background_gradient(image);
    wall_depth_by_column.resize(static_cast<ft_size_t>(image.width), 1.0e30);
    screen_x = 0;
    while (screen_x < image.width)
    {
        int32_t ray_end_x;
        double screen_sample_x;
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
        int32_t wall_center_y;
        uint32_t wall_color;
        uint32_t distance_shade;
        double hit_position_x;
        double hit_position_y;

        ray_end_x = screen_x + DEMO_RAYCAST_COLUMN_STEP - 1;
        if (ray_end_x >= image.width)
        {
            ray_end_x = image.width - 1;
        }
        screen_sample_x = static_cast<double>(screen_x + ray_end_x) / 2.0;
        camera_x = (2.0 * screen_sample_x / static_cast<double>(image.width)) - 1.0;
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
        hit_position_x = game_state.player_position_x
            + (ray_direction_x * perpendicular_wall_distance);
        hit_position_y = game_state.player_position_y
            + (ray_direction_y * perpendicular_wall_distance);
        line_height = static_cast<int32_t>(
            static_cast<double>(image.height) / perpendicular_wall_distance);
        wall_center_y = image.height / 2;
        draw_start_y = demo_clamp_s32((-line_height / 2) + wall_center_y,
            0, image.height - 1);
        draw_end_y = demo_clamp_s32((line_height / 2) + wall_center_y,
            0, image.height - 1);
        wall_color = demo_make_color(212U, 180U, 96U);
        if (wall_side == 1)
        {
            wall_color = demo_shade_color(wall_color, 3U, 4U);
        }
        distance_shade = demo_min_u32(
            static_cast<uint32_t>(perpendicular_wall_distance * 32.0), 180U);
        wall_color = demo_shade_color(wall_color, 220U - distance_shade, 220U);
        wall_color = demo_apply_level_lighting(*level, wall_color, hit_position_x,
            hit_position_y);
        {
            int32_t depth_x;

            depth_x = screen_x;
            while (depth_x <= ray_end_x)
            {
                wall_depth_by_column[static_cast<ft_size_t>(depth_x)]
                    = perpendicular_wall_distance;
                depth_x = depth_x + 1;
            }
        }
        demo_draw_wall_column(image, screen_x, ray_end_x, draw_start_y, draw_end_y,
            wall_color);
        screen_x = ray_end_x + 1;
    }
    demo_draw_level_light_sprites(image, game_state, *level, wall_depth_by_column);
    return ;
}

static void demo_rebuild_minimap_cache(const demo_level &level)
{
    int32_t tile_x;
    int32_t tile_y;
    int32_t tile_size;
    int32_t pixel_x;
    int32_t pixel_y;
    uint32_t color;

    tile_size = 8;
    g_demo_cached_minimap_pixels.resize(static_cast<ft_size_t>(
        level.width * tile_size * level.height * tile_size), 0U);
    tile_y = 0;
    while (tile_y < level.height)
    {
        tile_x = 0;
        while (tile_x < level.width)
        {
            if (demo_level_tile_at(level, tile_x, tile_y) == '1')
            {
                color = demo_make_color(82U, 66U, 52U);
            }
            else if (demo_level_tile_at(level, tile_x, tile_y) == 'E')
            {
                color = demo_make_color(50U, 180U, 110U);
            }
            else
            {
                color = demo_make_color(26U, 28U, 38U);
            }
            pixel_y = 0;
            while (pixel_y < tile_size - 1)
            {
                pixel_x = 0;
                while (pixel_x < tile_size - 1)
                {
                    g_demo_cached_minimap_pixels[static_cast<ft_size_t>(
                        (((tile_y * tile_size) + pixel_y) * (level.width * tile_size))
                        + ((tile_x * tile_size) + pixel_x))] = color;
                    pixel_x = pixel_x + 1;
                }
                pixel_y = pixel_y + 1;
            }
            tile_x = tile_x + 1;
        }
        tile_y = tile_y + 1;
    }
    return ;
}

static void demo_draw_minimap(demo_image &image, const demo_game_state &game_state)
{
    const demo_level *level;
    int32_t tile_size;
    int32_t map_origin_x;
    int32_t map_origin_y;
    int32_t player_x;
    int32_t player_y;
    int32_t line_end_x;
    int32_t line_end_y;
    int32_t cache_width;
    int32_t cache_height;
    int32_t copy_row;
    ft_size_t copy_width_bytes;
    uint32_t light_index;
    int32_t light_x;
    int32_t light_y;
    char light_label[3];

    level = &g_demo_levels[game_state.current_level_index];
    tile_size = 8;
    map_origin_x = 12;
    map_origin_y = 12;
    if (g_demo_cached_minimap_level_index != game_state.current_level_index)
    {
        demo_rebuild_minimap_cache(*level);
        g_demo_cached_minimap_level_index = game_state.current_level_index;
    }
    demo_draw_rectangle(image, map_origin_x - 4, map_origin_y - 4,
        (level->width * tile_size) + 8, (level->height * tile_size) + 8,
        demo_make_color(10U, 10U, 16U));
    cache_width = level->width * tile_size;
    cache_height = level->height * tile_size;
    copy_width_bytes = static_cast<ft_size_t>(cache_width) * sizeof(uint32_t);
    copy_row = 0;
    while (copy_row < cache_height)
    {
        std::memcpy(&image.pixels[static_cast<ft_size_t>(
                ((map_origin_y + copy_row) * image.width) + map_origin_x)],
            &g_demo_cached_minimap_pixels[static_cast<ft_size_t>(copy_row * cache_width)],
            copy_width_bytes);
        copy_row = copy_row + 1;
    }
    player_x = map_origin_x
        + static_cast<int32_t>(game_state.player_position_x * static_cast<double>(tile_size));
    player_y = map_origin_y
        + static_cast<int32_t>(game_state.player_position_y * static_cast<double>(tile_size));
    demo_draw_rectangle(image, player_x - 2, player_y - 2, 5, 5,
        demo_make_color(255U, 80U, 64U));
    line_end_x = player_x + static_cast<int32_t>(game_state.player_direction_x * 6.0);
    line_end_y = player_y + static_cast<int32_t>(game_state.player_direction_y * 6.0);
    demo_draw_line(image, player_x, player_y, line_end_x, line_end_y,
        demo_make_color(255U, 255U, 255U));
    light_index = 0U;
    while (light_index < level->light_count)
    {
        light_x = map_origin_x + static_cast<int32_t>(
            level->lights[light_index].position_x * static_cast<double>(tile_size));
        light_y = map_origin_y + static_cast<int32_t>(
            level->lights[light_index].position_y * static_cast<double>(tile_size));
        demo_draw_light_marker(image, light_x, light_y,
            level->lights[light_index].color);
        light_label[0] = 'L';
        light_label[1] = static_cast<char>('1' + light_index);
        light_label[2] = '\0';
        demo_draw_text(image, light_x + 8, light_y - 4, light_label, 1,
            demo_make_color(255U, 248U, 228U));
        light_index = light_index + 1U;
    }
    demo_draw_text(image, map_origin_x, map_origin_y + cache_height + 10,
        "L1 L2 L3 = LIGHTS", 1, demo_make_color(245U, 240U, 210U));
    return ;
}

static void demo_draw_playing_hud(demo_image &image, const demo_game_state &game_state)
{
    char time_buffer[32];
    char level_buffer[64];
    char fps_buffer[32];
    char average_fps_buffer[32];
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
    std::snprintf(fps_buffer, sizeof(fps_buffer), "FPS %u",
        game_state.displayed_fps);
    std::snprintf(average_fps_buffer, sizeof(average_fps_buffer), "AVG10 %u",
        game_state.average_fps_10_seconds);
    demo_draw_text(image, 12, image.height - 50, level_buffer, 2,
        demo_make_color(255U, 232U, 180U));
    demo_draw_text(image, 12, image.height - 28, time_buffer, 2,
        demo_make_color(210U, 230U, 255U));
    demo_draw_text(image, image.width - 110, 14, fps_buffer, 2,
        demo_make_color(255U, 236U, 166U));
    demo_draw_text(image, image.width - 146, 36, average_fps_buffer, 2,
        demo_make_color(210U, 230U, 255U));
    demo_draw_text(image, image.width - 170, image.height - 28,
        "W S MOVE", 2, demo_make_color(220U, 220U, 220U));
    demo_draw_text(image, image.width - 170, image.height - 50,
        "A D TURN", 2, demo_make_color(220U, 220U, 220U));
    return ;
}

static void demo_draw_menu(demo_image &image, const demo_game_state &game_state)
{
    const char *options[3];
    int32_t option_index;
    uint32_t color;

    options[0] = "START RUN";
    options[1] = "LEADERBOARD";
    options[2] = "QUIT";
    demo_draw_background_gradient(image);
    demo_draw_text(image, 86, 46, "DUMB MAZE 3D", 4,
        demo_make_color(255U, 220U, 120U));
    demo_draw_text(image, 110, 96, "SOFTWARE CRAWLER", 2,
        demo_make_color(205U, 220U, 255U));
    demo_draw_text(image, 136, 126, "FOUR LARGER MAPS", 2,
        demo_make_color(205U, 220U, 255U));
    option_index = 0;
    while (option_index < 3)
    {
        color = demo_make_color(200U, 200U, 200U);
        if (option_index == game_state.selected_menu_index)
        {
            demo_draw_rectangle(image, 132, 166 + (option_index * 42), 272, 28,
                demo_make_color(60U, 78U, 120U));
            color = demo_make_color(255U, 240U, 180U);
        }
        demo_draw_text(image, 150, 170 + (option_index * 42),
            options[option_index], 2, color);
        option_index = option_index + 1;
    }
    demo_draw_text(image, 118, image.height - 46, "ARROWS OR WASD THEN ENTER", 2,
        demo_make_color(190U, 200U, 220U));
    return ;
}

static void demo_draw_level_clear(demo_image &image,
    const demo_game_state &game_state)
{
    char level_buffer[64];

    demo_draw_background_gradient(image);
    demo_draw_text(image, 122, 82, "LEVEL CLEAR", 4,
        demo_make_color(255U, 226U, 122U));
    std::snprintf(level_buffer, sizeof(level_buffer), "NEXT %s",
        g_demo_levels[game_state.pending_level_index].name);
    demo_draw_text(image, 160, 148, level_buffer, 2,
        demo_make_color(220U, 235U, 255U));
    demo_draw_text(image, 136, 208, "ENTER FOR NEXT LEVEL", 2,
        demo_make_color(255U, 255U, 255U));
    demo_draw_text(image, 170, 236, "ESC FOR MENU", 2,
        demo_make_color(190U, 200U, 220U));
    return ;
}

static void demo_draw_leaderboard(demo_image &image,
    const demo_leaderboard &leaderboard)
{
    uint32_t index_entry;
    char line_buffer[64];
    char time_buffer[32];

    demo_draw_background_gradient(image);
    demo_draw_text(image, 112, 44, "LEADERBOARD", 4,
        demo_make_color(255U, 220U, 120U));
    if (leaderboard.entry_count == 0U)
    {
        demo_draw_text(image, 176, 146, "NO RUNS YET", 2,
            demo_make_color(220U, 230U, 255U));
    }
    index_entry = 0U;
    while (index_entry < leaderboard.entry_count)
    {
        demo_format_time(time_buffer, sizeof(time_buffer),
            leaderboard.entries[index_entry].elapsed_milliseconds);
        std::snprintf(line_buffer, sizeof(line_buffer), "%u  %s  %s",
            index_entry + 1U, leaderboard.entries[index_entry].name, time_buffer);
        demo_draw_text(image, 150, 116 + static_cast<int32_t>(index_entry * 30U),
            line_buffer, 2, demo_make_color(240U, 240U, 240U));
        index_entry = index_entry + 1U;
    }
    demo_draw_text(image, 116, image.height - 44, "ENTER OR ESC TO RETURN", 2,
        demo_make_color(190U, 200U, 220U));
    return ;
}

static void demo_draw_name_entry(demo_image &image,
    const demo_game_state &game_state)
{
    char time_buffer[32];
    char letter_buffer[2];
    uint32_t index_letter;
    int32_t position_x;

    demo_draw_background_gradient(image);
    demo_draw_text(image, 88, 44, "YOU CLEARED THE MAZE", 3,
        demo_make_color(255U, 220U, 120U));
    demo_format_time(time_buffer, sizeof(time_buffer),
        game_state.total_elapsed_milliseconds);
    demo_draw_text(image, 198, 96, time_buffer, 3,
        demo_make_color(220U, 235U, 255U));
    demo_draw_text(image, 160, 146, "ENTER YOUR TAG", 2,
        demo_make_color(255U, 255U, 255U));
    index_letter = 0U;
    while (index_letter < 3U)
    {
        position_x = 220 + static_cast<int32_t>(index_letter * 48U);
        if (index_letter == game_state.pending_name_index)
        {
            demo_draw_rectangle(image, position_x - 8, 178, 36, 56,
                demo_make_color(66U, 86U, 130U));
        }
        letter_buffer[0] = game_state.pending_name[index_letter];
        letter_buffer[1] = '\0';
        demo_draw_text(image, position_x, 190, letter_buffer, 4,
            demo_make_color(255U, 236U, 166U));
        index_letter = index_letter + 1U;
    }
    demo_draw_text(image, 92, image.height - 58,
        "UP DOWN CHANGE  LEFT RIGHT SELECT", 2,
        demo_make_color(200U, 210U, 225U));
    demo_draw_text(image, 160, image.height - 32, "ENTER TO SAVE", 2,
        demo_make_color(200U, 210U, 225U));
    return ;
}

void demo_draw_frame(ft_render_window &render_window,
    const demo_game_state &game_state, const demo_leaderboard &leaderboard)
{
    ft_render_framebuffer &framebuffer = render_window.framebuffer();
    demo_image            frame_image;

    frame_image = demo_prepare_frame_image(framebuffer);

    if (game_state.mode == DEMO_MODE_MENU)
    {
        demo_draw_menu(frame_image, game_state);
    }
    else if (game_state.mode == DEMO_MODE_PLAYING)
    {
        demo_draw_3d_view(frame_image, game_state);
        demo_draw_minimap(frame_image, game_state);
        demo_draw_playing_hud(frame_image, game_state);
    }
    else if (game_state.mode == DEMO_MODE_LEVEL_CLEAR)
    {
        demo_draw_level_clear(frame_image, game_state);
    }
    else if (game_state.mode == DEMO_MODE_LEADERBOARD)
    {
        demo_draw_leaderboard(frame_image, leaderboard);
    }
    else if (game_state.mode == DEMO_MODE_NAME_ENTRY)
    {
        demo_draw_name_entry(frame_image, game_state);
    }
    else
    {
        demo_draw_background_gradient(frame_image);
    }
    return ;
}

void demo_choose_window_size(ft_render_window_desc *window_desc)
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
