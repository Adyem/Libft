#include <stdint.h>
#include "../Basic/class_nullptr.hpp"
#include "voxel.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../RNG/rng.hpp"
#include "../Game/game_voxel_chunk.hpp"
#include "voxel_internal.hpp"

static void terrain_abort_unknown_block_id(uint32_t block_id,
    const char *method_name) noexcept
{
    char decimal_buffer[10];
    ft_size_t digit_count;
    int32_t write_error;

    write_error = errno_write_stderr("terrain error: method=");
    if (write_error != FT_ERR_SUCCESS)
    {
        su_abort();
        return ;
    }
    write_error = errno_write_stderr(method_name);
    if (write_error != FT_ERR_SUCCESS)
    {
        su_abort();
        return ;
    }
    write_error = errno_write_stderr(" unknown block id=");
    if (write_error != FT_ERR_SUCCESS)
    {
        su_abort();
        return ;
    }
    if (block_id == 0U)
    {
        if (errno_write_stderr("0") != FT_ERR_SUCCESS)
        {
            su_abort();
            return ;
        }
    }
    else
    {
        digit_count = 0;
        while (block_id > 0U && digit_count < sizeof(decimal_buffer))
        {
            decimal_buffer[digit_count] = static_cast<char>('0'
                + (block_id % 10U));
            block_id /= 10U;
            digit_count++;
        }
        while (digit_count > 0U)
        {
            digit_count--;
            if (su_write(2, &decimal_buffer[digit_count], 1U) != 1)
            {
                su_abort();
                return ;
            }
        }
    }
    if (errno_write_stderr("\n") != FT_ERR_SUCCESS)
    {
        su_abort();
        return ;
    }
    su_abort();
    return ;
}

static const terrain_block_metadata TERRAIN_BLOCK_REGISTRY[] =
{
    {FT_FALSE, FT_TRUE, FT_FALSE, FT_TRUE, FT_FALSE, FT_FALSE, 0U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 1U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 2U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 4U, FT_TRUE},
    {FT_FALSE, FT_TRUE, FT_FALSE, FT_TRUE, FT_FALSE, FT_TRUE, 1U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 3U, FT_TRUE},
    {FT_FALSE, FT_TRUE, FT_FALSE, FT_TRUE, FT_FALSE, FT_TRUE, 1U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 2U, FT_TRUE},
    {FT_FALSE, FT_TRUE, FT_TRUE, FT_TRUE, FT_FALSE, FT_FALSE, 0U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 255U, FT_FALSE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 1U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 1U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 3U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 3U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 3U, FT_TRUE},
    {FT_TRUE, FT_FALSE, FT_FALSE, FT_FALSE, FT_FALSE, FT_TRUE, 3U, FT_TRUE}
};

