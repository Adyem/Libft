#include "../test_internal.hpp"
#include "../../Modules/CMA/cma_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_metadata_allocate_block_zeroes_state)
{
    Block *block;

    block = cma_metadata_allocate_block();
    if (block == ft_nullptr)
        return (0);
    FT_ASSERT(block->next == ft_nullptr);
    FT_ASSERT(block->prev == ft_nullptr);
    FT_ASSERT(block->payload == ft_nullptr);
    FT_ASSERT(block->size == 0);
    FT_ASSERT(block->magic == 0);
    cma_metadata_release_block(block);
    return (1);
}

FT_TEST(test_cma_metadata_release_block_reuses_blocks)
{
    Block *first_block;
    Block *second_block;

    first_block = cma_metadata_allocate_block();
    if (first_block == ft_nullptr)
        return (0);
    cma_metadata_release_block(first_block);
    second_block = cma_metadata_allocate_block();
    if (second_block == ft_nullptr)
        return (0);
    FT_ASSERT(second_block == first_block);
    cma_metadata_release_block(second_block);
    return (1);
}
