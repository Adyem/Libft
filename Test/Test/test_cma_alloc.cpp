#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include <climits>
#include <thread>

FT_TEST(test_cma_calloc_overflow_guard, "cma_calloc rejects overflowed sizes")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocated_pointer;

    cma_get_stats(&allocation_count_before, ft_nullptr);
    ft_errno = ER_SUCCESS;
    allocated_pointer = cma_calloc(SIZE_MAX, 2);
    int allocation_errno = ft_errno;
    cma_get_stats(&allocation_count_after, ft_nullptr);
    ft_errno = allocation_errno;
    FT_ASSERT(allocated_pointer == ft_nullptr);
    FT_ASSERT_EQ(allocation_count_before, allocation_count_after);
    FT_ASSERT_EQ(allocation_errno, FT_ERR_INVALID_ARGUMENT);
    return (1);
}

FT_TEST(test_cma_malloc_zero_size_allocates, "cma_malloc returns a block for zero-size requests")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocation_pointer;
    ft_size_t expected_allocation_count;

    cma_set_alloc_limit(0);
    cma_get_stats(&allocation_count_before, ft_nullptr);
    ft_errno = ER_SUCCESS;
    allocation_pointer = cma_malloc(0);
    if (!allocation_pointer)
        return (0);
    cma_get_stats(&allocation_count_after, ft_nullptr);
    cma_free(allocation_pointer);
    expected_allocation_count = allocation_count_before + 1;
    FT_ASSERT_EQ(expected_allocation_count, allocation_count_after);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    return (1);
}

FT_TEST(test_cma_realloc_failure_preserves_original_buffer, "cma_realloc keeps the original buffer when growth fails")
{
    char *original_buffer;
    void *realloc_result;
    int byte_index;

    cma_set_alloc_limit(0);
    original_buffer = static_cast<char *>(cma_malloc(16));
    if (!original_buffer)
        return (0);
    ft_memset(original_buffer, 'X', 16);
    cma_set_alloc_limit(8);
    ft_errno = ER_SUCCESS;
    realloc_result = cma_realloc(original_buffer, 32);
    int realloc_errno = ft_errno;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, realloc_result);
    FT_ASSERT_EQ(realloc_errno, FT_ERR_NO_MEMORY);
    byte_index = 0;
    while (byte_index < 16)
    {
        if (original_buffer[byte_index] != 'X')
        {
            cma_free(original_buffer);
            return (0);
        }
        byte_index++;
    }
    cma_free(original_buffer);
    return (1);
}

FT_TEST(test_cma_malloc_limit_sets_errno, "cma_malloc reports allocation failures")
{
    void *allocation_pointer;
    int allocation_errno;

    cma_set_alloc_limit(8);
    ft_errno = ER_SUCCESS;
    allocation_pointer = cma_malloc(16);
    allocation_errno = ft_errno;
    cma_set_alloc_limit(0);
    FT_ASSERT(allocation_pointer == ft_nullptr);
    FT_ASSERT_EQ(allocation_errno, FT_ERR_NO_MEMORY);
    FT_ASSERT_EQ(ft_errno, FT_ERR_NO_MEMORY);
    return (1);
}

FT_TEST(test_cma_malloc_success_sets_errno, "cma_malloc reports success on allocation")
{
    void *allocation_pointer;

    cma_set_alloc_limit(0);
    ft_errno = FT_ERR_NO_MEMORY;
    allocation_pointer = cma_malloc(32);
    if (!allocation_pointer)
        return (0);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_realloc_success_sets_errno, "cma_realloc reports success on growth")
{
    char *original_buffer;
    void *reallocation_pointer;
    int byte_index;

    cma_set_alloc_limit(0);
    original_buffer = static_cast<char *>(cma_malloc(16));
    if (!original_buffer)
        return (0);
    ft_memset(original_buffer, 'Z', 16);
    ft_errno = FT_ERR_NO_MEMORY;
    reallocation_pointer = cma_realloc(original_buffer, 64);
    if (!reallocation_pointer)
    {
        cma_free(original_buffer);
        return (0);
    }
    byte_index = 0;
    while (byte_index < 16)
    {
        if (static_cast<char *>(reallocation_pointer)[byte_index] != 'Z')
        {
            cma_free(reallocation_pointer);
            return (0);
        }
        byte_index++;
    }
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(reallocation_pointer);
    return (1);
}

