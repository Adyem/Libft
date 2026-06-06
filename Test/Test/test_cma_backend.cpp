#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstdlib>

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

typedef struct s_backend_allocation_entry
{
    void *memory_pointer;
    ft_size_t size;
}   t_backend_allocation_entry;

struct test_backend_state
{
    t_backend_allocation_entry allocations[256];
    ft_size_t allocation_slots;
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_bool verify_zero_before_free;
    ft_bool observed_zero_before_free;
};

static int32_t backend_find_allocation_slot(const test_backend_state *state,
        const void *memory_pointer)
{
    ft_size_t index;

    if (memory_pointer == ft_nullptr)
        return (-1);
    index = 0;
    while (index < state->allocation_slots)
    {
        if (state->allocations[index].memory_pointer == memory_pointer)
            return (static_cast<int32_t>(index));
        index++;
    }
    return (-1);
}

static void initialize_test_backend_state(test_backend_state *state)
{
    ft_size_t index;

    index = 0;
    while (index < 256)
    {
        state->allocations[index].memory_pointer = ft_nullptr;
        state->allocations[index].size = 0;
        index++;
    }
    state->allocation_slots = 0;
    state->allocation_count = 0;
    state->free_count = 0;
    state->verify_zero_before_free = FT_FALSE;
    state->observed_zero_before_free = FT_FALSE;
    return ;
}

static void *test_backend_allocate(ft_size_t size, void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    void *memory_pointer = std::malloc(size);
    if (!memory_pointer)
    {
        return (ft_nullptr);
    }
    if (state->allocation_slots >= 256)
    {
        std::free(memory_pointer);
        return (ft_nullptr);
    }
    state->allocations[state->allocation_slots].memory_pointer = memory_pointer;
    state->allocations[state->allocation_slots].size = size;
    state->allocation_slots++;
    state->allocation_count++;
    return (memory_pointer);
}

static void *test_backend_reallocate(void *memory_pointer, ft_size_t size,
        void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    if (!memory_pointer)
        return (test_backend_allocate(size, user_data));
    if (size == 0)
    {
        int32_t slot_index = backend_find_allocation_slot(state, memory_pointer);

        if (slot_index >= 0)
        {
            state->allocations[slot_index]
                = state->allocations[state->allocation_slots - 1];
            state->allocations[state->allocation_slots - 1].memory_pointer = ft_nullptr;
            state->allocations[state->allocation_slots - 1].size = 0;
            state->allocation_slots--;
        }
        std::free(memory_pointer);
        state->free_count++;
        return (ft_nullptr);
    }
    int32_t slot_index = backend_find_allocation_slot(state, memory_pointer);
    void *new_pointer = std::realloc(memory_pointer, size);
    if (!new_pointer)
    {
        return (ft_nullptr);
    }
    if (slot_index >= 0)
    {
        state->allocations[slot_index].memory_pointer = new_pointer;
        state->allocations[slot_index].size = size;
    }
    return (new_pointer);
}

static void test_backend_deallocate(void *memory_pointer, void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    if (!memory_pointer)
        return ;
    int32_t slot_index = backend_find_allocation_slot(state, memory_pointer);
    ft_size_t allocation_size = 0;
    if (slot_index >= 0)
    {
        allocation_size = state->allocations[slot_index].size;
        if (state->verify_zero_before_free == FT_TRUE)
        {
            ft_size_t byte_index = 0;

            state->observed_zero_before_free = FT_TRUE;
            while (byte_index < allocation_size)
            {
                if (static_cast<unsigned char *>(memory_pointer)[byte_index] != 0)
                {
                    state->observed_zero_before_free = FT_FALSE;
                    break ;
                }
                byte_index++;
            }
        }
        state->allocations[slot_index]
            = state->allocations[state->allocation_slots - 1];
        state->allocations[state->allocation_slots - 1].memory_pointer = ft_nullptr;
        state->allocations[state->allocation_slots - 1].size = 0;
        state->allocation_slots--;
    }
    state->free_count++;
    std::free(memory_pointer);
    return ;
}

