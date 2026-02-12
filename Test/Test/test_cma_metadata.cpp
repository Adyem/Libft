#include "../test_internal.hpp"
#include "../../CMA/cma_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_metadata_allocate_block_zeroes_state,
        "cma_metadata_allocate_block returns cleared block")
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

FT_TEST(test_cma_metadata_release_block_reuses_blocks,
        "cma_metadata_release_block recycles allocations")
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

