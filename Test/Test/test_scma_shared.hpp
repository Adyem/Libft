#ifndef TEST_SCMA_SHARED_HPP
# define TEST_SCMA_SHARED_HPP

#include "../../SCMA/SCMA.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

struct scma_test_pair
{
    int first;
    int second;
};

static inline void    scma_test_reset(void)
{
    if (scma_is_initialized())
        scma_shutdown();
    return ;
}

static inline int    scma_test_initialize(ft_size_t capacity)
{
    scma_test_reset();
    return (scma_initialize(capacity));
}

#endif
