#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
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