FT_TEST(test_cma_memdup_copies_buffer, "cma_memdup duplicates raw bytes")
{
    unsigned char source[5];
    unsigned char *duplicate;

    source[0] = 0x10;
    source[1] = 0x20;
    source[2] = 0x30;
    source[3] = 0x40;
    source[4] = 0x50;
    cma_set_alloc_limit(0);
    ft_errno = FT_ERR_NO_MEMORY;
    duplicate = static_cast<unsigned char *>(cma_memdup(source, sizeof(source)));
    if (!duplicate)
        return (0);
    FT_ASSERT_EQ(0, ft_memcmp(source, duplicate, sizeof(source)));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_memdup_zero_size_returns_valid_block, "cma_memdup returns a block when size is zero")
{
    unsigned char source;
    void *duplicate;

    source = 0xAB;
    cma_set_alloc_limit(0);
    ft_errno = FT_ERR_NO_MEMORY;
    duplicate = cma_memdup(&source, 0);
    if (!duplicate)
        return (0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_memdup_null_source_sets_errno, "cma_memdup rejects null source pointers when size is non-zero")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, cma_memdup(ft_nullptr, 4));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cma_memdup_allocation_failure_sets_errno, "cma_memdup propagates allocation failures")
{
    void *duplicate;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    duplicate = cma_memdup("buffer", 16);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

static void release_allocation_range(void **allocation_pointers, ft_size_t start_index, ft_size_t end_index)
{
    ft_size_t current_index;

    if (start_index > end_index)
        return ;
    current_index = start_index;
    while (current_index <= end_index)
    {
        if (allocation_pointers[current_index] != ft_nullptr)
        {
            cma_free(allocation_pointers[current_index]);
            allocation_pointers[current_index] = ft_nullptr;
        }
        if (current_index == end_index)
            break ;
        current_index = current_index + 1;
    }
    return ;
}

FT_TEST(test_cma_randomized_stress_allocations, "cma handles randomized allocation stress")
{
    static const ft_size_t allocation_total_count = 1024u;
    void *allocation_pointers[allocation_total_count];
    ft_size_t allocation_sizes[allocation_total_count];
    unsigned char allocation_patterns[allocation_total_count];
    ft_size_t allocation_count_before;
    ft_size_t free_count_before;
    ft_size_t allocation_count_after;
    ft_size_t free_count_after;
    ft_size_t allocation_index;

    ft_seed_random_engine(8472u);
    allocation_index = 0;
    while (allocation_index < allocation_total_count)
    {
        allocation_pointers[allocation_index] = ft_nullptr;
        allocation_sizes[allocation_index] = 0;
        allocation_patterns[allocation_index] = 0;
        allocation_index = allocation_index + 1;
    }
    cma_set_alloc_limit(0);
    cma_get_stats(&allocation_count_before, &free_count_before);
    allocation_index = 0;
    while (allocation_index < allocation_total_count)
    {
        unsigned int random_value;
        ft_size_t requested_size;
        unsigned char pattern_value;

        random_value = static_cast<unsigned int>(ft_random_int());
        requested_size = static_cast<ft_size_t>((random_value % 4096u) + 1u);
        pattern_value = static_cast<unsigned char>((random_value % 251u) + 1u);
        allocation_sizes[allocation_index] = requested_size;
        allocation_patterns[allocation_index] = pattern_value;
        allocation_pointers[allocation_index] = cma_malloc(requested_size);
        if (!allocation_pointers[allocation_index])
        {
            if (allocation_index > 0)
                release_allocation_range(allocation_pointers, 0, allocation_index - 1);
            return (0);
        }
        ft_memset(allocation_pointers[allocation_index], pattern_value, requested_size);
        allocation_index = allocation_index + 1;
    }
    allocation_index = 0;
    while (allocation_index < allocation_total_count)
    {
        unsigned char *current_block;
        ft_size_t block_size;
        unsigned char pattern_value;
        ft_size_t offset;

        current_block = static_cast<unsigned char *>(allocation_pointers[allocation_index]);
        block_size = allocation_sizes[allocation_index];
        pattern_value = allocation_patterns[allocation_index];
        offset = 0;
        while (offset < block_size)
        {
            if (current_block[offset] != pattern_value)
            {
                release_allocation_range(allocation_pointers, allocation_index, allocation_total_count - 1);
                return (0);
            }
            offset = offset + 1;
        }
        cma_free(current_block);
        allocation_pointers[allocation_index] = ft_nullptr;
        allocation_index = allocation_index + 1;
    }
    cma_get_stats(&allocation_count_after, &free_count_after);
    FT_ASSERT_EQ(allocation_count_before + allocation_total_count, allocation_count_after);
    FT_ASSERT_EQ(free_count_before + allocation_total_count, free_count_after);
    return (1);
}

static void release_thread_slots(void **allocation_slots, int slot_count)
{
    int slot_index;

    slot_index = 0;
    while (slot_index < slot_count)
    {
        if (allocation_slots[slot_index] != ft_nullptr)
        {
            cma_free(allocation_slots[slot_index]);
            allocation_slots[slot_index] = ft_nullptr;
        }
        slot_index = slot_index + 1;
    }
    return ;
}

static void cma_thread_mix_sequence(bool *thread_result)
{
    static const int allocation_slot_count = 32;
    static const int iteration_limit = 384;
    void *allocation_slots[allocation_slot_count];
    ft_size_t allocation_sizes[allocation_slot_count];
    unsigned char allocation_patterns[allocation_slot_count];
    int slot_index;
    int iteration_index;

    slot_index = 0;
    while (slot_index < allocation_slot_count)
    {
        allocation_slots[slot_index] = ft_nullptr;
        allocation_sizes[slot_index] = 0;
        allocation_patterns[slot_index] = 0;
        slot_index = slot_index + 1;
    }
    iteration_index = 0;
    while (iteration_index < iteration_limit)
    {
        int current_slot;
        ft_size_t requested_size;
        unsigned char pattern_value;

        current_slot = iteration_index % allocation_slot_count;
        requested_size = static_cast<ft_size_t>(((iteration_index % 17) + 1) * 24);
        pattern_value = static_cast<unsigned char>((current_slot % 251) + 1);
        if (allocation_slots[current_slot] == ft_nullptr)
        {
            ft_errno = ER_SUCCESS;
            allocation_slots[current_slot] = cma_malloc(requested_size);
            if (!allocation_slots[current_slot] || ft_errno != ER_SUCCESS)
            {
                release_thread_slots(allocation_slots, allocation_slot_count);
                *thread_result = false;
                return ;
            }
            allocation_sizes[current_slot] = requested_size;
            allocation_patterns[current_slot] = pattern_value;
            ft_memset(allocation_slots[current_slot], pattern_value, requested_size);
        }
        else if ((iteration_index % 3) == 0)
        {
            unsigned char *previous_block;
            void *reallocated_block;
            ft_size_t previous_size;

            previous_block = static_cast<unsigned char *>(allocation_slots[current_slot]);
            previous_size = allocation_sizes[current_slot];
            if (previous_block[0] != allocation_patterns[current_slot])
            {
                release_thread_slots(allocation_slots, allocation_slot_count);
                *thread_result = false;
                return ;
            }
            ft_errno = ER_SUCCESS;
            reallocated_block = cma_realloc(allocation_slots[current_slot], requested_size + 48);
            if (!reallocated_block || ft_errno != ER_SUCCESS)
            {
                release_thread_slots(allocation_slots, allocation_slot_count);
                *thread_result = false;
                return ;
            }
            if (static_cast<unsigned char *>(reallocated_block)[0] != allocation_patterns[current_slot])
            {
                cma_free(reallocated_block);
                release_thread_slots(allocation_slots, allocation_slot_count);
                *thread_result = false;
                return ;
            }
            if (requested_size + 48 > previous_size)
            {
                ft_size_t offset;

                offset = previous_size;
                while (offset < requested_size + 48)
                {
                    static_cast<unsigned char *>(reallocated_block)[offset] = allocation_patterns[current_slot];
                    offset = offset + 1;
                }
            }
            allocation_slots[current_slot] = reallocated_block;
            allocation_sizes[current_slot] = requested_size + 48;
        }
        else
        {
            ft_errno = ER_SUCCESS;
            cma_free(allocation_slots[current_slot]);
            if (ft_errno != ER_SUCCESS)
            {
                release_thread_slots(allocation_slots, allocation_slot_count);
                *thread_result = false;
                return ;
            }
            allocation_slots[current_slot] = ft_nullptr;
            allocation_sizes[current_slot] = 0;
            allocation_patterns[current_slot] = 0;
        }
        iteration_index = iteration_index + 1;
    }
    slot_index = 0;
    while (slot_index < allocation_slot_count)
    {
        if (allocation_slots[slot_index] != ft_nullptr)
        {
            unsigned char *block_bytes;
            ft_size_t offset;

            block_bytes = static_cast<unsigned char *>(allocation_slots[slot_index]);
            offset = 0;
            while (offset < allocation_sizes[slot_index])
            {
                if (block_bytes[offset] != allocation_patterns[slot_index])
                {
                    release_thread_slots(allocation_slots, allocation_slot_count);
                    *thread_result = false;
                    return ;
                }
                offset = offset + 1;
            }
            cma_free(block_bytes);
            allocation_slots[slot_index] = ft_nullptr;
        }
        slot_index = slot_index + 1;
    }
    *thread_result = true;
    return ;
}

FT_TEST(test_cma_threaded_reallocation_stress, "cma handles concurrent malloc, realloc, and free operations")
{
    static const int thread_count = 6;
    bool thread_results[thread_count];
    std::thread worker_threads[thread_count];
    ft_size_t allocation_count_before;
    ft_size_t free_count_before;
    ft_size_t allocation_count_after;
    ft_size_t free_count_after;
    int thread_index;
    ft_size_t outstanding_before;
    ft_size_t outstanding_after;

    cma_set_thread_safety(true);
    cma_set_alloc_limit(0);
    cma_get_stats(&allocation_count_before, &free_count_before);
    thread_index = 0;
    while (thread_index < thread_count)
    {
        thread_results[thread_index] = false;
        worker_threads[thread_index] = std::thread(cma_thread_mix_sequence, &thread_results[thread_index]);
        thread_index = thread_index + 1;
    }
    thread_index = 0;
    while (thread_index < thread_count)
    {
        worker_threads[thread_index].join();
        thread_index = thread_index + 1;
    }
    thread_index = 0;
    while (thread_index < thread_count)
    {
        FT_ASSERT(thread_results[thread_index] == true);
        thread_index = thread_index + 1;
    }
    cma_get_stats(&allocation_count_after, &free_count_after);
    outstanding_before = allocation_count_before - free_count_before;
    outstanding_after = allocation_count_after - free_count_after;
    FT_ASSERT_EQ(outstanding_before, outstanding_after);
    FT_ASSERT(allocation_count_after >= allocation_count_before);
    FT_ASSERT(free_count_after >= free_count_before);
    return (1);
}
