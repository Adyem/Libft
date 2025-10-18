#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"

FT_TEST(test_cma_leak_detection_tracks_allocations,
        "cma leak detection counts outstanding allocations")
{
    void *first_pointer;
    ft_size_t tracked_block_size;

    cma_set_alloc_limit(0);
    cma_leak_detection_disable();
    cma_leak_detection_clear();
    cma_leak_detection_enable();
    FT_ASSERT_EQ(0ull, cma_leak_detection_outstanding_allocations());
    FT_ASSERT_EQ(0ull, cma_leak_detection_outstanding_bytes());
    first_pointer = cma_malloc(64);
    if (first_pointer == ft_nullptr)
        return (0);
    tracked_block_size = cma_alloc_size(first_pointer);
    FT_ASSERT_EQ(1ull, cma_leak_detection_outstanding_allocations());
    FT_ASSERT_EQ(tracked_block_size, cma_leak_detection_outstanding_bytes());
    cma_free(first_pointer);
    FT_ASSERT_EQ(0ull, cma_leak_detection_outstanding_allocations());
    FT_ASSERT_EQ(0ull, cma_leak_detection_outstanding_bytes());
    cma_leak_detection_disable();
    return (1);
}

FT_TEST(test_cma_leak_detection_realloc_updates_records,
        "cma leak detection reflects realloc resizing")
{
    void *allocation_pointer;
    void *resized_pointer;
    ft_size_t resized_block_size;

    cma_set_alloc_limit(0);
    cma_leak_detection_disable();
    cma_leak_detection_clear();
    cma_leak_detection_enable();
    allocation_pointer = cma_malloc(32);
    if (allocation_pointer == ft_nullptr)
        return (0);
    resized_pointer = cma_realloc(allocation_pointer, 96);
    if (resized_pointer == ft_nullptr)
        return (0);
    resized_block_size = cma_alloc_size(resized_pointer);
    FT_ASSERT_EQ(1ull, cma_leak_detection_outstanding_allocations());
    FT_ASSERT_EQ(resized_block_size, cma_leak_detection_outstanding_bytes());
    cma_free(resized_pointer);
    FT_ASSERT_EQ(0ull, cma_leak_detection_outstanding_allocations());
    FT_ASSERT_EQ(0ull, cma_leak_detection_outstanding_bytes());
    cma_leak_detection_disable();
    return (1);
}

FT_TEST(test_cma_leak_detection_report_optionally_clears,
        "cma leak detection report can clear outstanding state")
{
    void *allocation_pointer;
    ft_string leak_report;

    cma_set_alloc_limit(0);
    cma_leak_detection_disable();
    cma_leak_detection_clear();
    cma_leak_detection_enable();
    allocation_pointer = cma_malloc(48);
    if (allocation_pointer == ft_nullptr)
        return (0);
    leak_report = cma_leak_detection_report(false);
    FT_ASSERT(leak_report.find("outstanding allocations: 1") != ft_string::npos);
    FT_ASSERT_EQ(1ull, cma_leak_detection_outstanding_allocations());
    leak_report = cma_leak_detection_report(true);
    FT_ASSERT(leak_report.find("outstanding allocations: 1") != ft_string::npos);
    FT_ASSERT_EQ(0ull, cma_leak_detection_outstanding_allocations());
    cma_free(allocation_pointer);
    cma_leak_detection_disable();
    return (1);
}