static const terrain_tree_template_block TERRAIN_SMALL_OAK_TREE_BLOCKS[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-1, 2, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 2, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 2, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 2, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 2, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 2, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 2, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 2, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_SMALL_PINE_TREE_BLOCKS[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-1, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 6, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_SMALL_CACTUS_TREE_BLOCKS[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_CACTUS_BLOCK}
};

static const terrain_tree_template_block TERRAIN_LARGE_OAK_TREE_BLOCKS[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-2, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -2, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 2, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {2, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 6, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_LARGE_OAK_TREE_BLOCKS_VARIANT_1[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-2, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -2, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 2, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {2, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 6, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_LARGE_PINE_TREE_BLOCKS[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 6, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 6, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 6, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 6, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 6, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 7, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_LARGE_PINE_TREE_BLOCKS_VARIANT_1[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-1, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 6, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 7, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 8, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template TERRAIN_SMALL_OAK_TREE_TEMPLATE =
{
    TERRAIN_SMALL_OAK_TREE_BLOCKS,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_OAK_TREE_BLOCKS)
        / sizeof(TERRAIN_SMALL_OAK_TREE_BLOCKS[0]))
};

static const terrain_tree_template TERRAIN_SMALL_PINE_TREE_TEMPLATE =
{
    TERRAIN_SMALL_PINE_TREE_BLOCKS,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_PINE_TREE_BLOCKS)
        / sizeof(TERRAIN_SMALL_PINE_TREE_BLOCKS[0]))
};

static const terrain_tree_template TERRAIN_SMALL_CACTUS_TREE_TEMPLATE =
{
    TERRAIN_SMALL_CACTUS_TREE_BLOCKS,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_CACTUS_TREE_BLOCKS)
        / sizeof(TERRAIN_SMALL_CACTUS_TREE_BLOCKS[0]))
};

static const terrain_tree_template TERRAIN_LARGE_OAK_TREE_TEMPLATE =
{
    TERRAIN_LARGE_OAK_TREE_BLOCKS,
    static_cast<uint32_t>(sizeof(TERRAIN_LARGE_OAK_TREE_BLOCKS)
        / sizeof(TERRAIN_LARGE_OAK_TREE_BLOCKS[0]))
};

static const terrain_tree_template TERRAIN_LARGE_OAK_TREE_TEMPLATE_VARIANT_1 =
{
    TERRAIN_LARGE_OAK_TREE_BLOCKS_VARIANT_1,
    static_cast<uint32_t>(sizeof(TERRAIN_LARGE_OAK_TREE_BLOCKS_VARIANT_1)
        / sizeof(TERRAIN_LARGE_OAK_TREE_BLOCKS_VARIANT_1[0]))
};

static const terrain_tree_template TERRAIN_LARGE_PINE_TREE_TEMPLATE =
{
    TERRAIN_LARGE_PINE_TREE_BLOCKS,
    static_cast<uint32_t>(sizeof(TERRAIN_LARGE_PINE_TREE_BLOCKS)
        / sizeof(TERRAIN_LARGE_PINE_TREE_BLOCKS[0]))
};

static const terrain_tree_template TERRAIN_LARGE_PINE_TREE_TEMPLATE_VARIANT_1 =
{
    TERRAIN_LARGE_PINE_TREE_BLOCKS_VARIANT_1,
    static_cast<uint32_t>(sizeof(TERRAIN_LARGE_PINE_TREE_BLOCKS_VARIANT_1)
        / sizeof(TERRAIN_LARGE_PINE_TREE_BLOCKS_VARIANT_1[0]))
};

static const terrain_tree_template_block TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_1[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-2, 2, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 2, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 2, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 2, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 2, -2, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 2, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 2, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 2, 2, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 2, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 2, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 2, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {2, 2, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_2[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-1, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_1[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 3, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 3, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_2[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 4, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {0, 5, 0, TERRAIN_GENERATOR_OAK_LOG_BLOCK},
    {-1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {1, 4, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, -1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 4, 1, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK},
    {0, 6, 0, TERRAIN_GENERATOR_OAK_LEAVES_BLOCK}
};

static const terrain_tree_template_block TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_1[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {1, 2, 0, TERRAIN_GENERATOR_CACTUS_BLOCK}
};

static const terrain_tree_template_block TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_2[] =
{
    {0, 0, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 1, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 2, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {0, 3, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {-1, 1, 0, TERRAIN_GENERATOR_CACTUS_BLOCK},
    {1, 2, 0, TERRAIN_GENERATOR_CACTUS_BLOCK}
};

static const terrain_tree_template TERRAIN_SMALL_OAK_TREE_TEMPLATE_VARIANT_1 =
{
    TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_1,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_1)
        / sizeof(TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_1[0]))
};

static const terrain_tree_template TERRAIN_SMALL_OAK_TREE_TEMPLATE_VARIANT_2 =
{
    TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_2,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_2)
        / sizeof(TERRAIN_SMALL_OAK_TREE_BLOCKS_VARIANT_2[0]))
};

static const terrain_tree_template TERRAIN_SMALL_PINE_TREE_TEMPLATE_VARIANT_1 =
{
    TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_1,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_1)
        / sizeof(TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_1[0]))
};

static const terrain_tree_template TERRAIN_SMALL_PINE_TREE_TEMPLATE_VARIANT_2 =
{
    TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_2,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_2)
        / sizeof(TERRAIN_SMALL_PINE_TREE_BLOCKS_VARIANT_2[0]))
};

