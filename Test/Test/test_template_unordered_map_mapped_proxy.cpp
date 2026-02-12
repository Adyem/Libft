#include "../test_internal.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

struct unordered_map_proxy_value
{
    int number;
    int bump(void);
};

int unordered_map_proxy_value::bump(void)
{
    this->number = this->number + 1;
    return (this->number);
}

FT_TEST(test_unordered_map_mapped_proxy_default_state,
    "unordered_map mapped_proxy default constructor yields invalid state")
{
    ft_unordered_map<int, unordered_map_proxy_value>::mapped_proxy proxy;

    FT_ASSERT_EQ(0, proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, proxy.get_error());
    return (1);
}

FT_TEST(test_unordered_map_mapped_proxy_default_arrow_safe,
    "unordered_map mapped_proxy default arrow returns safe pointer")
{
    ft_unordered_map<int, unordered_map_proxy_value>::mapped_proxy proxy;
    unordered_map_proxy_value *value_pointer;

    value_pointer = proxy.operator->();
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, proxy.get_error());
    return (1);
}

FT_TEST(test_unordered_map_mapped_proxy_default_dereference_safe,
    "unordered_map mapped_proxy default dereference returns fallback reference")
{
    ft_unordered_map<int, unordered_map_proxy_value>::mapped_proxy proxy;
    unordered_map_proxy_value &value_reference = *proxy;

    value_reference.number = 77;
    FT_ASSERT_EQ(77, (*proxy).number);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, proxy.get_error());
    return (1);
}

FT_TEST(test_unordered_map_mapped_proxy_assignment_updates_value,
    "unordered_map mapped_proxy assignment writes mapped value")
{
    ft_unordered_map<int, int> map_instance;
    int value;

    map_instance[1] = 5;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    value = map_instance[1];
    FT_ASSERT_EQ(5, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    return (1);
}

FT_TEST(test_unordered_map_mapped_proxy_arrow_updates_struct,
    "unordered_map mapped_proxy arrow can update struct members")
{
    ft_unordered_map<int, unordered_map_proxy_value> map_instance;

    map_instance[2]->number = 10;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    map_instance[2]->bump();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    FT_ASSERT_EQ(11, map_instance[2]->number);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    return (1);
}

FT_TEST(test_unordered_map_mapped_proxy_missing_key_creates_valid_entry,
    "unordered_map mapped_proxy from missing key creates valid default entry")
{
    ft_unordered_map<int, unordered_map_proxy_value> map_instance;
    auto proxy = map_instance[9];

    FT_ASSERT_EQ(1, proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, proxy.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), map_instance.size());
    return (1);
}

FT_TEST(test_unordered_map_mapped_proxy_conversion_reads_current_value,
    "unordered_map mapped_proxy conversion returns mapped value")
{
    ft_unordered_map<int, int> map_instance;
    int value;

    map_instance[3] = 12;
    value = map_instance[3];
    FT_ASSERT_EQ(12, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    return (1);
}

FT_TEST(test_unordered_map_invalid_proxy_does_not_poison_map_operations,
    "unordered_map invalid standalone proxy does not poison later map operations")
{
    ft_unordered_map<int, unordered_map_proxy_value> map_instance;
    ft_unordered_map<int, unordered_map_proxy_value>::mapped_proxy invalid_proxy;

    invalid_proxy.operator->()->number = 99;
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, invalid_proxy.get_error());

    map_instance[5]->number = 8;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    FT_ASSERT_EQ(8, map_instance[5]->number);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    return (1);
}

FT_TEST(test_unordered_map_invalid_proxy_assignment_is_noop,
    "unordered_map invalid standalone proxy assignment remains invalid")
{
    ft_unordered_map<int, unordered_map_proxy_value>::mapped_proxy invalid_proxy;
    unordered_map_proxy_value value;

    value.number = 123;
    invalid_proxy = value;
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, invalid_proxy.get_error());
    FT_ASSERT_EQ(0, invalid_proxy.is_valid());
    return (1);
}

FT_TEST(test_unordered_map_proxy_chain_and_later_operation_are_independent,
    "unordered_map mapped proxy chain errors do not force later unrelated failures")
{
    ft_unordered_map<int, unordered_map_proxy_value> map_instance;
    ft_unordered_map<int, unordered_map_proxy_value>::mapped_proxy invalid_proxy;

    map_instance[1]->number = 1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());

    invalid_proxy.operator->()->number = 50;
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, invalid_proxy.get_error());

    map_instance[1]->bump();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    FT_ASSERT_EQ(2, map_instance[1]->number);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    return (1);
}
