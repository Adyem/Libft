#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

struct cma_metadata_chunk
{
    unsigned char    *memory;
    ft_size_t    size;
    ft_size_t    used;
    bool        protected_state;
    cma_metadata_chunk    *next;
};

static cma_metadata_chunk    *g_cma_metadata_chunks = ft_nullptr;
static Block    *g_cma_metadata_free_list = ft_nullptr;
static ft_size_t    g_cma_metadata_stride = 0;
static ft_size_t    g_cma_metadata_page_size = 0;

static bool cma_metadata_add_chunk(void);
#if CMA_ENABLE_METADATA_PROTECTION
static int cma_metadata_apply_protection(int protection);
#endif

static ft_size_t cma_metadata_compute_stride(void)
{
    ft_size_t    stride;

    if (g_cma_metadata_stride != 0)
        return (g_cma_metadata_stride);
    stride = align16(static_cast<ft_size_t>(sizeof(Block)));
    g_cma_metadata_stride = stride;
    return (stride);
}

static ft_size_t cma_metadata_compute_page_size(void)
{
    long    system_page_size;

    if (g_cma_metadata_page_size != 0)
        return (g_cma_metadata_page_size);
    system_page_size = sysconf(_SC_PAGESIZE);
    if (system_page_size <= 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (0);
    }
    g_cma_metadata_page_size = static_cast<ft_size_t>(system_page_size);
    ft_errno = ER_SUCCESS;
    return (g_cma_metadata_page_size);
}

static bool cma_metadata_add_chunk(void)
{
    ft_size_t    stride;
    ft_size_t    page_size;
    ft_size_t    chunk_stride_count;
    ft_size_t    chunk_size;
    cma_metadata_chunk    *chunk;

    stride = cma_metadata_compute_stride();
    if (stride == 0)
        return (false);
    page_size = cma_metadata_compute_page_size();
    if (page_size == 0)
        return (false);
    chunk_stride_count = page_size / stride;
    if (chunk_stride_count == 0)
        chunk_stride_count = 1;
    chunk_size = chunk_stride_count * stride;
    chunk = static_cast<cma_metadata_chunk *>(std::malloc(sizeof(cma_metadata_chunk)));
    if (chunk == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (false);
    }
    chunk->memory = static_cast<unsigned char *>(mmap(ft_nullptr, chunk_size,
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (chunk->memory == reinterpret_cast<unsigned char *>(MAP_FAILED))
    {
        std::free(chunk);
        ft_errno = FT_ERR_NO_MEMORY;
        return (false);
    }
    chunk->size = chunk_size;
    chunk->used = 0;
    chunk->protected_state = false;
    chunk->next = g_cma_metadata_chunks;
    g_cma_metadata_chunks = chunk;
    PROTECT_METADATA(chunk->memory, chunk->size);
    ft_errno = ER_SUCCESS;
    return (true);
}

#if CMA_ENABLE_METADATA_PROTECTION

static int cma_metadata_apply_protection(int protection)
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
            if (protection == PROT_NONE)
            {
                PROTECT_METADATA(chunk->memory, chunk->size);
                chunk->protected_state = false;
            }
            chunk = chunk->next;
            continue ;
        }
        if (mprotect(chunk->memory, chunk->size, protection) != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (-1);
        }
        if (protection == PROT_NONE)
        {
            PROTECT_METADATA(chunk->memory, chunk->size);
            chunk->protected_state = false;
        }
        else
        {
            UNPROTECT_METADATA(chunk->memory, chunk->size);
            chunk->protected_state = true;
        }
        chunk = chunk->next;
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int cma_metadata_make_writable(void)
{
    if (g_cma_metadata_chunks == ft_nullptr)
    {
        if (!cma_metadata_add_chunk())
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (-1);
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
                if (mprotect(chunk->memory, chunk->size,
                        PROT_READ | PROT_WRITE) != 0)
                {
                    ft_errno = FT_ERR_INVALID_STATE;
                    return (-1);
                }
                UNPROTECT_METADATA(chunk->memory, chunk->size);
                chunk->protected_state = true;
            }
            chunk = chunk->next;
        }
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (cma_metadata_apply_protection(PROT_READ | PROT_WRITE) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_metadata_make_inaccessible(void)
{
    if (g_cma_metadata_chunks == ft_nullptr)
        return ;
    cma_metadata_apply_protection(PROT_NONE);
    return ;
}

bool cma_metadata_guard_increment(void)
{
    g_cma_metadata_access_depth++;
    return (true);
}

bool cma_metadata_guard_decrement(void)
{
    if (g_cma_metadata_access_depth == 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    g_cma_metadata_access_depth--;
    if (g_cma_metadata_access_depth == 0)
        cma_metadata_make_inaccessible();
    return (true);
}

#else

int cma_metadata_make_writable(void)
{
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_metadata_make_inaccessible(void)
{
    return ;
}

bool cma_metadata_guard_increment(void)
{
    g_cma_metadata_access_depth++;
    return (true);
}

bool cma_metadata_guard_decrement(void)
{
    if (g_cma_metadata_access_depth == 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    g_cma_metadata_access_depth--;
    return (true);
}

#endif

Block    *cma_metadata_allocate_block(void)
{
    cma_metadata_chunk    *chunk;
    Block                *block;
    ft_size_t            stride;

    if (cma_metadata_make_writable() != 0)
        return (ft_nullptr);
    if (g_cma_metadata_free_list != ft_nullptr)
    {
        block = g_cma_metadata_free_list;
        g_cma_metadata_free_list = block->next;
        std::memset(block, 0, sizeof(Block));
        ft_errno = ER_SUCCESS;
        return (block);
    }
    if (g_cma_metadata_chunks == ft_nullptr)
    {
        if (!cma_metadata_add_chunk())
        {
            ft_errno = FT_ERR_NO_MEMORY;
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
            ft_errno = ER_SUCCESS;
            return (block);
        }
        if (chunk->next == ft_nullptr)
        {
            if (!cma_metadata_add_chunk())
            {
                ft_errno = FT_ERR_NO_MEMORY;
                return (ft_nullptr);
            }
            chunk = g_cma_metadata_chunks;
            continue ;
        }
        chunk = chunk->next;
    }
    ft_errno = FT_ERR_NO_MEMORY;
    return (ft_nullptr);
}

void    cma_metadata_release_block(Block *block)
{
    if (block == ft_nullptr)
        return ;
    if (cma_metadata_make_writable() != 0)
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
            munmap(chunk->memory, chunk->size);
        std::free(chunk);
        chunk = next_chunk;
    }
    g_cma_metadata_chunks = ft_nullptr;
    g_cma_metadata_free_list = ft_nullptr;
    g_cma_metadata_stride = 0;
    g_cma_metadata_page_size = 0;
    return ;
}

