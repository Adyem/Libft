#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"

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
    return (free_result == -1 && ft_errno == FT_ERR_INVALID_POINTER);
}

