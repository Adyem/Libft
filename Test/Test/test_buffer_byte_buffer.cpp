#include "../test_internal.hpp"
#include "../../Modules/Buffer/byte_buffer.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_buffer_byte_buffer_append_and_read_bytes)
{
    ft_byte_buffer buffer;
    uint8_t input[4];
    uint8_t output[4];

    input[0] = 1;
    input[1] = 2;
    input[2] = 3;
    input[3] = 4;
    output[0] = 0;
    output[1] = 0;
    output[2] = 0;
    output[3] = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append(input, 4));
    FT_ASSERT_EQ(4, buffer.size());
    FT_ASSERT_EQ(4, buffer.remaining());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read(output, 4));
    FT_ASSERT_EQ(0, ft_memcmp(input, output, 4));
    FT_ASSERT_EQ(0, buffer.remaining());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.destroy());
    return (1);
}

FT_TEST(test_buffer_byte_buffer_consume_advances_read_position)
{
    ft_byte_buffer buffer;
    uint8_t input[3];
    uint8_t value;

    input[0] = 9;
    input[1] = 8;
    input[2] = 7;
    value = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append(input, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.consume(2));
    FT_ASSERT_EQ(2, buffer.read_position());
    FT_ASSERT_EQ(1, buffer.remaining());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read_u8(&value));
    FT_ASSERT_EQ(7, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.destroy());
    return (1);
}

FT_TEST(test_buffer_byte_buffer_fixed_capacity_rejects_growth)
{
    ft_byte_buffer buffer;
    uint8_t input[3];

    input[0] = 1;
    input[1] = 2;
    input[2] = 3;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize(2, FT_TRUE));
    FT_ASSERT_EQ(FT_TRUE, buffer.is_fixed_capacity());
    FT_ASSERT_EQ(FT_ERR_FULL, buffer.append(input, 3));
    FT_ASSERT_EQ(0, buffer.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append(input, 2));
    FT_ASSERT_EQ(2, buffer.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.destroy());
    return (1);
}

FT_TEST(test_buffer_byte_buffer_endian_helpers_roundtrip)
{
    ft_byte_buffer buffer;
    uint16_t value_u16_be;
    uint16_t value_u16_le;
    uint32_t value_u32_be;
    uint32_t value_u32_le;
    uint64_t value_u64_be;
    uint64_t value_u64_le;

    value_u16_be = 0;
    value_u16_le = 0;
    value_u32_be = 0;
    value_u32_le = 0;
    value_u64_be = 0;
    value_u64_le = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u16_be(0x1234U));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u16_le(0x5678U));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u32_be(0x12345678U));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u32_le(0x90ABCDEFU));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u64_be(0x0123456789ABCDEFULL));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append_u64_le(0xFEDCBA9876543210ULL));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read_u16_be(&value_u16_be));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read_u16_le(&value_u16_le));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read_u32_be(&value_u32_be));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read_u32_le(&value_u32_le));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read_u64_be(&value_u64_be));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.read_u64_le(&value_u64_le));
    FT_ASSERT_EQ(0x1234U, value_u16_be);
    FT_ASSERT_EQ(0x5678U, value_u16_le);
    FT_ASSERT_EQ(0x12345678U, value_u32_be);
    FT_ASSERT_EQ(0x90ABCDEFU, value_u32_le);
    FT_ASSERT_EQ(0x0123456789ABCDEFULL, value_u64_be);
    FT_ASSERT_EQ(0xFEDCBA9876543210ULL, value_u64_le);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.destroy());
    return (1);
}

FT_TEST(test_buffer_byte_buffer_view_checks_bounds)
{
    ft_byte_buffer buffer;
    uint8_t input[4];
    const uint8_t *view_pointer;

    input[0] = 10;
    input[1] = 20;
    input[2] = 30;
    input[3] = 40;
    view_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.append(input, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.view(1, 2, &view_pointer));
    FT_ASSERT(view_pointer != ft_nullptr);
    FT_ASSERT_EQ(20, view_pointer[0]);
    FT_ASSERT_EQ(30, view_pointer[1]);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, buffer.view(3, 2, &view_pointer));
    FT_ASSERT(view_pointer == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.destroy());
    return (1);
}

FT_TEST(test_buffer_byte_buffer_copy_preserves_state)
{
    ft_byte_buffer source;
    ft_byte_buffer copy;
    uint8_t value;

    value = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.append_u8(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.append_u8(2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.consume(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    FT_ASSERT_EQ(1, copy.read_position());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.read_u8(&value));
    FT_ASSERT_EQ(2, value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    return (1);
}

FT_TEST(test_buffer_byte_buffer_thread_safety_toggle)
{
    ft_byte_buffer buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_FALSE, buffer.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, buffer.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, buffer.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.destroy());
    return (1);
}