static ft_size_t test_backend_get_allocation_size(const void *memory_pointer,
        void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    if (!memory_pointer)
        return (0);
    int32_t slot_index = backend_find_allocation_slot(state, memory_pointer);

    if (slot_index < 0)
        return (0);
    return (state->allocations[slot_index].size);
}

static ft_bool test_backend_owns_allocation(const void *memory_pointer,
        void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    if (!memory_pointer)
        return (FT_FALSE);
    if (backend_find_allocation_slot(state, memory_pointer) < 0)
        return (FT_FALSE);
    return (FT_TRUE);
}

static int32_t test_cma_backend_hooks_impl(void)
{
    test_backend_state backend_state;
    initialize_test_backend_state(&backend_state);
    cma_backend_hooks hooks;
    hooks.allocate = &test_backend_allocate;
    hooks.reallocate = &test_backend_reallocate;
    hooks.deallocate = &test_backend_deallocate;
    hooks.aligned_allocate = ft_nullptr;
    hooks.get_allocation_size = &test_backend_get_allocation_size;
    hooks.owns_allocation = &test_backend_owns_allocation;
    hooks.user_data = &backend_state;

    if (cma_set_backend(&hooks) != 0)
        return (0);
    if (cma_backend_is_enabled() != 1)
        return (0);

    void *memory_pointer = cma_malloc(64);
    if (!memory_pointer)
        return (0);
    if (backend_find_allocation_slot(&backend_state, memory_pointer) < 0)
        return (0);
    if (cma_alloc_size(memory_pointer) != 64)
        return (0);

    ft_memset(memory_pointer, 0x5A, 32);
    void *reallocated_pointer = cma_realloc(memory_pointer, 128);
    if (!reallocated_pointer)
        return (0);
    if (cma_alloc_size(reallocated_pointer) != 128)
        return (0);
    if (backend_find_allocation_slot(&backend_state, reallocated_pointer) < 0)
        return (0);
    cma_free(reallocated_pointer);
    if (backend_state.allocation_slots != 0)
        return (0);
    cma_clear_backend();
    if (cma_backend_is_enabled() != 0)
        return (0);

    void *default_memory_pointer = cma_malloc(16);
    if (!default_memory_pointer)
        return (0);
    if (cma_backend_is_enabled() != 0)
        return (0);
    cma_free(default_memory_pointer);
    return (1);
}

FT_TEST(test_cma_backend_hooks)
{
    FT_ASSERT_EQ(1, test_cma_backend_hooks_impl());
    return (1);
}

FT_TEST(test_cma_bzero_and_free_zeroes_backend_allocation_before_release)
{
    test_backend_state backend_state;
    cma_backend_hooks hooks;
    void *memory_pointer;

    initialize_test_backend_state(&backend_state);
    backend_state.verify_zero_before_free = FT_TRUE;
    hooks.allocate = &test_backend_allocate;
    hooks.reallocate = &test_backend_reallocate;
    hooks.deallocate = &test_backend_deallocate;
    hooks.aligned_allocate = ft_nullptr;
    hooks.get_allocation_size = &test_backend_get_allocation_size;
    hooks.owns_allocation = &test_backend_owns_allocation;
    hooks.user_data = &backend_state;
    if (cma_set_backend(&hooks) != FT_ERR_SUCCESS)
        return (0);
    memory_pointer = cma_malloc(64);
    if (!memory_pointer)
    {
        cma_clear_backend();
        return (0);
    }
    ft_memset(memory_pointer, 0x5A, 64);
    cma_bzero_and_free(memory_pointer);
    if (backend_state.observed_zero_before_free != FT_TRUE)
    {
        cma_clear_backend();
        return (0);
    }
    cma_clear_backend();
    return (1);
}
