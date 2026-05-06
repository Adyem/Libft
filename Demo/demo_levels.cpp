#include "demo_game.hpp"

#include "../Modules/CPP_class/class_nullptr.hpp"
#include "../Modules/Errno/errno.hpp"
#include <cstdio>
#include <vector>

static const char g_demo_level_tiles_0[] =
    "1111111111111111"
    "1000000000000001"
    "1011110111111101"
    "1010000100000101"
    "1010111101110101"
    "1010100001010101"
    "1010101111010101"
    "1010101000010101"
    "1010101011110101"
    "1010001010000101"
    "1011101010111101"
    "1000001010000001"
    "1111101010111111"
    "1000100010100001"
    "10100000000000E1"
    "1111111111111111";

static const char g_demo_level_tiles_1[] =
    "1111111111111111"
    "1000001000000001"
    "1011101011111101"
    "1010101010000101"
    "1010101010110101"
    "1010100010010101"
    "1010111111010101"
    "1010000000010101"
    "1011111111010101"
    "1000000010010101"
    "1111111011010101"
    "1000000011010101"
    "1011111110010101"
    "1010000000000101"
    "10101111111110E1"
    "1111111111111111";

static const char g_demo_level_tiles_2[] =
    "1111111111111111"
    "1000000001000001"
    "1011111101011101"
    "1000000101010001"
    "1111110101010111"
    "1000010100010001"
    "1011010111111101"
    "1010010000000101"
    "1010011111110101"
    "1010000000010101"
    "1011111111010101"
    "1000001000010101"
    "1111101011110101"
    "1000001010000101"
    "10111110101110E1"
    "1111111111111111";

static const char g_demo_level_tiles_3[] =
    "1111111111111111"
    "1000000000000001"
    "1011111111111101"
    "1010000000000101"
    "1010111111110101"
    "1010100000010101"
    "1010101111010101"
    "1010101001010101"
    "1010001011010001"
    "1011101010011111"
    "1000101010000001"
    "1110101011111101"
    "1000101000000101"
    "1011101111110101"
    "10000000000000E1"
    "1111111111111111";

static const demo_level::demo_light g_demo_level_lights_0[] =
{
    { 13.5, 1.5, 4.6, 0xFFD38CU, 190U },
    { 11.5, 3.5, 3.8, 0x7CC8FFU, 160U },
    { 14.5, 14.5, 5.8, 0xFFC06BU, 245U }
};

static const demo_level::demo_light g_demo_level_lights_1[] =
{
    { 5.5, 1.5, 4.2, 0xFFD39AU, 180U },
    { 11.5, 7.5, 4.4, 0x88B8FFU, 170U },
    { 13.5, 13.5, 5.0, 0xFFB45AU, 220U }
};

static const demo_level::demo_light g_demo_level_lights_2[] =
{
    { 2.5, 3.5, 4.6, 0xFFD18AU, 185U },
    { 9.5, 7.5, 4.0, 0x8FD6FFU, 155U },
    { 13.5, 13.5, 4.6, 0xFFAA52U, 215U }
};

static const demo_level::demo_light g_demo_level_lights_3[] =
{
    { 13.5, 1.5, 4.2, 0xFFC978U, 180U },
    { 11.5, 5.5, 4.2, 0xA1C8FFU, 175U },
    { 13.5, 14.5, 5.0, 0xFF9E48U, 225U }
};

const demo_level g_demo_levels[] =
{
    {
        "ENTRY HALL",
        g_demo_level_tiles_0,
        16,
        16,
        1.5,
        1.5,
        1.0,
        0.0,
        g_demo_level_lights_0,
        sizeof(g_demo_level_lights_0) / sizeof(g_demo_level_lights_0[0])
    },
    {
        "BRICK LOOP",
        g_demo_level_tiles_1,
        16,
        16,
        1.5,
        1.5,
        1.0,
        0.0,
        g_demo_level_lights_1,
        sizeof(g_demo_level_lights_1) / sizeof(g_demo_level_lights_1[0])
    },
    {
        "LOW VAULT",
        g_demo_level_tiles_2,
        16,
        16,
        1.5,
        1.5,
        1.0,
        0.0,
        g_demo_level_lights_2,
        sizeof(g_demo_level_lights_2) / sizeof(g_demo_level_lights_2[0])
    },
    {
        "FINAL SPIRAL",
        g_demo_level_tiles_3,
        16,
        16,
        1.5,
        1.5,
        1.0,
        0.0,
        g_demo_level_lights_3,
        sizeof(g_demo_level_lights_3) / sizeof(g_demo_level_lights_3[0])
    }
};

const uint32_t g_demo_level_count = sizeof(g_demo_levels) / sizeof(g_demo_levels[0]);

ft_bool demo_level_is_wall(const demo_level &level, int32_t tile_x, int32_t tile_y)
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

char demo_level_tile_at(const demo_level &level, int32_t tile_x, int32_t tile_y)
{
    int32_t tile_index;

    if (tile_x < 0 || tile_y < 0 || tile_x >= level.width || tile_y >= level.height)
    {
        return ('1');
    }
    tile_index = (tile_y * level.width) + tile_x;
    return (level.tiles[tile_index]);
}

