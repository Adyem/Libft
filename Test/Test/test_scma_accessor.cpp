#include <utility>
#include "test_scma_shared.hpp"

struct scma_test_vector3
{
    float x;
    float y;
    float z;
};

struct scma_test_quad
{
    double a;
    double b;
    double c;
    double d;
};

FT_TEST(test_scma_accessor_struct_pointer_semantics, "scma handle accessor pointer operations write through")
{
    scma_handle handle;
    scma_handle_accessor<scma_test_pair> accessor;
    scma_test_pair result;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_pair))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_pair)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    {
        auto proxy = accessor.operator->();
        proxy->first = 7;
        proxy->second = 14;
    }
    FT_ASSERT_EQ(1, accessor.read_struct(result));
    FT_ASSERT_EQ(7, result.first);
    FT_ASSERT_EQ(14, result.second);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_float_array_semantics, "scma accessor supports float arrays through indexing")
{
    scma_handle handle;
    scma_handle_accessor<float> accessor;
    float write_values[3];
    float read_value;
    ft_size_t index;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(float) * 3)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(float) * 3));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    index = 0;
    while (index < 3)
    {
        write_values[static_cast<size_t>(index)] = static_cast<float>(index) + 0.5f;
        accessor[index] = write_values[static_cast<size_t>(index)];
        index = index + 1;
    }
    read_value = accessor[2];
    FT_ASSERT_EQ(write_values[2], read_value);
    read_value = accessor[0];
    FT_ASSERT_EQ(write_values[0], read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_array_indexing, "scma handle accessor array indexing updates elements")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int) * 3)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int) * 3));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0] = 5;
    accessor[1] = 10;
    accessor[2] = 15;
    read_value = accessor[1];
    FT_ASSERT_EQ(10, read_value);
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(handle, static_cast<ft_size_t>(sizeof(int) * 2), &read_value, static_cast<ft_size_t>(sizeof(int))));
    FT_ASSERT_EQ(15, read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_struct_vector_semantics, "scma accessor pointer operators handle struct arrays")
{
    scma_handle handle;
    scma_handle_accessor<scma_test_vector3> accessor;
    scma_test_vector3 vector_value;
    scma_test_vector3 read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_vector3) * 2)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_vector3) * 2));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    {
        auto proxy = accessor.operator->();
        proxy->x = 1.0f;
        proxy->y = 2.0f;
        proxy->z = 3.0f;
    }
    vector_value.x = 4.0f;
    vector_value.y = 5.0f;
    vector_value.z = 6.0f;
    FT_ASSERT_EQ(1, accessor.write_struct(vector_value));
    read_value.x = 0.0f;
    read_value.y = 0.0f;
    read_value.z = 0.0f;
    FT_ASSERT_EQ(1, accessor.read_struct(read_value));
    FT_ASSERT_EQ(vector_value.x, read_value.x);
    FT_ASSERT_EQ(vector_value.y, read_value.y);
    FT_ASSERT_EQ(vector_value.z, read_value.z);
    {
        auto element_proxy = accessor[1];
        auto element_pointer = element_proxy.operator->();
        element_pointer->x = 7.0f;
        element_pointer->y = 8.0f;
        element_pointer->z = 9.0f;
    }
    FT_ASSERT_EQ(1, accessor.read_struct(read_value));
    FT_ASSERT_EQ(vector_value.x, read_value.x);
    FT_ASSERT_EQ(vector_value.y, read_value.y);
    FT_ASSERT_EQ(vector_value.z, read_value.z);
    read_value.x = 0.0f;
    read_value.y = 0.0f;
    read_value.z = 0.0f;
    FT_ASSERT_EQ(1, accessor.read_at(read_value, 1));
    FT_ASSERT_EQ(7.0f, read_value.x);
    FT_ASSERT_EQ(8.0f, read_value.y);
    FT_ASSERT_EQ(9.0f, read_value.z);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_copy_and_move_semantics, "scma accessor copy and move constructors maintain binding")
{
    scma_handle handle;
    scma_handle_accessor<scma_test_pair> primary_accessor;
    scma_test_pair pair_value;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_pair))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_pair)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, primary_accessor.bind(handle));
    {
        auto proxy = primary_accessor.operator->();
        proxy->first = 55;
        proxy->second = 65;
    }
    scma_handle_accessor<scma_test_pair> copy_accessor(primary_accessor);
    FT_ASSERT_EQ(1, copy_accessor.is_bound());
    {
        auto proxy = copy_accessor.operator->();
        FT_ASSERT_EQ(55, proxy->first);
        FT_ASSERT_EQ(65, proxy->second);
    }
    scma_handle_accessor<scma_test_pair> moved_accessor(std::move(copy_accessor));
    FT_ASSERT_EQ(1, moved_accessor.is_bound());
    FT_ASSERT_EQ(0, copy_accessor.is_bound());
    pair_value.first = 77;
    pair_value.second = 88;
    FT_ASSERT_EQ(1, moved_accessor.write_struct(pair_value));
    FT_ASSERT_EQ(1, primary_accessor.read_struct(pair_value));
    FT_ASSERT_EQ(77, pair_value.first);
    FT_ASSERT_EQ(88, pair_value.second);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_double_struct_semantics, "scma accessor maintains precision for double structs")
{
    scma_handle handle;
    scma_handle_accessor<scma_test_quad> accessor;
    scma_test_quad write_value;
    scma_test_quad read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_quad))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_quad)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    write_value.a = 1.125;
    write_value.b = 2.25;
    write_value.c = 3.5;
    write_value.d = 4.75;
    FT_ASSERT_EQ(1, accessor.write_struct(write_value));
    FT_ASSERT_EQ(1, accessor.read_struct(read_value));
    FT_ASSERT_EQ(write_value.a, read_value.a);
    FT_ASSERT_EQ(write_value.b, read_value.b);
    FT_ASSERT_EQ(write_value.c, read_value.c);
    FT_ASSERT_EQ(write_value.d, read_value.d);
    {
        auto proxy = accessor.operator->();
        proxy->a = proxy->a + 1.0;
        proxy->d = proxy->d + 2.0;
    }
    FT_ASSERT_EQ(1, accessor.read_struct(read_value));
    FT_ASSERT_EQ(2.125, read_value.a);
    FT_ASSERT_EQ(2.25, read_value.b);
    FT_ASSERT_EQ(3.5, read_value.c);
    FT_ASSERT_EQ(6.75, read_value.d);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_dereference_reads_value, "scma accessor dereference returns first element")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;
    int read_value;
    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(int) * 2)));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(int) * 2));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0] = 12;
    accessor[1] = 34;
    read_value = *accessor;
    FT_ASSERT_EQ(12, read_value);
    const scma_handle_accessor<int> const_accessor(accessor);
    FT_ASSERT_EQ(ER_SUCCESS, const_accessor.get_error());
    read_value = const_accessor[1];
    FT_ASSERT_EQ(34, read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_bind_rejects_invalid_handle, "scma accessor bind rejects invalid handles")
{
    scma_handle_accessor<int> accessor;
    scma_handle invalid_handle;

    FT_ASSERT_EQ(1, scma_test_initialize(32));
    invalid_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    invalid_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    FT_ASSERT_EQ(0, accessor.bind(invalid_handle));
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, accessor.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_detects_stale_handle, "scma accessor detects when bound handle is freed")
{
    scma_handle handle;
    scma_handle_accessor<scma_test_pair> accessor;
    scma_test_pair pair_value;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_pair))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_pair)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    FT_ASSERT_EQ(1, scma_free(handle));
    FT_ASSERT_EQ(0, accessor.read_struct(pair_value));
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, accessor.get_error());
    scma_shutdown();
    return (1);
}
