#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <new>
#include <cstdint>

namespace
{
    struct alignas(64) aligned_trivial_type
    {
        int value;
    };

    struct alignas(128) aligned_large_type
    {
        int value;
    };
}

FT_TEST(test_cma_global_new_preserves_alignment,
    "global operator new respects over-aligned requirements")
{
    aligned_trivial_type *instance;
    std::uintptr_t instance_address;
    std::uintptr_t alignment_mask;

    cma_set_alloc_limit(0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    instance = new (std::nothrow) aligned_trivial_type;
    if (instance == ft_nullptr)
        return (0);
    instance_address = reinterpret_cast<std::uintptr_t>(instance);
    alignment_mask = alignof(aligned_trivial_type) - 1;
    FT_ASSERT_EQ(instance_address & alignment_mask, 0);
    FT_ASSERT_EQ(ft_errno, FT_ERR_INVALID_ARGUMENT);
    delete instance;
    return (1);
}

FT_TEST(test_cma_global_new_alignment_failure_sets_errno,
    "global operator new propagates allocator failures for over-aligned types")
{
    aligned_large_type *instance;

    cma_set_alloc_limit(16);
    ft_errno = ER_SUCCESS;
    instance = new (std::nothrow) aligned_large_type;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(instance, ft_nullptr);
    FT_ASSERT_EQ(ft_errno, FT_ERR_NO_MEMORY);
    return (1);
}
