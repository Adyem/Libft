#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include <cstdlib>
#include <unordered_map>

struct test_backend_state
{
    std::unordered_map<void*, ft_size_t> allocation_sizes;
    ft_size_t allocation_count;
    ft_size_t free_count;
};

static void initialize_test_backend_state(test_backend_state *state)
{
    state->allocation_sizes.clear();
    state->allocation_count = 0;
    state->free_count = 0;
    return ;
}

static void *test_backend_allocate(ft_size_t size, void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    void *memory_pointer = std::malloc(static_cast<size_t>(size));
    if (!memory_pointer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    state->allocation_sizes[memory_pointer] = size;
    state->allocation_count++;
    ft_errno = ER_SUCCESS;
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
        if (memory_pointer)
            state->allocation_sizes.erase(memory_pointer);
        std::free(memory_pointer);
        state->free_count++;
        ft_errno = ER_SUCCESS;
        return (ft_nullptr);
    }
    void *new_pointer = std::realloc(memory_pointer, static_cast<size_t>(size));
    if (!new_pointer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    state->allocation_sizes.erase(memory_pointer);
    state->allocation_sizes[new_pointer] = size;
    ft_errno = ER_SUCCESS;
    return (new_pointer);
}

static void test_backend_deallocate(void *memory_pointer, void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    if (!memory_pointer)
        return ;
    state->allocation_sizes.erase(memory_pointer);
    state->free_count++;
    std::free(memory_pointer);
    ft_errno = ER_SUCCESS;
    return ;
}

static ft_size_t test_backend_get_allocation_size(const void *memory_pointer,
        void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    if (!memory_pointer)
        return (0);
    std::unordered_map<void*, ft_size_t>::iterator iterator =
        state->allocation_sizes.find(const_cast<void*>(memory_pointer));
    if (iterator == state->allocation_sizes.end())
        return (0);
    return (iterator->second);
}

static int test_backend_owns_allocation(const void *memory_pointer,
        void *user_data)
{
    test_backend_state *state = static_cast<test_backend_state*>(user_data);
    if (!memory_pointer)
        return (0);
    std::unordered_map<void*, ft_size_t>::iterator iterator =
        state->allocation_sizes.find(const_cast<void*>(memory_pointer));
    if (iterator == state->allocation_sizes.end())
        return (0);
    return (1);
}

int test_cma_backend_hooks(void)
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

    ft_errno = ER_SUCCESS;
    if (cma_set_backend(&hooks) != 0)
        return (0);
    if (cma_backend_is_enabled() != 1)
        return (0);

    void *memory_pointer = cma_malloc(64);
    if (!memory_pointer)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    if (backend_state.allocation_sizes.find(memory_pointer)
        == backend_state.allocation_sizes.end())
        return (0);
    if (cma_alloc_size(memory_pointer) != 64)
        return (0);

    ft_memset(memory_pointer, 0x5A, 32);
    void *reallocated_pointer = cma_realloc(memory_pointer, 128);
    if (!reallocated_pointer)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    if (cma_alloc_size(reallocated_pointer) != 128)
        return (0);
    if (backend_state.allocation_sizes.find(reallocated_pointer)
        == backend_state.allocation_sizes.end())
        return (0);

    cma_free(reallocated_pointer);
    if (!backend_state.allocation_sizes.empty())
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
