#include <cstring>
#include <cstdlib>
#include "test_scma_shared.hpp"

FT_TEST(test_scma_allocation_roundtrip, "scma allocation read/write roundtrip")
{
    scma_handle handle;
    int write_value;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(128));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    write_value = 123456789;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &write_value, static_cast<ft_size_t>(sizeof(int))));
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(handle, 0, &read_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(write_value, read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_allocation_unsigned_char_payload, "scma allocation stores arbitrary byte buffers")
{
    scma_handle handle;
    unsigned char payload[6];
    unsigned char readback[6];
    ft_size_t index;

    FT_ASSERT_EQ(1, scma_test_initialize(128));
    handle = scma_allocate(6);
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    index = 0;
    while (index < 6)
    {
        payload[static_cast<size_t>(index)] = static_cast<unsigned char>(index * 3 + 1);
        index = index + 1;
    }
    FT_ASSERT_EQ(1, scma_write(handle, 0, payload, 6));
    std::memset(readback, 0, sizeof(readback));
    FT_ASSERT_EQ(1, scma_read(handle, 0, readback, 6));
    index = 0;
    while (index < 6)
    {
        FT_ASSERT_EQ(payload[static_cast<size_t>(index)], readback[static_cast<size_t>(index)]);
        index = index + 1;
    }
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_compaction_preserves_live_blocks, "scma compaction keeps live data valid")
{
    scma_handle first_handle;
    scma_handle second_handle;
    scma_handle third_handle;
    int first_value;
    int second_value;
    int third_value;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(256));
    first_handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    second_handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(first_handle));
    FT_ASSERT_EQ(1, scma_handle_is_valid(second_handle));
    first_value = 111;
    second_value = 222;
    FT_ASSERT_EQ(1, scma_write(first_handle, 0, &first_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(1, scma_write(second_handle, 0, &second_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(1, scma_free(first_handle));
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(second_handle, 0, &read_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(second_value, read_value);
    third_handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(third_handle));
    third_value = 333;
    FT_ASSERT_EQ(1, scma_write(third_handle, 0, &third_value, static_cast<ft_size_t>(sizeof(int))));
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(second_handle, 0, &read_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(second_value, read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_allocation_double_precision_roundtrip, "scma allocation keeps double precision values exact")
{
    scma_handle handle;
    double write_values[3];
    double read_values[3];
    ft_size_t index;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(double) * 3)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(double) * 3));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    index = 0;
    while (index < 3)
    {
        write_values[static_cast<size_t>(index)] = static_cast<double>(index) + 0.125;
        index = index + 1;
    }
    FT_ASSERT_EQ(1, scma_write(handle, 0, write_values, static_cast<ft_size_t>(sizeof(double) * 3)));
    std::memset(read_values, 0, sizeof(read_values));
    FT_ASSERT_EQ(1, scma_read(handle, 0, read_values, static_cast<ft_size_t>(sizeof(double) * 3)));
    index = 0;
    while (index < 3)
    {
        FT_ASSERT_EQ(write_values[static_cast<size_t>(index)], read_values[static_cast<size_t>(index)]);
        index = index + 1;
    }
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_compaction_reuses_space_for_new_blocks, "scma compaction reclaims free space for future allocations")
{
    scma_handle handles[3];
    int stored_values[3];
    ft_size_t index;
    int read_value;
    scma_handle expanded_handle;
    int expanded_values[4];
    ft_size_t expanded_index;

    FT_ASSERT_EQ(1, scma_test_initialize(256));
    index = 0;
    while (index < 3)
    {
        handles[static_cast<size_t>(index)] = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
        FT_ASSERT_EQ(1, scma_handle_is_valid(handles[static_cast<size_t>(index)]));
        stored_values[static_cast<size_t>(index)] = static_cast<int>(index * 100 + 1);
        FT_ASSERT_EQ(1, scma_write(handles[static_cast<size_t>(index)], 0, &stored_values[static_cast<size_t>(index)], static_cast<ft_size_t>(sizeof(int))));
        index = index + 1;
    }
    FT_ASSERT_EQ(1, scma_free(handles[1]));
    expanded_handle = scma_allocate(static_cast<ft_size_t>(sizeof(int) * 4));
    FT_ASSERT_EQ(1, scma_handle_is_valid(expanded_handle));
    expanded_index = 0;
    while (expanded_index < 4)
    {
        expanded_values[static_cast<size_t>(expanded_index)] = static_cast<int>(expanded_index + 10);
        FT_ASSERT_EQ(1, scma_write(expanded_handle,
                static_cast<ft_size_t>(sizeof(int)) * expanded_index,
                &expanded_values[static_cast<size_t>(expanded_index)],
                static_cast<ft_size_t>(sizeof(int))));
        expanded_index = expanded_index + 1;
    }
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(handles[0], 0, &read_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(stored_values[0], read_value);
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(handles[2], 0, &read_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(stored_values[2], read_value);
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(expanded_handle, static_cast<ft_size_t>(sizeof(int)) * 3, &read_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(expanded_values[3], read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_resize_grow_and_shrink_preserves_data, "scma resize keeps live bytes on growth and shrink")
{
    scma_handle handle;
    unsigned char initial_payload[4];
    unsigned char buffer[8];
    ft_size_t index;

    FT_ASSERT_EQ(1, scma_test_initialize(64));
    handle = scma_allocate(4);
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    index = 0;
    while (index < 4)
    {
        initial_payload[static_cast<size_t>(index)] = static_cast<unsigned char>(index + 1);
        index = index + 1;
    }
    FT_ASSERT_EQ(1, scma_write(handle, 0, initial_payload, 4));
    FT_ASSERT_EQ(1, scma_resize(handle, 8));
    FT_ASSERT_EQ(static_cast<ft_size_t>(8), scma_get_size(handle));
    std::memset(buffer, 0, sizeof(buffer));
    FT_ASSERT_EQ(1, scma_read(handle, 0, buffer, 4));
    index = 0;
    while (index < 4)
    {
        FT_ASSERT_EQ(initial_payload[static_cast<size_t>(index)], buffer[static_cast<size_t>(index)]);
        index = index + 1;
    }
    buffer[0] = 9;
    buffer[1] = 8;
    buffer[2] = 7;
    buffer[3] = 6;
    buffer[4] = 5;
    buffer[5] = 4;
    buffer[6] = 3;
    buffer[7] = 2;
    FT_ASSERT_EQ(1, scma_write(handle, 0, buffer, 8));
    FT_ASSERT_EQ(1, scma_resize(handle, 2));
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), scma_get_size(handle));
    std::memset(initial_payload, 0, sizeof(initial_payload));
    FT_ASSERT_EQ(1, scma_read(handle, 0, initial_payload, 2));
    FT_ASSERT_EQ(static_cast<unsigned char>(9), initial_payload[0]);
    FT_ASSERT_EQ(static_cast<unsigned char>(8), initial_payload[1]);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_resize_rejects_zero_size, "scma resize rejects requests to shrink to zero bytes")
{
    scma_handle handle;

    FT_ASSERT_EQ(1, scma_test_initialize(32));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(0, scma_resize(handle, 0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_snapshot_returns_independent_copy, "scma snapshot duplicates payload")
{
    scma_handle handle;
    int values[3];
    ft_size_t index;
    void *snapshot;
    ft_size_t snapshot_size;
    int *snapshot_values;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int) * 3)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int) * 3));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    index = 0;
    while (index < 3)
    {
        ft_size_t offset;

        values[static_cast<size_t>(index)] = static_cast<int>(index * 10 + 1);
        offset = static_cast<ft_size_t>(sizeof(int));
        if (offset != 0)
            offset = offset * index;
        FT_ASSERT_EQ(1, scma_write(handle,
                offset,
                &values[static_cast<size_t>(index)],
                static_cast<ft_size_t>(sizeof(int))));
        index = index + 1;
    }
    snapshot = scma_snapshot(handle, &snapshot_size);
    FT_ASSERT_EQ(0, snapshot == ft_nullptr);
    FT_ASSERT_EQ(static_cast<ft_size_t>(sizeof(int) * 3), snapshot_size);
    snapshot_values = static_cast<int *>(snapshot);
    FT_ASSERT_EQ(values[0], snapshot_values[0]);
    FT_ASSERT_EQ(values[1], snapshot_values[1]);
    FT_ASSERT_EQ(values[2], snapshot_values[2]);
    values[0] = 101;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &values[0], static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(values[0], snapshot_values[0]);
    std::free(snapshot);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_write_and_read_reject_null_pointers, "scma read and write protect against null pointers")
{
    scma_handle handle;
    int value;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(0, scma_write(handle, 0, ft_nullptr, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_errno);
    value = 0;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(0, scma_read(handle, 0, ft_nullptr, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_errno);
    scma_shutdown();
    return (1);
}
