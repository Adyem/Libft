#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "cma_internal.hpp"
#include "../Compatebility/compatebility_cma_platform.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"

struct cma_metadata_chunk
{
    unsigned char    *memory;
    ft_size_t    size;
    ft_size_t    used;
    ft_bool        protected_state;
    cma_metadata_chunk    *next;
};

static cma_metadata_chunk    *g_cma_metadata_chunks = ft_nullptr;
static Block    *g_cma_metadata_free_list = ft_nullptr;
static ft_size_t    g_cma_metadata_stride = 0;
static ft_size_t    g_cma_metadata_page_size = 0;

static ft_bool cma_metadata_add_chunk(void);
#if CMA_ENABLE_METADATA_PROTECTION
static int32_t cma_metadata_apply_protection(ft_bool make_inaccessible);
#endif

static ft_size_t cma_metadata_compute_stride(void)
{
    ft_size_t    stride;

    if (g_cma_metadata_stride != 0)
        return (g_cma_metadata_stride);
    stride = align16(sizeof(Block));
    g_cma_metadata_stride = stride;
    return (stride);
}

static ft_size_t cma_metadata_compute_page_size(void)
{
    ft_size_t page_size_value;

    if (g_cma_metadata_page_size != 0)
        return (g_cma_metadata_page_size);
    if (cmp_cma_get_page_size(&page_size_value) != FT_ERR_SUCCESS)
        return (0);
    g_cma_metadata_page_size = page_size_value;
    return (g_cma_metadata_page_size);
}

static ft_bool cma_metadata_add_chunk(void)
{
    ft_size_t    stride;
    ft_size_t    page_size;
    ft_size_t    chunk_stride_count;
    ft_size_t    chunk_size;
    cma_metadata_chunk    *chunk;

    stride = cma_metadata_compute_stride();
    if (stride == 0)
        return (FT_FALSE);
    page_size = cma_metadata_compute_page_size();
    if (page_size == 0)
        return (FT_FALSE);
    chunk_stride_count = page_size / stride;
    if (chunk_stride_count == 0)
        chunk_stride_count = 1;
    chunk_size = chunk_stride_count * stride;
    chunk = static_cast<cma_metadata_chunk *>(std::malloc(sizeof(cma_metadata_chunk)));
    if (chunk == ft_nullptr)
        return (FT_FALSE);
    chunk->memory = static_cast<unsigned char *>(
            cmp_cma_memory_map_read_write(chunk_size));
    if (chunk->memory == ft_nullptr)
    {
        std::free(chunk);
        return (FT_FALSE);
    }
    chunk->size = chunk_size;
    chunk->used = 0;
    chunk->next = g_cma_metadata_chunks;
    g_cma_metadata_chunks = chunk;
    if (g_cma_metadata_access_depth == 0)
    {
        chunk->protected_state = FT_FALSE;
        PROTECT_METADATA(chunk->memory, chunk->size);
    }
    else
    {
        chunk->protected_state = FT_TRUE;
        UNPROTECT_METADATA(chunk->memory, chunk->size);
    }
    return (FT_TRUE);
}

#if CMA_ENABLE_METADATA_PROTECTION

static int32_t cma_metadata_apply_protection(ft_bool make_inaccessible)
{
    cma_metadata_chunk    *chunk;

    chunk = g_cma_metadata_chunks;
    while (chunk)
    {
        if (chunk->memory == ft_nullptr)
        {
            chunk = chunk->next;
            continue ;
        }
        if (chunk->protected_state)
        {
            if (make_inaccessible == FT_TRUE)
            {
                PROTECT_METADATA(chunk->memory, chunk->size);
                chunk->protected_state = FT_FALSE;
            }
            chunk = chunk->next;
            continue ;
        }
        if (make_inaccessible == FT_TRUE)
        {
            if (cmp_cma_memory_protect_none(chunk->memory,
                    chunk->size) != FT_ERR_SUCCESS)
                return (FT_ERR_INVALID_STATE);
            PROTECT_METADATA(chunk->memory, chunk->size);
            chunk->protected_state = FT_FALSE;
        }
        else
        {
            if (cmp_cma_memory_protect_read_write(chunk->memory,
                    chunk->size) != FT_ERR_SUCCESS)
                return (FT_ERR_INVALID_STATE);
            UNPROTECT_METADATA(chunk->memory, chunk->size);
            chunk->protected_state = FT_TRUE;
        }
        chunk = chunk->next;
    }
    return (FT_ERR_SUCCESS);
}

