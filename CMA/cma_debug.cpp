#include <cstring>
#include <limits>
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"

#if DEBUG

static const unsigned char g_cma_guard_pattern = 0xA5;
static const ft_size_t g_cma_guard_size = 32;

ft_size_t cma_debug_guard_size(void)
{
    return (g_cma_guard_size);
}

ft_size_t cma_debug_allocation_size(ft_size_t requested_size)
{
    ft_size_t guard_bytes;
    ft_size_t maximum_value;

    guard_bytes = g_cma_guard_size * 2;
    maximum_value = std::numeric_limits<ft_size_t>::max();
    if (requested_size > maximum_value - guard_bytes)
    {
        return (maximum_value);
    }
    return (requested_size + guard_bytes);
}

void cma_debug_initialize_block(Block *block)
{
    if (!block)
        return ;
    block->debug_base_pointer = block->payload;
    block->debug_user_size = 0;
    return ;
}

static void cma_debug_abort(void)
{
    su_sigabrt();
    return ;
}

static unsigned char *cma_debug_base_pointer(Block *block)
{
    unsigned char   *base_pointer;

    if (!block)
        return (ft_nullptr);
    base_pointer = block->debug_base_pointer;
    if (!base_pointer)
        base_pointer = block->payload;
    return (base_pointer);
}

static unsigned char *cma_debug_const_base_pointer(const Block *block)
{
    unsigned char   *base_pointer;

    if (!block)
        return (ft_nullptr);
    base_pointer = block->debug_base_pointer;
    if (!base_pointer)
        base_pointer = block->payload;
    return (base_pointer);
}

void cma_debug_prepare_allocation(Block *block, ft_size_t user_size)
{
    unsigned char   *base_pointer;
    unsigned char   *user_pointer;
    unsigned char   *rear_guard_pointer;
    ft_size_t        guard_size;
    ft_size_t        required_size;
    ft_size_t        index;

    if (!block)
        return ;
    base_pointer = cma_debug_base_pointer(block);
    if (!base_pointer)
        cma_debug_abort();
    guard_size = cma_debug_guard_size();
    required_size = user_size;
    required_size += guard_size * 2;
    if (block->size < required_size)
        cma_debug_abort();
    user_pointer = base_pointer + guard_size;
    rear_guard_pointer = user_pointer + user_size;
    block->debug_base_pointer = base_pointer;
    block->debug_user_size = user_size;
    index = 0;
    while (index < guard_size)
    {
        base_pointer[index] = g_cma_guard_pattern;
        index++;
    }
    index = 0;
    while (index < guard_size)
    {
        rear_guard_pointer[index] = g_cma_guard_pattern;
        index++;
    }
    return ;
}

void cma_debug_release_allocation(Block *block, const char *context,
        const void *user_pointer)
{
    unsigned char   *base_pointer;
    unsigned char   *user_area_pointer;
    unsigned char   *rear_guard_pointer;
    ft_size_t        guard_size;
    ft_size_t        index;
    ft_size_t        wipe_index;

    (void)context;
    (void)user_pointer;
    if (!block)
        return ;
    guard_size = cma_debug_guard_size();
    base_pointer = cma_debug_base_pointer(block);
    if (!base_pointer)
        return ;
    user_area_pointer = base_pointer + guard_size;
    rear_guard_pointer = user_area_pointer + block->debug_user_size;
    index = 0;
    while (index < guard_size)
    {
        if (base_pointer[index] != g_cma_guard_pattern)
            cma_debug_abort();
        if (rear_guard_pointer[index] != g_cma_guard_pattern)
            cma_debug_abort();
        index++;
    }
    wipe_index = 0;
    while (wipe_index < block->debug_user_size)
    {
        user_area_pointer[wipe_index] = 0xDD;
        wipe_index++;
    }
    block->debug_user_size = 0;
    index = 0;
    while (index < guard_size)
    {
        base_pointer[index] = 0xDE;
        rear_guard_pointer[index] = 0xDE;
        index++;
    }
    return ;
}

unsigned char *cma_debug_user_pointer(const Block *block)
{
    unsigned char   *base_pointer;

    base_pointer = cma_debug_const_base_pointer(block);
    if (!base_pointer)
        return (ft_nullptr);
    return (base_pointer + cma_debug_guard_size());
}

ft_size_t cma_debug_user_size(const Block *block)
{
    if (!block)
        return (0);
    return (block->debug_user_size);
}

#endif
