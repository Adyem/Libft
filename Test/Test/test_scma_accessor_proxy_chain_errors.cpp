#include "../test_internal.hpp"
#include "test_scma_shared.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

struct scma_chain_value
{
    int number;
    int bump(void);
};

int scma_chain_value::bump(void)
{
    this->number = this->number + 1;
    return (this->number);
}

FT_TEST(test_scma_proxy_invalid_index_reports_error,
    "scma proxy reports out-of-range error on invalid index")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));

    auto element_proxy = accessor[5];

    FT_ASSERT_EQ(0, element_proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, element_proxy.get_error());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, accessor.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_proxy_invalid_chain_arrow_returns_safe_pointer,
    "scma proxy arrow access on invalid index returns safe pointer")
{
    scma_handle handle;
    scma_handle_accessor<scma_chain_value> accessor;
    scma_chain_value read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(scma_chain_value)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(scma_chain_value));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0].operator->()->number = 11;

    auto element_proxy = accessor[3];
    scma_chain_value *element_pointer = element_proxy.operator->();

    FT_ASSERT(element_pointer != ft_nullptr);
    element_pointer->number = 42;
    FT_ASSERT_EQ(1, accessor.read_at(read_value, 0));
    FT_ASSERT_EQ(11, read_value.number);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_proxy_invalid_assignment_is_noop,
    "scma proxy assignment on invalid index does not modify backing storage")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0] = 9;

    auto element_proxy = accessor[2];
    element_proxy = 77;

    FT_ASSERT_EQ(1, accessor.read_at(read_value, 0));
    FT_ASSERT_EQ(9, read_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, element_proxy.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_proxy_failure_does_not_block_next_valid_operation,
    "scma proxy failure does not poison later valid operations")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));

    auto invalid_proxy = accessor[9];
    FT_ASSERT_EQ(0, invalid_proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, accessor.get_error());

    accessor[0] = 25;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.get_error());
    FT_ASSERT_EQ(1, accessor.read_at(read_value, 0));
    FT_ASSERT_EQ(25, read_value);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_proxy_instances_track_errors_independently,
    "scma proxies keep per-instance validity and error state")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0] = 5;

    auto invalid_proxy = accessor[4];
    auto valid_proxy = accessor[0];

    FT_ASSERT_EQ(0, invalid_proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, invalid_proxy.get_error());
    FT_ASSERT_EQ(1, valid_proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, valid_proxy.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_const_proxy_invalid_index_reports_error,
    "scma const proxy reports invalid index through proxy error state")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0] = 6;

    const scma_handle_accessor<int> const_accessor(accessor);
    auto invalid_proxy = const_accessor[8];

    FT_ASSERT_EQ(0, invalid_proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, invalid_proxy.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_const_proxy_invalid_arrow_is_safe,
    "scma const proxy arrow on invalid index returns safe pointer")
{
    scma_handle handle;
    scma_handle_accessor<scma_chain_value> accessor;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(scma_chain_value)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(scma_chain_value));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0].operator->()->number = 13;

    const scma_handle_accessor<scma_chain_value> const_accessor(accessor);
    auto invalid_proxy = const_accessor[9];
    const scma_chain_value *value_pointer = invalid_proxy.operator->();

    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, invalid_proxy.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_proxy_invalid_dereference_uses_fallback,
    "scma proxy invalid dereference uses fallback value without writeback")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));
    accessor[0] = 14;

    auto invalid_proxy = accessor[7];
    int &fallback_reference = *invalid_proxy;
    fallback_reference = 1234;

    FT_ASSERT_EQ(1, accessor.read_at(read_value, 0));
    FT_ASSERT_EQ(14, read_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, invalid_proxy.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_proxy_valid_arrow_writes_back_on_scope_end,
    "scma proxy valid arrow writes updated struct back to storage")
{
    scma_handle handle;
    scma_handle_accessor<scma_chain_value> accessor;
    scma_chain_value read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(scma_chain_value)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(scma_chain_value));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));

    {
        auto element_proxy = accessor[0];
        element_proxy->number = 33;
    }

    FT_ASSERT_EQ(1, accessor.read_at(read_value, 0));
    FT_ASSERT_EQ(33, read_value.number);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.get_error());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_error_is_overwritten_by_success,
    "scma accessor last error resets after a later successful operation")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(1, accessor.bind(handle));

    (void)accessor[10];
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, accessor.get_error());

    FT_ASSERT_EQ(1, accessor.write_at(44, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.get_error());
    FT_ASSERT_EQ(1, accessor.read_at(read_value, 0));
    FT_ASSERT_EQ(44, read_value);
    scma_shutdown();
    return (1);
}