static const terrain_tree_template TERRAIN_SMALL_CACTUS_TREE_TEMPLATE_VARIANT_1 =
{
    TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_1,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_1)
        / sizeof(TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_1[0]))
};

static const terrain_tree_template TERRAIN_SMALL_CACTUS_TREE_TEMPLATE_VARIANT_2 =
{
    TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_2,
    static_cast<uint32_t>(sizeof(TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_2)
        / sizeof(TERRAIN_SMALL_CACTUS_TREE_BLOCKS_VARIANT_2[0]))
};

int32_t terrain_floor_div(int32_t value, int32_t divisor) noexcept
{
    int32_t quotient;
    int32_t remainder;

    quotient = value / divisor;
    remainder = value % divisor;
    if (remainder < 0)
        quotient -= 1;
    return (quotient);
}

uint64_t terrain_mix_u64(uint64_t value) noexcept
{
    value ^= value >> 30;
    value *= UINT64_C(0xBF58476D1CE4E5B9);
    value ^= value >> 27;
    value *= UINT64_C(0x94D049BB133111EB);
    value ^= value >> 31;
    return (value);
}

double terrain_lerp(double left_value, double right_value,
    double factor) noexcept
{
    return (left_value + ((right_value - left_value) * factor));
}

double terrain_smooth_factor(double factor) noexcept
{
    return (factor * factor * (3.0 - (2.0 * factor)));
}

uint64_t terrain_seed_value(const char *seed_string) noexcept
{
    return (static_cast<uint64_t>(rng_seed_value(seed_string)));
}

uint64_t terrain_feature_seed(uint64_t seed_value, int32_t world_block_x,
    int32_t world_block_z, uint64_t salt) noexcept
{
    uint64_t feature_seed;

    feature_seed = terrain_mix_u64(seed_value ^ salt
        ^ (static_cast<uint64_t>(static_cast<int64_t>(world_block_x))
            * UINT64_C(0x9E3779B97F4A7C15))
        ^ (static_cast<uint64_t>(static_cast<int64_t>(world_block_z))
            * UINT64_C(0xBF58476D1CE4E5B9)));
    return (feature_seed);
}

double terrain_signed_unit_noise(uint64_t seed_value, int32_t grid_x,
    int32_t grid_z) noexcept
{
    uint64_t mixed_value;

    mixed_value = terrain_mix_u64(seed_value
        ^ (static_cast<uint64_t>(static_cast<int64_t>(grid_x))
            * UINT64_C(0x9E3779B97F4A7C15))
        ^ (static_cast<uint64_t>(static_cast<int64_t>(grid_z))
            * UINT64_C(0xBF58476D1CE4E5B9)));
    mixed_value = terrain_mix_u64(mixed_value);
    return (static_cast<double>(mixed_value >> 11)
        * (1.0 / 9007199254740992.0) * 2.0 - 1.0);
}

double terrain_value_noise(uint64_t seed_value, int32_t world_block_x,
    int32_t world_block_z, int32_t scale) noexcept
{
    int32_t grid_x0;
    int32_t grid_z0;
    int32_t grid_x1;
    int32_t grid_z1;
    int32_t local_x;
    int32_t local_z;
    double factor_x;
    double factor_z;
    double smooth_x;
    double smooth_z;
    double noise_top;
    double noise_bottom;
    double noise_left;
    double noise_right;
    double noise_value;

    grid_x0 = terrain_floor_div(world_block_x, scale);
    grid_z0 = terrain_floor_div(world_block_z, scale);
    grid_x1 = grid_x0 + 1;
    grid_z1 = grid_z0 + 1;
    local_x = world_block_x - (grid_x0 * scale);
    local_z = world_block_z - (grid_z0 * scale);
    factor_x = static_cast<double>(local_x) / static_cast<double>(scale);
    factor_z = static_cast<double>(local_z) / static_cast<double>(scale);
    smooth_x = terrain_smooth_factor(factor_x);
    smooth_z = terrain_smooth_factor(factor_z);
    noise_left = terrain_signed_unit_noise(seed_value, grid_x0, grid_z0);
    noise_right = terrain_signed_unit_noise(seed_value, grid_x1, grid_z0);
    noise_top = terrain_lerp(noise_left, noise_right, smooth_x);
    noise_left = terrain_signed_unit_noise(seed_value, grid_x0, grid_z1);
    noise_right = terrain_signed_unit_noise(seed_value, grid_x1, grid_z1);
    noise_bottom = terrain_lerp(noise_left, noise_right, smooth_x);
    noise_value = terrain_lerp(noise_top, noise_bottom, smooth_z);
    return (noise_value);
}

