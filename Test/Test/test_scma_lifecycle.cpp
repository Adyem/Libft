#include "test_scma_shared.hpp"

FT_TEST(test_scma_initialize_cycle,
        "scma initialization cycle handles double init and shutdown")
{
    scma_test_reset();
    FT_ASSERT_EQ(1, scma_initialize(64));
    FT_ASSERT_EQ(0, scma_initialize(64));
    scma_shutdown();
    FT_ASSERT_EQ(0, scma_is_initialized());
    return (1);
}

FT_TEST(test_scma_allocate_zero_size_rejected, "scma rejects zero-sized allocations")
{
    scma_handle handle;

    FT_ASSERT_EQ(1, scma_test_initialize(32));
    handle = scma_allocate(0);
    FT_ASSERT_EQ(0, scma_handle_is_valid(handle));
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_double_free_and_bounds_errors,
        "scma reports bounds violations and double free")
{
    scma_handle handle;
    int value;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int) * 2)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int) * 2));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    value = 42;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(0, scma_write(handle, static_cast<ft_size_t>(sizeof(int) * 2),
                &value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(1, scma_free(handle));
    FT_ASSERT_EQ(0, scma_free(handle));
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_handle_invalid_after_shutdown,
        "scma invalidates handles once shutdown")
{
    scma_handle handle;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    scma_shutdown();
    FT_ASSERT_EQ(0, scma_handle_is_valid(handle));
    return (1);
}

FT_TEST(test_scma_free_invalid_handle_reports_error,
        "scma free reports invalid handle when runtime offline")
{
    scma_handle handle;

    handle.index = 0;
    handle.generation = 0;
    scma_test_reset();
    FT_ASSERT_EQ(0, scma_free(handle));
    return (1);
}

FT_TEST(test_scma_allocate_requires_runtime_online,
        "scma allocation rejects offline runtime")
{
    scma_handle handle;

    scma_test_reset();
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(0, scma_handle_is_valid(handle));
    return (1);
}
