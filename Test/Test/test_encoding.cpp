#include "../test_internal.hpp"
#include "../../Modules/Encoding/encoding.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_encoding_base64_roundtrip)
{
    const uint8_t input[] = {'h', 'e', 'l', 'l', 'o'};
    char *encoded;
    uint8_t *decoded;
    ft_size_t decoded_size;

    decoded_size = 0;
    encoded = encoding_base64_encode(input, 5);
    FT_ASSERT(encoded != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(encoded, "aGVsbG8="));
    decoded = encoding_base64_decode(encoded, ft_strlen_size_t(encoded),
            &decoded_size);
    FT_ASSERT(decoded != ft_nullptr);
    FT_ASSERT_EQ(5, decoded_size);
    FT_ASSERT_EQ(0, ft_memcmp(input, decoded, decoded_size));
    cma_free(encoded);
    cma_free(decoded);
    return (1);
}

FT_TEST(test_encoding_base64url_without_padding)
{
    const uint8_t input[] = {0xFB, 0xEF, 0xFF};
    char *encoded;
    uint8_t *decoded;
    ft_size_t decoded_size;

    decoded_size = 0;
    encoded = encoding_base64url_encode(input, 3, FT_FALSE);
    FT_ASSERT(encoded != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(encoded, "--__"));
    decoded = encoding_base64url_decode(encoded, ft_strlen_size_t(encoded),
            &decoded_size);
    FT_ASSERT(decoded != ft_nullptr);
    FT_ASSERT_EQ(3, decoded_size);
    FT_ASSERT_EQ(0, ft_memcmp(input, decoded, decoded_size));
    cma_free(encoded);
    cma_free(decoded);
    return (1);
}

FT_TEST(test_encoding_base64url_decode_rejects_standard_alphabet)
{
    uint8_t *decoded;
    ft_size_t decoded_size;

    decoded_size = 0;
    decoded = encoding_base64url_decode("+/==", 4, &decoded_size);
    FT_ASSERT(decoded == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, encoding_get_error());
    FT_ASSERT_EQ(0, decoded_size);
    return (1);
}

FT_TEST(test_encoding_hex_roundtrip)
{
    const uint8_t input[] = {0x00, 0xAB, 0xCD, 0xEF};
    char *encoded;
    uint8_t *decoded;
    ft_size_t decoded_size;

    decoded_size = 0;
    encoded = encoding_hex_encode(input, 4, FT_TRUE);
    FT_ASSERT(encoded != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(encoded, "00ABCDEF"));
    decoded = encoding_hex_decode(encoded, ft_strlen_size_t(encoded),
            &decoded_size);
    FT_ASSERT(decoded != ft_nullptr);
    FT_ASSERT_EQ(4, decoded_size);
    FT_ASSERT_EQ(0, ft_memcmp(input, decoded, decoded_size));
    cma_free(encoded);
    cma_free(decoded);
    return (1);
}

FT_TEST(test_encoding_hex_decode_rejects_invalid_input)
{
    uint8_t *decoded;
    ft_size_t decoded_size;

    decoded_size = 0;
    decoded = encoding_hex_decode("0G", 2, &decoded_size);
    FT_ASSERT(decoded == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, encoding_get_error());
    FT_ASSERT_EQ(0, decoded_size);
    return (1);
}

FT_TEST(test_encoding_percent_roundtrip)
{
    const uint8_t input[] = {'a', ' ', '/', '~'};
    char *encoded;
    uint8_t *decoded;
    ft_size_t decoded_size;

    decoded_size = 0;
    encoded = encoding_percent_encode(input, 4);
    FT_ASSERT(encoded != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(encoded, "a%20%2F~"));
    decoded = encoding_percent_decode(encoded, ft_strlen_size_t(encoded),
            &decoded_size);
    FT_ASSERT(decoded != ft_nullptr);
    FT_ASSERT_EQ(4, decoded_size);
    FT_ASSERT_EQ(0, ft_memcmp(input, decoded, decoded_size));
    cma_free(encoded);
    cma_free(decoded);
    return (1);
}

FT_TEST(test_encoding_percent_decode_rejects_short_escape)
{
    uint8_t *decoded;
    ft_size_t decoded_size;

    decoded_size = 0;
    decoded = encoding_percent_decode("abc%", 4, &decoded_size);
    FT_ASSERT(decoded == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, encoding_get_error());
    FT_ASSERT_EQ(0, decoded_size);
    return (1);
}

FT_TEST(test_encoding_utf8_validate_accepts_valid_text)
{
    const char *input;

    input = "hello " "\xF0\x9F\x99\x82";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encoding_utf8_validate(input,
        ft_strlen_size_t(input)));
    return (1);
}

FT_TEST(test_encoding_utf8_validate_rejects_invalid_text)
{
    const char invalid_input[] = {'a', static_cast<char>(0xC0),
        static_cast<char>(0xAF), '\0'};

    FT_ASSERT(encoding_utf8_validate(invalid_input, 3) != FT_ERR_SUCCESS);
    FT_ASSERT(encoding_get_error() != FT_ERR_SUCCESS);
    return (1);
}

FT_TEST(test_encoding_utf8_next_codepoint_iterates)
{
    const char *input;
    ft_size_t index;
    uint32_t code_point;

    input = "A" "\xC3\xA9";
    index = 0;
    code_point = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encoding_utf8_next_codepoint(input,
        ft_strlen_size_t(input), &index, &code_point));
    FT_ASSERT_EQ('A', code_point);
    FT_ASSERT_EQ(1, index);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encoding_utf8_next_codepoint(input,
        ft_strlen_size_t(input), &index, &code_point));
    FT_ASSERT_EQ(0xE9U, code_point);
    FT_ASSERT_EQ(3, index);
    return (1);
}