terrain_biome terrain_pick_biome(uint64_t seed_value,
    int32_t world_block_x, int32_t world_block_z) noexcept
{
    int32_t biome_zone_x;
    int32_t biome_zone_z;
    int64_t biome_selector;

    biome_zone_x = terrain_floor_div(world_block_x, TERRAIN_BIOME_ZONE_WIDTH);
    biome_zone_z = terrain_floor_div(world_block_z, TERRAIN_BIOME_ZONE_WIDTH);
    biome_selector = static_cast<int64_t>(seed_value % 5U)
        + static_cast<int64_t>(biome_zone_x)
        + static_cast<int64_t>(biome_zone_z);
    biome_selector %= 5;
    if (biome_selector < 0)
        biome_selector += 5;
    if (biome_selector == 0)
        return (TERRAIN_BIOME_PLAINS);
    if (biome_selector == 1)
        return (TERRAIN_BIOME_HILLS);
    if (biome_selector == 2)
        return (TERRAIN_BIOME_DESERT);
    if (biome_selector == 3)
        return (TERRAIN_BIOME_SNOW);
    return (TERRAIN_BIOME_MOUNTAINS);
}

uint32_t terrain_surface_block_for_biome(terrain_biome biome) noexcept
{
    if (biome == TERRAIN_BIOME_HILLS)
        return (TERRAIN_GENERATOR_MOSS_ROCK_BLOCK);
    if (biome == TERRAIN_BIOME_DESERT)
        return (TERRAIN_GENERATOR_SAND_BLOCK);
    if (biome == TERRAIN_BIOME_SNOW)
        return (TERRAIN_GENERATOR_SNOW_BLOCK);
    if (biome == TERRAIN_BIOME_MOUNTAINS)
        return (TERRAIN_GENERATOR_SLATE_BLOCK);
    return (TERRAIN_GENERATOR_GRASS_BLOCK);
}

uint32_t terrain_subsurface_block_for_biome(terrain_biome biome) noexcept
{
    if (biome == TERRAIN_BIOME_HILLS)
        return (TERRAIN_GENERATOR_MOSS_ROCK_BLOCK);
    if (biome == TERRAIN_BIOME_DESERT)
        return (TERRAIN_GENERATOR_CANYON_ROCK_BLOCK);
    if (biome == TERRAIN_BIOME_SNOW)
        return (TERRAIN_GENERATOR_PERMAFROST_BLOCK);
    if (biome == TERRAIN_BIOME_MOUNTAINS)
        return (TERRAIN_GENERATOR_STONE_BLOCK);
    return (TERRAIN_GENERATOR_DIRT_BLOCK);
}

uint32_t terrain_deep_block_for_biome(terrain_biome biome) noexcept
{
    if (biome == TERRAIN_BIOME_MOUNTAINS)
        return (TERRAIN_GENERATOR_CANYON_ROCK_BLOCK);
    return (TERRAIN_GENERATOR_STONE_BLOCK);
}