static int32_t demo_validate_level_tiles(const demo_level &level,
    int32_t *exit_x_pointer, int32_t *exit_y_pointer)
{
    int32_t tile_x;
    int32_t tile_y;
    int32_t exit_count;
    char    tile_value;

    exit_count = 0;
    tile_y = 0;
    while (tile_y < level.height)
    {
        tile_x = 0;
        while (tile_x < level.width)
        {
            tile_value = demo_level_tile_at(level, tile_x, tile_y);
            if (tile_value != '0' && tile_value != '1' && tile_value != 'E')
            {
                return (FT_ERR_CONFIGURATION);
            }
            if ((tile_y == 0 || tile_y == level.height - 1
                    || tile_x == 0 || tile_x == level.width - 1)
                && tile_value != '1' && tile_value != 'E')
            {
                return (FT_ERR_CONFIGURATION);
            }
            if (tile_value == 'E')
            {
                *exit_x_pointer = tile_x;
                *exit_y_pointer = tile_y;
                exit_count = exit_count + 1;
            }
            tile_x = tile_x + 1;
        }
        tile_y = tile_y + 1;
    }
    if (exit_count != 1)
    {
        return (FT_ERR_CONFIGURATION);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t demo_validate_level_reachability(const demo_level &level,
    int32_t start_x, int32_t start_y, int32_t exit_x, int32_t exit_y)
{
    std::vector<uint8_t> visited;
    std::vector<int32_t> queue_x;
    std::vector<int32_t> queue_y;
    uint32_t             head_index;
    uint32_t             tail_index;
    int32_t              tile_x;
    int32_t              tile_y;
    int32_t              index_flat;

    visited.resize(static_cast<ft_size_t>(level.width * level.height), 0U);
    queue_x.resize(static_cast<ft_size_t>(level.width * level.height), 0);
    queue_y.resize(static_cast<ft_size_t>(level.width * level.height), 0);
    head_index = 0U;
    tail_index = 1U;
    queue_x[0] = start_x;
    queue_y[0] = start_y;
    visited[static_cast<ft_size_t>((start_y * level.width) + start_x)] = 1U;
    while (head_index < tail_index)
    {
        tile_x = queue_x[head_index];
        tile_y = queue_y[head_index];
        head_index = head_index + 1U;
        if (tile_x == exit_x && tile_y == exit_y)
        {
            return (FT_ERR_SUCCESS);
        }
        index_flat = (tile_y * level.width) + (tile_x + 1);
        if (tile_x + 1 < level.width && demo_level_is_wall(level, tile_x + 1, tile_y) == FT_FALSE
            && visited[static_cast<ft_size_t>(index_flat)] == 0U)
        {
            visited[static_cast<ft_size_t>(index_flat)] = 1U;
            queue_x[tail_index] = tile_x + 1;
            queue_y[tail_index] = tile_y;
            tail_index = tail_index + 1U;
        }
        index_flat = (tile_y * level.width) + (tile_x - 1);
        if (tile_x - 1 >= 0 && demo_level_is_wall(level, tile_x - 1, tile_y) == FT_FALSE
            && visited[static_cast<ft_size_t>(index_flat)] == 0U)
        {
            visited[static_cast<ft_size_t>(index_flat)] = 1U;
            queue_x[tail_index] = tile_x - 1;
            queue_y[tail_index] = tile_y;
            tail_index = tail_index + 1U;
        }
        index_flat = ((tile_y + 1) * level.width) + tile_x;
        if (tile_y + 1 < level.height && demo_level_is_wall(level, tile_x, tile_y + 1) == FT_FALSE
            && visited[static_cast<ft_size_t>(index_flat)] == 0U)
        {
            visited[static_cast<ft_size_t>(index_flat)] = 1U;
            queue_x[tail_index] = tile_x;
            queue_y[tail_index] = tile_y + 1;
            tail_index = tail_index + 1U;
        }
        index_flat = ((tile_y - 1) * level.width) + tile_x;
        if (tile_y - 1 >= 0 && demo_level_is_wall(level, tile_x, tile_y - 1) == FT_FALSE
            && visited[static_cast<ft_size_t>(index_flat)] == 0U)
        {
            visited[static_cast<ft_size_t>(index_flat)] = 1U;
            queue_x[tail_index] = tile_x;
            queue_y[tail_index] = tile_y - 1;
            tail_index = tail_index + 1U;
        }
    }
    return (FT_ERR_CONFIGURATION);
}

static int32_t demo_validate_one_level(const demo_level &level)
{
    int32_t start_x;
    int32_t start_y;
    int32_t exit_x;
    int32_t exit_y;
    int32_t validate_error;

    if (level.name == ft_nullptr || level.tiles == ft_nullptr
        || level.width <= 0 || level.height <= 0)
    {
        return (FT_ERR_CONFIGURATION);
    }
    start_x = static_cast<int32_t>(level.start_position_x);
    start_y = static_cast<int32_t>(level.start_position_y);
    if (start_x < 0 || start_y < 0 || start_x >= level.width || start_y >= level.height)
    {
        return (FT_ERR_CONFIGURATION);
    }
    if (demo_level_is_wall(level, start_x, start_y) == FT_TRUE)
    {
        return (FT_ERR_CONFIGURATION);
    }
    exit_x = -1;
    exit_y = -1;
    validate_error = demo_validate_level_tiles(level, &exit_x, &exit_y);
    if (validate_error != FT_ERR_SUCCESS)
    {
        return (validate_error);
    }
    return (demo_validate_level_reachability(level, start_x, start_y, exit_x, exit_y));
}

int32_t demo_validate_levels(void)
{
    uint32_t index_level;
    int32_t  validate_error;

    index_level = 0U;
    while (index_level < g_demo_level_count)
    {
        validate_error = demo_validate_one_level(g_demo_levels[index_level]);
        if (validate_error != FT_ERR_SUCCESS)
        {
            std::fprintf(stderr, "Demo level validation failed for level %u (%s)\n",
                index_level + 1U, g_demo_levels[index_level].name);
            return (validate_error);
        }
        index_level = index_level + 1U;
    }
    return (FT_ERR_SUCCESS);
}
