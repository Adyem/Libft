#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

int test_cma_checked_free_basic(void)
{
    ft_errno = 0;
    void *p = cma_malloc(32);
    if (!p)
        return 0;
    int r = cma_checked_free(p);
    return (r == 0 && ft_errno == ER_SUCCESS);
}

int test_cma_checked_free_offset(void)
{
    ft_errno = 0;
    char *p = static_cast<char*>(cma_malloc(32));
    if (!p)
        return 0;
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