ft_bool terrain_biome_has_shrubs(terrain_biome biome) noexcept
{
    if (biome == TERRAIN_BIOME_PLAINS)
        return (FT_TRUE);
    if (biome == TERRAIN_BIOME_HILLS)
        return (FT_TRUE);
    if (biome == TERRAIN_BIOME_DESERT)
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool terrain_biome_has_trees(terrain_biome biome) noexcept
{
    if (biome == TERRAIN_BIOME_PLAINS)
        return (FT_TRUE);
    if (biome == TERRAIN_BIOME_HILLS)
        return (FT_TRUE);
    if (biome == TERRAIN_BIOME_DESERT)
        return (FT_TRUE);
    if (biome == TERRAIN_BIOME_SNOW)
        return (FT_TRUE);
    if (biome == TERRAIN_BIOME_MOUNTAINS)
        return (FT_TRUE);
    return (FT_FALSE);
}

static uint32_t terrain_normalise_tree_variant(uint32_t variant_index,
    uint32_t variant_count) noexcept
{
    if (variant_count == 0U)
        return (0U);
    return (variant_index % variant_count);
}

const terrain_block_metadata &terrain_get_block_metadata(uint32_t block_id)
    noexcept
{
    if (block_id >= static_cast<uint32_t>(sizeof(TERRAIN_BLOCK_REGISTRY)
            / sizeof(TERRAIN_BLOCK_REGISTRY[0])))
    {
        terrain_abort_unknown_block_id(block_id,
            "terrain_get_block_metadata");
        return (TERRAIN_BLOCK_REGISTRY[GAME_VOXEL_AIR_BLOCK]);
    }
    return (TERRAIN_BLOCK_REGISTRY[block_id]);
}

ft_bool terrain_block_is_known(uint32_t block_id) noexcept
{
    if (block_id >= static_cast<uint32_t>(sizeof(TERRAIN_BLOCK_REGISTRY)
            / sizeof(TERRAIN_BLOCK_REGISTRY[0])))
        return (FT_FALSE);
    return (FT_TRUE);
}

ft_bool terrain_block_is_solid(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).solid);
}

ft_bool terrain_block_is_transparent(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).transparent);
}

ft_bool terrain_block_is_liquid(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).liquid);
}

ft_bool terrain_block_is_replaceable(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).replaceable);
}

ft_bool terrain_block_emits_light(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).light_emitting);
}

ft_bool terrain_block_occludes_faces(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).occludes_faces);
}

uint32_t terrain_block_hardness(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).hardness);
}

ft_bool terrain_block_is_breakable(uint32_t block_id) noexcept
{
    return (terrain_get_block_metadata(block_id).breakable);
}

const terrain_tree_template &terrain_small_oak_tree_template_variant(
    uint32_t variant_index) noexcept
{
    variant_index = terrain_normalise_tree_variant(variant_index, 3U);
    if (variant_index == 1U)
        return (TERRAIN_SMALL_OAK_TREE_TEMPLATE_VARIANT_1);
    if (variant_index == 2U)
        return (TERRAIN_SMALL_OAK_TREE_TEMPLATE_VARIANT_2);
    return (TERRAIN_SMALL_OAK_TREE_TEMPLATE);
}

const terrain_tree_template &terrain_small_pine_tree_template_variant(
    uint32_t variant_index) noexcept
{
    variant_index = terrain_normalise_tree_variant(variant_index, 3U);
    if (variant_index == 1U)
        return (TERRAIN_SMALL_PINE_TREE_TEMPLATE_VARIANT_1);
    if (variant_index == 2U)
        return (TERRAIN_SMALL_PINE_TREE_TEMPLATE_VARIANT_2);
    return (TERRAIN_SMALL_PINE_TREE_TEMPLATE);
}

const terrain_tree_template &terrain_small_cactus_tree_template_variant(
    uint32_t variant_index) noexcept
{
    variant_index = terrain_normalise_tree_variant(variant_index, 3U);
    if (variant_index == 1U)
        return (TERRAIN_SMALL_CACTUS_TREE_TEMPLATE_VARIANT_1);
    if (variant_index == 2U)
        return (TERRAIN_SMALL_CACTUS_TREE_TEMPLATE_VARIANT_2);
    return (TERRAIN_SMALL_CACTUS_TREE_TEMPLATE);
}