int32_t cma_metadata_make_writable(void)
{
    if (g_cma_metadata_chunks == ft_nullptr)
    {
        if (!cma_metadata_add_chunk())
        {
            return (FT_ERR_NO_MEMORY);
        }
    }
    if (g_cma_metadata_access_depth != 0)
    {
        cma_metadata_chunk    *chunk;

        chunk = g_cma_metadata_chunks;
        while (chunk)
        {
            if (!chunk->protected_state)
            {
                if (cmp_cma_memory_protect_read_write(chunk->memory,
                        chunk->size) != FT_ERR_SUCCESS)
                    return (FT_ERR_INVALID_STATE);
                UNPROTECT_METADATA(chunk->memory, chunk->size);
                chunk->protected_state = FT_TRUE;
            }
            chunk = chunk->next;
        }
        return (FT_ERR_SUCCESS);
    }
    if (cma_metadata_apply_protection(FT_FALSE) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESS);
}

void cma_metadata_make_inaccessible(void)
{
    if (g_cma_metadata_chunks == ft_nullptr)
        return ;
    cma_metadata_apply_protection(FT_TRUE);
    return ;
}

ft_bool cma_metadata_guard_increment(void)
{
    g_cma_metadata_access_depth++;
    return (FT_TRUE);
}

ft_bool cma_metadata_guard_decrement(void)
{
    if (g_cma_metadata_access_depth == 0)
        return (FT_FALSE);
    g_cma_metadata_access_depth--;
    if (g_cma_metadata_access_depth == 0)
        cma_metadata_make_inaccessible();
    return (FT_TRUE);
}

#else

int32_t cma_metadata_make_writable(void)
{
    return (FT_ERR_SUCCESS);
}

void cma_metadata_make_inaccessible(void)
{
    return ;
}

ft_bool cma_metadata_guard_increment(void)
{
    g_cma_metadata_access_depth++;
    return (FT_TRUE);
}

ft_bool cma_metadata_guard_decrement(void)
{
    if (g_cma_metadata_access_depth == 0)
        return (FT_FALSE);
    g_cma_metadata_access_depth--;
    return (FT_TRUE);
}

#endif

Block    *cma_metadata_allocate_block(void)
{
    cma_metadata_chunk    *chunk;
    Block                *block;
    ft_size_t            stride;

    if (cma_metadata_make_writable() != FT_ERR_SUCCESS)
        return (ft_nullptr);
    if (g_cma_metadata_free_list != ft_nullptr)
    {
        block = g_cma_metadata_free_list;
        g_cma_metadata_free_list = block->next;
        std::memset(block, 0, sizeof(Block));
        return (block);
    }
    if (g_cma_metadata_chunks == ft_nullptr)
    {
        if (!cma_metadata_add_chunk())
        {
            return (ft_nullptr);
        }
    }
    stride = cma_metadata_compute_stride();
    chunk = g_cma_metadata_chunks;
    while (chunk)
    {
        if (chunk->used + stride <= chunk->size)
        {
            block = reinterpret_cast<Block *>(chunk->memory + chunk->used);
            chunk->used += stride;
            std::memset(block, 0, sizeof(Block));
            return (block);
        }
        if (chunk->next == ft_nullptr)
        {
            if (!cma_metadata_add_chunk())
            {
                return (ft_nullptr);
            }
            chunk = g_cma_metadata_chunks;
            continue ;
        }
        chunk = chunk->next;
    }
    return (ft_nullptr);
}

void    cma_metadata_release_block(Block *block)
{
    if (block == ft_nullptr)
        return ;
    if (cma_metadata_make_writable() != FT_ERR_SUCCESS)
        return ;
    block->next = g_cma_metadata_free_list;
    block->prev = ft_nullptr;
    block->size = 0;
    block->payload = ft_nullptr;
#if DEBUG
    block->debug_base_pointer = ft_nullptr;
    block->debug_user_size = 0;
#endif
    g_cma_metadata_free_list = block;
    return ;
}

void    cma_metadata_reset(void)
{
    cma_metadata_chunk    *chunk;

    chunk = g_cma_metadata_chunks;
    while (chunk)
    {
        cma_metadata_chunk    *next_chunk;

        next_chunk = chunk->next;
        if (chunk->memory != ft_nullptr && chunk->size != 0)
            (void)cmp_cma_memory_unmap(chunk->memory, chunk->size);
        std::free(chunk);
        chunk = next_chunk;
    }
    g_cma_metadata_chunks = ft_nullptr;
    g_cma_metadata_free_list = ft_nullptr;
    g_cma_metadata_stride = 0;
    g_cma_metadata_page_size = 0;
    return ;
}
