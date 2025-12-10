#include "../../Compression/compression.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_base64_encode_null_input_sets_errno, "ft_base64_encode null input sets FT_ERR_INVALID_ARGUMENT")
{
    std::size_t encoded_length;

    encoded_length = 123;
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_base64_encode(ft_nullptr, 4, &encoded_length));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), encoded_length);
    return (1);
}

FT_TEST(test_base64_encode_null_size_sets_errno, "ft_base64_encode null encoded_size sets FT_ERR_INVALID_ARGUMENT")
{
    unsigned char input_buffer[3];

    input_buffer[0] = 'a';
    input_buffer[1] = 'b';
    input_buffer[2] = 'c';
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_base64_encode(input_buffer, 3, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_encode_allocation_failure_sets_errno, "ft_base64_encode allocation failure sets FT_ERR_NO_MEMORY")
{
    unsigned char   input_buffer[3];
    unsigned char   *result_buffer;
    std::size_t     encoded_length;

    input_buffer[0] = 'A';
    input_buffer[1] = 'B';
    input_buffer[2] = 'C';
    encoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    cma_set_alloc_limit(2);
    result_buffer = ft_base64_encode(input_buffer, 3, &encoded_length);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, result_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), encoded_length);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_base64_encode_success_resets_errno, "ft_base64_encode success resets errno")
{
    unsigned char   input_buffer[1];
    unsigned char   *encoded_buffer;
    std::size_t     encoded_length;

    input_buffer[0] = '\0';
    encoded_length = 0;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    encoded_buffer = ft_base64_encode(input_buffer, 0, &encoded_length);
    FT_ASSERT(encoded_buffer != ft_nullptr);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), encoded_length);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(encoded_buffer);
    return (1);
}

FT_TEST(test_base64_decode_null_input_sets_errno, "ft_base64_decode null input sets FT_ERR_INVALID_ARGUMENT")
{
    std::size_t decoded_length;

    decoded_length = 77;
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_base64_decode(ft_nullptr, 4, &decoded_length));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(static_cast<std::size_t>(77), decoded_length);
    return (1);
}

FT_TEST(test_base64_decode_null_size_sets_errno, "ft_base64_decode null decoded_size sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char *input_buffer;

    input_buffer = reinterpret_cast<const unsigned char *>("AAAA");
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_base64_decode(input_buffer, 4, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_empty_input_resets_errno, "ft_base64_decode empty input resets errno")
{
    unsigned char   *decoded_buffer;
    std::size_t     decoded_length;

    decoded_length = 99;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    decoded_buffer = ft_base64_decode(reinterpret_cast<const unsigned char *>(""), 0, &decoded_length);
    FT_ASSERT(decoded_buffer != ft_nullptr);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(decoded_buffer);
    return (1);
}

FT_TEST(test_base64_decode_short_block_sets_errno, "ft_base64_decode short input sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("A");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    decoded_buffer = ft_base64_decode(input_buffer, 1, &decoded_length);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_allocation_failure_sets_errno, "ft_base64_decode allocation failure sets FT_ERR_NO_MEMORY")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("AAAA");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    cma_set_alloc_limit(2);
    decoded_buffer = ft_base64_decode(input_buffer, 4, &decoded_length);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_invalid_first_character_sets_errno, "ft_base64_decode invalid first char sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("!AAA");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    decoded_buffer = ft_base64_decode(input_buffer, 4, &decoded_length);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_invalid_second_character_sets_errno, "ft_base64_decode invalid second char sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("A!AA");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    decoded_buffer = ft_base64_decode(input_buffer, 4, &decoded_length);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_invalid_third_character_sets_errno, "ft_base64_decode invalid third char sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("AA!A");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    decoded_buffer = ft_base64_decode(input_buffer, 4, &decoded_length);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_invalid_fourth_character_sets_errno, "ft_base64_decode invalid fourth char sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("AAA!");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    decoded_buffer = ft_base64_decode(input_buffer, 4, &decoded_length);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_invalid_padding_mismatch_sets_errno, "ft_base64_decode padding mismatch sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("AA=A");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    decoded_buffer = ft_base64_decode(input_buffer, 4, &decoded_length);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_trailing_data_after_padding_sets_errno, "ft_base64_decode trailing data after padding sets FT_ERR_INVALID_ARGUMENT")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("AAA=AAAA");
    decoded_length = 0;
    ft_errno = FT_ER_SUCCESSS;
    decoded_buffer = ft_base64_decode(input_buffer, 8, &decoded_length);
    FT_ASSERT_EQ(ft_nullptr, decoded_buffer);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), decoded_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_base64_decode_success_resets_errno, "ft_base64_decode success resets errno")
{
    const unsigned char   *input_buffer;
    unsigned char         *decoded_buffer;
    std::size_t           decoded_length;

    input_buffer = reinterpret_cast<const unsigned char *>("TQ==");
    decoded_length = 0;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    decoded_buffer = ft_base64_decode(input_buffer, 4, &decoded_length);
    FT_ASSERT(decoded_buffer != ft_nullptr);
    FT_ASSERT_EQ(static_cast<std::size_t>(1), decoded_length);
    FT_ASSERT_EQ('M', decoded_buffer[0]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(decoded_buffer);
    return (1);
}