const terrain_tree_template &terrain_small_oak_tree_template(
    uint32_t variant_index) noexcept
{
    return (terrain_small_oak_tree_template_variant(variant_index));
}

const terrain_tree_template &terrain_small_pine_tree_template(
    uint32_t variant_index) noexcept
{
    return (terrain_small_pine_tree_template_variant(variant_index));
}

const terrain_tree_template &terrain_small_cactus_tree_template(
    uint32_t variant_index) noexcept
{
    return (terrain_small_cactus_tree_template_variant(variant_index));
}

const terrain_tree_template &terrain_large_oak_tree_template(
    uint32_t variant_index) noexcept
{
    return (terrain_large_oak_tree_template_variant(variant_index));
}

const terrain_tree_template &terrain_large_pine_tree_template(
    uint32_t variant_index) noexcept
{
    return (terrain_large_pine_tree_template_variant(variant_index));
}

const terrain_tree_template &terrain_large_oak_tree_template_variant(
    uint32_t variant_index) noexcept
{
    variant_index = terrain_normalise_tree_variant(variant_index, 2U);
    if (variant_index == 1U)
        return (TERRAIN_LARGE_OAK_TREE_TEMPLATE_VARIANT_1);
    return (TERRAIN_LARGE_OAK_TREE_TEMPLATE);
}

const terrain_tree_template &terrain_large_pine_tree_template_variant(
    uint32_t variant_index) noexcept
{
    variant_index = terrain_normalise_tree_variant(variant_index, 2U);
    if (variant_index == 1U)
        return (TERRAIN_LARGE_PINE_TREE_TEMPLATE_VARIANT_1);
    return (TERRAIN_LARGE_PINE_TREE_TEMPLATE);
}

const terrain_tree_template &terrain_tree_template_for_biome(
    terrain_biome biome) noexcept
{
    return (terrain_tree_template_for_biome(biome, 0U));
}

const terrain_tree_template &terrain_tree_template_for_biome(
    terrain_biome biome, uint64_t seed_value) noexcept
{
    uint32_t variant_index;

    variant_index = static_cast<uint32_t>(seed_value % 5U);
    if (biome == TERRAIN_BIOME_DESERT)
        return (terrain_small_cactus_tree_template(variant_index));
    if (biome == TERRAIN_BIOME_SNOW)
    {
        if (variant_index < 3U)
            return (terrain_small_pine_tree_template(variant_index));
        return (terrain_large_pine_tree_template(variant_index - 3U));
    }
    if (biome == TERRAIN_BIOME_MOUNTAINS)
    {
        if (variant_index < 3U)
            return (terrain_small_pine_tree_template(variant_index));
        return (terrain_large_pine_tree_template(variant_index - 3U));
    }
    if (biome == TERRAIN_BIOME_HILLS)
    {
        if (variant_index < 3U)
            return (terrain_small_oak_tree_template(variant_index));
        return (terrain_large_oak_tree_template(variant_index - 3U));
    }
    if (variant_index < 3U)
        return (terrain_small_oak_tree_template(variant_index));
    return (terrain_large_oak_tree_template(variant_index - 3U));
}

terrain_biome terrain_get_biome(int32_t world_block_x, int32_t world_block_z,
    const char *seed_string) noexcept
{
    return (terrain_pick_biome(terrain_seed_value(seed_string), world_block_x,
        world_block_z));
}

uint32_t terrain_select_biome(const terrain_generation_config &config,
    uint64_t seed_value, int32_t world_block_x, int32_t world_block_z) noexcept
{
    uint32_t selected;

    if (config.biome_count == 0U)
        return (0U);
    if (config.biome_selector != ft_nullptr)
        selected = config.biome_selector(seed_value, world_block_x,
            world_block_z, config.biome_count, config.biome_selector_user_data);
    else
        selected = static_cast<uint32_t>(terrain_pick_biome(seed_value,
            world_block_x, world_block_z));
    return (selected % config.biome_count);
}

