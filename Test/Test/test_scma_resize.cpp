#include "../test_internal.hpp"
#include "test_scma_shared.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_scma_resize_grows_preserves_data, "scma resize enlarges block while keeping existing contents")
{
    scma_handle handle;
    int write_values[4];
    int read_value;
    ft_size_t index;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int) * 4)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int) * 2));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    index = 0;
    while (index < 2)
    {
        write_values[static_cast<size_t>(index)] = static_cast<int>(index * 10 + 1);
        FT_ASSERT_EQ(1, scma_write(handle,
                static_cast<ft_size_t>(sizeof(int)) * index,
                &write_values[static_cast<size_t>(index)],
                static_cast<ft_size_t>(sizeof(int))));
        index = index + 1;
    }
    FT_ASSERT_EQ(1, scma_resize(handle, static_cast<ft_size_t>(sizeof(int) * 4)));
    FT_ASSERT_EQ(static_cast<ft_size_t>(sizeof(int) * 4), scma_get_size(handle));
    index = 0;
    while (index < 2)
    {
        read_value = 0;
        FT_ASSERT_EQ(1, scma_read(handle,
                static_cast<ft_size_t>(sizeof(int)) * index,
                &read_value,
                static_cast<ft_size_t>(sizeof(int))));
        FT_ASSERT_EQ(write_values[static_cast<size_t>(index)], read_value);
        index = index + 1;
    }
    write_values[2] = 33;
    write_values[3] = 44;
    FT_ASSERT_EQ(1, scma_write(handle,
            static_cast<ft_size_t>(sizeof(int) * 2),
            &write_values[2],
            static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(1, scma_write(handle,
            static_cast<ft_size_t>(sizeof(int) * 3),
            &write_values[3],
            static_cast<ft_size_t>(sizeof(int))));
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(handle,
            static_cast<ft_size_t>(sizeof(int) * 3),
            &read_value,
            static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(write_values[3], read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_resize_shrinks_truncates_tail, "scma resize shrinking truncates trailing data and enforces bounds")
{
    scma_handle handle;
    int write_values[3];
    int read_value;
    ft_size_t index;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int) * 3)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int) * 3));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    index = 0;
    while (index < 3)
    {
        write_values[static_cast<size_t>(index)] = static_cast<int>(index * 5 + 2);
        FT_ASSERT_EQ(1, scma_write(handle,
                static_cast<ft_size_t>(sizeof(int)) * index,
                &write_values[static_cast<size_t>(index)],
                static_cast<ft_size_t>(sizeof(int))));
        index = index + 1;
    }
    FT_ASSERT_EQ(1, scma_resize(handle, static_cast<ft_size_t>(sizeof(int) * 2)));
    FT_ASSERT_EQ(static_cast<ft_size_t>(sizeof(int) * 2), scma_get_size(handle));
    index = 0;
    while (index < 2)
    {
        read_value = 0;
        FT_ASSERT_EQ(1, scma_read(handle,
                static_cast<ft_size_t>(sizeof(int)) * index,
                &read_value,
                static_cast<ft_size_t>(sizeof(int))));
        FT_ASSERT_EQ(write_values[static_cast<size_t>(index)], read_value);
        index = index + 1;
    }
    read_value = 0;
    FT_ASSERT_EQ(0, scma_read(handle,
            static_cast<ft_size_t>(sizeof(int) * 2),
            &read_value,
            static_cast<ft_size_t>(sizeof(int))));
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_resize_rejects_zero_size, "scma resize rejects zero-sized targets")
{
    scma_handle handle;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(0, scma_resize(handle, 0));
    scma_shutdown();
    return (1);
}
