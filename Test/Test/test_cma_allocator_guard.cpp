#include "../../CMA/cma_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_cma_allocator_guard_unlock_resets_errno,
        "cma_allocator_guard unlock sets ft_errno to success")
{
    cma_allocator_guard allocator_guard;
    int sentinel_errno;

    if (!allocator_guard.is_active())
        return (0);
    sentinel_errno = FT_ERR_INVALID_ARGUMENT;
    ft_errno = sentinel_errno;
    allocator_guard.unlock();
    FT_ASSERT_EQ(false, allocator_guard.is_active());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, allocator_guard.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_cma_allocator_guard_destructor_resets_errno,
        "cma_allocator_guard destructor sets ft_errno to success")
{
    int sentinel_errno;

    sentinel_errno = FT_ERR_INVALID_ARGUMENT;
    ft_errno = sentinel_errno;
    {
        cma_allocator_guard allocator_guard;

        if (!allocator_guard.is_active())
            return (0);
        ft_errno = sentinel_errno;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
