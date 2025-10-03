#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include <thread>

int test_cma_checked_free_basic(void)
{
    void *memory_pointer;
    int free_result;

    ft_errno = 0;
    memory_pointer = cma_malloc(32);
    if (!memory_pointer)
        return (0);
    free_result = cma_checked_free(memory_pointer);
    return (free_result == 0 && ft_errno == ER_SUCCESS);
}

int test_cma_checked_free_offset(void)
{
    char *memory_pointer;
    int free_result;

    ft_errno = 0;
    memory_pointer = static_cast<char *>(cma_malloc(32));
    if (!memory_pointer)
        return (0);
    free_result = cma_checked_free(memory_pointer + 10);
    return (free_result == 0 && ft_errno == ER_SUCCESS);
}

int test_cma_checked_free_invalid(void)
{
    int local_variable;
    int free_result;

    ft_errno = 0;
    free_result = cma_checked_free(&local_variable);
    return (free_result == -1 && ft_errno == CMA_INVALID_PTR);
}

static void cma_thread_success(bool *thread_result)
{
    void *memory_pointer;
    int free_result;

    ft_errno = ER_SUCCESS;
    memory_pointer = cma_malloc(32);
    if (!memory_pointer)
    {
        *thread_result = false;
        return ;
    }
    free_result = cma_checked_free(memory_pointer);
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
    int free_result;

    ft_errno = ER_SUCCESS;
    free_result = cma_checked_free(&local_variable);
    *thread_result = (free_result == -1 && ft_errno == CMA_INVALID_PTR);
    return ;
}

int test_cma_thread_local(void)
{
    bool success_result;
    bool failure_result;

    success_result = false;
    failure_result = false;
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
        void *memory_pointer;

        memory_pointer = cma_malloc(64);
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
