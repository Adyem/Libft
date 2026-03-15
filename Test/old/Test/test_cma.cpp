#include "../test_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

int test_cma_checked_free_basic(void)
{
    void *memory_pointer;

    memory_pointer = cma_malloc(32);
    if (!memory_pointer)
        return (0);
    if (cma_checked_free(memory_pointer) != 0)
        return (0);
    return (1);
}

int test_cma_checked_free_offset(void)
{
    char *memory_pointer;

    memory_pointer = static_cast<char *>(cma_malloc(32));
    if (!memory_pointer)
        return (0);
    if (cma_checked_free(memory_pointer + 10) == 0)
        return (0);
    cma_free(memory_pointer);
    return (1);
}

int test_cma_checked_free_invalid(void)
{
    int local_variable;

    if (cma_checked_free(&local_variable) == 0)
        return (0);
    return (1);
}