uint32_t terrain_get_biome_index(const terrain_generation_config &config,
    int32_t world_block_x, int32_t world_block_z,
    const char *seed_string) noexcept
{
    return (terrain_select_biome(config, terrain_seed_value(seed_string),
        world_block_x, world_block_z));
}

terrain_generation_config terrain_default_generation_config() noexcept
{
    terrain_generation_config config = {};
    uint32_t index;

    config.sea_level = TERRAIN_GENERATOR_SEA_LEVEL;
    config.large_noise_scale = 32;
    config.detail_noise_scale = 8;
    config.detail_noise_percent = 50;
    config.water_chance_percent = 100U;
    config.biome_count = 5U;
    config.biome_selector = ft_nullptr;
    config.biome_selector_user_data = ft_nullptr;
    index = 0U;
    while (index < config.biome_count)
    {
        terrain_biome biome = static_cast<terrain_biome>(index);
        config.biomes[index].profile = terrain_get_biome_profile(biome);
        config.biomes[index].surface_block_id = terrain_surface_block_for_biome(biome);
        config.biomes[index].subsurface_block_id = terrain_subsurface_block_for_biome(biome);
        config.biomes[index].deep_block_id = terrain_deep_block_for_biome(biome);
        config.biomes[index].allow_shrubs = terrain_biome_has_shrubs(biome);
        config.biomes[index].allow_trees = terrain_biome_has_trees(biome);
        config.biomes[index].shrub_chance_percent = 6U;
        config.biomes[index].tree_chance_percent = 18U;
        /* A null template keeps the built-in seeded variant selection.  A
         * caller can assign a template to override it for that biome. */
        config.biomes[index].tree_template = ft_nullptr;
        index += 1U;
    }
    return (config);
}

static ft_bool terrain_template_is_valid(
    const terrain_tree_template *tree_template) noexcept
{
    uint32_t index;

    if (tree_template == ft_nullptr)
        return (FT_TRUE);
    if (tree_template->blocks == ft_nullptr && tree_template->block_count != 0U)
        return (FT_FALSE);
    index = 0U;
    while (index < tree_template->block_count)
    {
        if (terrain_block_is_known(tree_template->blocks[index].block_id)
            == FT_FALSE)
            return (FT_FALSE);
        index += 1U;
    }
    return (FT_TRUE);
}

ft_bool terrain_generation_config_is_valid(
    const terrain_generation_config &config) noexcept
{
    uint32_t index;

    if (config.biome_count == 0U || config.biome_count > TERRAIN_MAX_CUSTOM_BIOMES
        || config.feature_count > TERRAIN_MAX_FEATURE_RULES
        || config.large_noise_scale <= 0 || config.detail_noise_scale <= 0
        || config.detail_noise_percent < 0 || config.detail_noise_percent > 100
        || config.water_chance_percent > 100)
        return (FT_FALSE);
    index = 0U;
    while (index < config.biome_count)
    {
        if (config.biomes[index].profile.height_variation < 0
            || config.biomes[index].profile.topsoil_depth < 0
            || config.biomes[index].shrub_chance_percent > 100
            || config.biomes[index].tree_chance_percent > 100
            || terrain_block_is_known(config.biomes[index].surface_block_id)
                == FT_FALSE
            || terrain_block_is_known(config.biomes[index].subsurface_block_id)
                == FT_FALSE
            || terrain_block_is_known(config.biomes[index].deep_block_id)
                == FT_FALSE
            || terrain_template_is_valid(config.biomes[index].tree_template)
                == FT_FALSE)
            return (FT_FALSE);
        index += 1U;
    }
    index = 0U;
    while (index < config.feature_count)
    {
        if (config.features[index].chance_percent > 100
            || config.features[index].biome_index < -1
            || config.features[index].biome_index >=
                static_cast<int32_t>(config.biome_count)
            || config.features[index].minimum_height
                > config.features[index].maximum_height
            || terrain_template_is_valid(config.features[index].template_data)
                == FT_FALSE)
            return (FT_FALSE);
        index += 1U;
    }
    return (FT_TRUE);
}

