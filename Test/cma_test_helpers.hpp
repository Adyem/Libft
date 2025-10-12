#ifndef TEST_CMA_TEST_HELPERS_HPP
#define TEST_CMA_TEST_HELPERS_HPP

#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

static inline bool ensure_cma_cleanup_success(void)
{
    int attempt;

    attempt = 0;
    while (attempt < 8)
    {
        ft_errno = ER_SUCCESS;
        cma_cleanup();
        if (ft_errno != FT_ERR_INVALID_STATE)
            return (true);
        attempt++;
    }
    return (false);
}

#endif
