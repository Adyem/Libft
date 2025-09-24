#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdint>
#include <thread>

int test_cma_checked_free_basic(void)
{
    ft_errno = 0;
    void *p = cma_malloc(32);
    if (!p)
        return (0);
    int r = cma_checked_free(p);
    return (r == 0 && ft_errno == ER_SUCCESS);
}

int test_cma_checked_free_offset(void)
{
    ft_errno = 0;
    char *p = static_cast<char*>(cma_malloc(32));
    if (!p)
        return (0);
    int r = cma_checked_free(p + 10);
    return (r == 0 && ft_errno == ER_SUCCESS);
}

int test_cma_checked_free_invalid(void)
{
    int local;
    ft_errno = 0;
    int r = cma_checked_free(&local);
    return (r == -1 && ft_errno == CMA_INVALID_PTR);
}

FT_TEST(test_cma_calloc_overflow_guard, "cma_calloc rejects overflowed sizes")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocated_pointer;

    cma_get_stats(&allocation_count_before, ft_nullptr);
    allocated_pointer = cma_calloc(SIZE_MAX, 2);
    cma_get_stats(&allocation_count_after, ft_nullptr);
    FT_ASSERT(allocated_pointer == ft_nullptr);
    FT_ASSERT_EQ(allocation_count_before, allocation_count_after);
    return (1);
}

FT_TEST(test_cma_malloc_zero_size_allocates, "cma_malloc returns a block for zero-size requests")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocation_pointer;

    cma_set_alloc_limit(0);
    cma_get_stats(&allocation_count_before, ft_nullptr);
    allocation_pointer = cma_malloc(0);
    if (!allocation_pointer)
        return (0);
    cma_get_stats(&allocation_count_after, ft_nullptr);
    cma_free(allocation_pointer);
    ft_size_t expected_allocation_count = allocation_count_before + 1;
    FT_ASSERT_EQ(expected_allocation_count, allocation_count_after);
    return (1);
}

FT_TEST(test_cma_realloc_failure_preserves_original_buffer, "cma_realloc keeps the original buffer when growth fails")
{
    char *original_buffer;
    void *realloc_result;
    int byte_index;

    cma_set_alloc_limit(0);
    original_buffer = static_cast<char*>(cma_malloc(16));
    if (!original_buffer)
        return (0);
    ft_memset(original_buffer, 'X', 16);
    cma_set_alloc_limit(8);
    realloc_result = cma_realloc(original_buffer, 32);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, realloc_result);
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

static void cma_thread_success(bool *thread_result)
{
    ft_errno = ER_SUCCESS;
    void *memory_pointer = cma_malloc(32);
    if (!memory_pointer)
    {
        *thread_result = false;
        return ;
    }
    int free_result = cma_checked_free(memory_pointer);
    if (free_result != 0)
    {
        *thread_result = false;
        return ;
    }
    *thread_result = (ft_errno == ER_SUCCESS);
    return ;
}

static void cma_thread_failure(bool *thread_result)
{
    int local_variable;
    ft_errno = ER_SUCCESS;
    int free_result = cma_checked_free(&local_variable);
    *thread_result = (free_result == -1 && ft_errno == CMA_INVALID_PTR);
    return ;
}

int test_cma_thread_local(void)
{
    bool success_result = false;
    bool failure_result = false;
    std::thread success_thread(cma_thread_success, &success_result);
    std::thread failure_thread(cma_thread_failure, &failure_result);
    success_thread.join();
    failure_thread.join();
    return (success_result && failure_result);
}

static void allocation_thread(bool *thread_result)
{
    int allocation_index;

    allocation_index = 0;
    while (allocation_index < 100)
    {
        void *memory_pointer = cma_malloc(64);
        if (!memory_pointer)
        {
            *thread_result = false;
            return ;
        }
        cma_free(memory_pointer);
        allocation_index++;
    }
    *thread_result = true;
    return ;
}

int test_cma_thread_alloc(void)
{
    static const int thread_count = 4;
    bool thread_results[thread_count];
    std::thread threads[thread_count];
    int thread_index;

    cma_set_thread_safety(true);
    thread_index = 0;
    while (thread_index < thread_count)
    {
        thread_results[thread_index] = false;
        threads[thread_index] = std::thread(allocation_thread, &thread_results[thread_index]);
        thread_index++;
    }
    thread_index = 0;
    while (thread_index < thread_count)
    {
        threads[thread_index].join();
        thread_index++;
    }
    thread_index = 0;
    while (thread_index < thread_count)
    {
        if (thread_results[thread_index] == false)
            return (0);
        thread_index++;
    }
    return (1);
}