terrain_biome_profile terrain_get_biome_profile(terrain_biome biome) noexcept
{
    terrain_biome_profile biome_profile;

    if (biome == TERRAIN_BIOME_HILLS)
    {
        biome_profile.surface_height = 80;
        biome_profile.height_variation = 8;
        biome_profile.topsoil_depth = 4;
        return (biome_profile);
    }
    if (biome == TERRAIN_BIOME_DESERT)
    {
        biome_profile.surface_height = 70;
        biome_profile.height_variation = 3;
        biome_profile.topsoil_depth = 5;
        return (biome_profile);
    }
    if (biome == TERRAIN_BIOME_SNOW)
    {
        biome_profile.surface_height = 84;
        biome_profile.height_variation = 6;
        biome_profile.topsoil_depth = 4;
        return (biome_profile);
    }
    if (biome == TERRAIN_BIOME_MOUNTAINS)
    {
        biome_profile.surface_height = 100;
        biome_profile.height_variation = 14;
        biome_profile.topsoil_depth = 2;
        return (biome_profile);
    }
    biome_profile.surface_height = TERRAIN_GENERATOR_SURFACE_HEIGHT;
    biome_profile.height_variation = 2;
    biome_profile.topsoil_depth = 3;
    return (biome_profile);
}

ft_bool terrain_can_place_tree_template(game_voxel_chunk &chunk,
    int32_t local_origin_x, int32_t local_origin_y, int32_t local_origin_z,
    const terrain_tree_template &tree_template) noexcept
{
    uint32_t block_index;
    int32_t target_x;
    int32_t target_y;
    int32_t target_z;
    uint32_t block_id;

    if (tree_template.blocks == ft_nullptr)
        return (FT_FALSE);
    block_index = 0;
    while (block_index < tree_template.block_count)
    {
        target_x = local_origin_x + tree_template.blocks[block_index].offset_x;
        target_y = local_origin_y + tree_template.blocks[block_index].offset_y;
        target_z = local_origin_z + tree_template.blocks[block_index].offset_z;
        if (target_x < 0 || target_x >= GAME_VOXEL_CHUNK_WIDTH
            || target_y < 0 || target_y >= GAME_VOXEL_CHUNK_HEIGHT
            || target_z < 0 || target_z >= GAME_VOXEL_CHUNK_DEPTH)
            return (FT_FALSE);
        if (chunk.read_block(target_x, target_y, target_z, &block_id)
            != FT_ERR_SUCCESS)
            return (FT_FALSE);
        if (terrain_block_is_replaceable(block_id) == FT_FALSE)
            return (FT_FALSE);
        block_index += 1;
    }
    return (FT_TRUE);
}

int32_t terrain_place_tree_template(game_voxel_chunk &chunk,
    int32_t local_origin_x, int32_t local_origin_y, int32_t local_origin_z,
    const terrain_tree_template &tree_template) noexcept
{
    if (terrain_can_place_tree_template(chunk, local_origin_x, local_origin_y,
            local_origin_z, tree_template) == FT_FALSE)
        return (FT_ERR_INVALID_OPERATION);
    uint32_t block_index;
    int32_t target_x;
    int32_t target_y;
    int32_t target_z;
    int32_t error_code;

    if (tree_template.blocks == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    block_index = 0;
    while (block_index < tree_template.block_count)
    {
        target_x = local_origin_x + tree_template.blocks[block_index].offset_x;
        target_y = local_origin_y + tree_template.blocks[block_index].offset_y;
        target_z = local_origin_z + tree_template.blocks[block_index].offset_z;
        if (target_x >= 0 && target_x < GAME_VOXEL_CHUNK_WIDTH
            && target_y >= 0 && target_y < GAME_VOXEL_CHUNK_HEIGHT
            && target_z >= 0 && target_z < GAME_VOXEL_CHUNK_DEPTH)
        {
            error_code = chunk.write_block(target_x, target_y, target_z,
                tree_template.blocks[block_index].block_id);
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        block_index += 1;
    }
    return (FT_ERR_SUCCESS);
}

#endif
