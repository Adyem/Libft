#include "encoding.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/utf8.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

static thread_local int32_t g_encoding_error = FT_ERR_SUCCESS;

static int32_t encoding_set_error(int32_t error_code) noexcept
{
    g_encoding_error = error_code;
    return (error_code);
}

int32_t encoding_get_error(void) noexcept
{
    return (g_encoding_error);
}

const char *encoding_get_error_str(void) noexcept
{
    return (ft_strerror(g_encoding_error));
}

static ft_bool encoding_add_overflows(ft_size_t left, ft_size_t right) noexcept
{
    if (left > FT_SYSTEM_SIZE_MAX - right)
        return (FT_TRUE);
    return (FT_FALSE);
}

static int32_t encoding_base64_value(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
        return (character - 'A');
    if (character >= 'a' && character <= 'z')
        return (character - 'a' + 26);
    if (character >= '0' && character <= '9')
        return (character - '0' + 52);
    if (character == '+' || character == '-')
        return (62);
    if (character == '/' || character == '_')
        return (63);
    return (-1);
}

static char encoding_base64_character(uint8_t value, ft_bool url_safe) noexcept
{
    const char *standard_table;
    const char *url_table;

    standard_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    url_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    if (url_safe == FT_TRUE)
        return (url_table[value]);
    return (standard_table[value]);
}

static ft_bool encoding_is_unreserved_percent_byte(uint8_t value) noexcept
{
    if (value >= 'A' && value <= 'Z')
        return (FT_TRUE);
    if (value >= 'a' && value <= 'z')
        return (FT_TRUE);
    if (value >= '0' && value <= '9')
        return (FT_TRUE);
    if (value == '-' || value == '_' || value == '.' || value == '~')
        return (FT_TRUE);
    return (FT_FALSE);
}

static int32_t encoding_hex_value(char character) noexcept
{
    if (character >= '0' && character <= '9')
        return (character - '0');
    if (character >= 'a' && character <= 'f')
        return (character - 'a' + 10);
    if (character >= 'A' && character <= 'F')
        return (character - 'A' + 10);
    return (-1);
}

static char *encoding_base64_encode_internal(const uint8_t *input,
    ft_size_t input_size, ft_bool url_safe, ft_bool padded)
{
    ft_size_t output_size;
    ft_size_t output_index;
    ft_size_t input_index;
    uint32_t byte_one;
    uint32_t byte_two;
    uint32_t byte_three;
    char *output;

    if (input == ft_nullptr && input_size != 0)
    {
        encoding_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (input_size > (FT_SYSTEM_SIZE_MAX / 4) * 3)
    {
        encoding_set_error(FT_ERR_OUT_OF_RANGE);
        return (ft_nullptr);
    }
    output_size = ((input_size + 2) / 3) * 4;
    if (url_safe == FT_TRUE && padded == FT_FALSE)
    {
        if (input_size % 3 == 1)
            output_size -= 2;
        else if (input_size % 3 == 2)
            output_size -= 1;
    }
    output = static_cast<char *>(cma_malloc(output_size + 1));
    if (output == ft_nullptr)
    {
        encoding_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        byte_one = input[input_index];
        byte_two = 0;
        byte_three = 0;
        if (input_index + 1 < input_size)
            byte_two = input[input_index + 1];
        if (input_index + 2 < input_size)
            byte_three = input[input_index + 2];
        output[output_index++] = encoding_base64_character(
            static_cast<uint8_t>(byte_one >> 2), url_safe);
        output[output_index++] = encoding_base64_character(
            static_cast<uint8_t>(((byte_one & 0x03U) << 4) | (byte_two >> 4)),
            url_safe);
        if (input_index + 1 < input_size)
            output[output_index++] = encoding_base64_character(
                static_cast<uint8_t>(((byte_two & 0x0FU) << 2)
                    | (byte_three >> 6)), url_safe);
        else if (padded == FT_TRUE)
            output[output_index++] = '=';
        if (input_index + 2 < input_size)
            output[output_index++] = encoding_base64_character(
                static_cast<uint8_t>(byte_three & 0x3FU), url_safe);
        else if (padded == FT_TRUE)
            output[output_index++] = '=';
        input_index += 3;
    }
    output[output_index] = '\0';
    encoding_set_error(FT_ERR_SUCCESS);
    return (output);
}

char *encoding_base64_encode(const uint8_t *input, ft_size_t input_size)
{
    return (encoding_base64_encode_internal(input, input_size, FT_FALSE, FT_TRUE));
}

char *encoding_base64url_encode(const uint8_t *input, ft_size_t input_size,
    ft_bool padded)
{
    return (encoding_base64_encode_internal(input, input_size, FT_TRUE, padded));
}

static uint8_t *encoding_base64_decode_internal(const char *input,
    ft_size_t input_size, ft_size_t *output_size, ft_bool url_safe)
{
    uint8_t *output;
    ft_size_t clean_count;
    ft_size_t padding_count;
    ft_size_t output_capacity;
    ft_size_t input_index;
    ft_size_t output_index;
    char chunk[4];
    ft_size_t chunk_size;
    int32_t values[4];
    uint32_t triple;

    if (output_size != ft_nullptr)
        *output_size = 0;
    if (input == ft_nullptr && input_size != 0)
    {
        encoding_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    clean_count = 0;
    padding_count = 0;
    input_index = 0;
    while (input_index < input_size)
    {
        if (input[input_index] == '=')
            padding_count++;
        else if (encoding_base64_value(input[input_index]) < 0)
        {
            encoding_set_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        clean_count++;
        input_index++;
    }
    if (clean_count % 4 == 1 || padding_count > 2)
    {
        encoding_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    output_capacity = ((clean_count + 3) / 4) * 3;
    output = static_cast<uint8_t *>(cma_malloc(output_capacity + 1));
    if (output == ft_nullptr)
    {
        encoding_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        chunk_size = 0;
        while (chunk_size < 4 && input_index < input_size)
            chunk[chunk_size++] = input[input_index++];
        while (chunk_size < 4)
            chunk[chunk_size++] = '=';
        values[0] = encoding_base64_value(chunk[0]);
        values[1] = encoding_base64_value(chunk[1]);
        values[2] = 0;
        values[3] = 0;
        if (chunk[2] != '=')
            values[2] = encoding_base64_value(chunk[2]);
        if (chunk[3] != '=')
            values[3] = encoding_base64_value(chunk[3]);
        if (values[0] < 0 || values[1] < 0 || values[2] < 0 || values[3] < 0)
        {
            cma_free(output);
            encoding_set_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        if (url_safe == FT_TRUE && (chunk[0] == '+' || chunk[0] == '/'
            || chunk[1] == '+' || chunk[1] == '/'
            || chunk[2] == '+' || chunk[2] == '/'
            || chunk[3] == '+' || chunk[3] == '/'))
        {
            cma_free(output);
            encoding_set_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        triple = (static_cast<uint32_t>(values[0]) << 18)
            | (static_cast<uint32_t>(values[1]) << 12)
            | (static_cast<uint32_t>(values[2]) << 6)
            | static_cast<uint32_t>(values[3]);
        output[output_index++] = static_cast<uint8_t>((triple >> 16) & 0xFFU);
        if (chunk[2] != '=')
            output[output_index++] = static_cast<uint8_t>((triple >> 8) & 0xFFU);
        if (chunk[3] != '=')
            output[output_index++] = static_cast<uint8_t>(triple & 0xFFU);
    }
    if (output_size != ft_nullptr)
        *output_size = output_index;
    encoding_set_error(FT_ERR_SUCCESS);
    return (output);
}

uint8_t *encoding_base64_decode(const char *input, ft_size_t input_size,
    ft_size_t *output_size)
{
    return (encoding_base64_decode_internal(input, input_size, output_size, FT_FALSE));
}

uint8_t *encoding_base64url_decode(const char *input, ft_size_t input_size,
    ft_size_t *output_size)
{
    return (encoding_base64_decode_internal(input, input_size, output_size, FT_TRUE));
}

char *encoding_hex_encode(const uint8_t *input, ft_size_t input_size,
    ft_bool uppercase)
{
    const char *digits;
    char *output;
    ft_size_t input_index;
    ft_size_t output_index;

    if (input == ft_nullptr && input_size != 0)
    {
        encoding_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (input_size > FT_SYSTEM_SIZE_MAX / 2)
    {
        encoding_set_error(FT_ERR_OUT_OF_RANGE);
        return (ft_nullptr);
    }
    digits = "0123456789abcdef";
    if (uppercase == FT_TRUE)
        digits = "0123456789ABCDEF";
    output = static_cast<char *>(cma_malloc((input_size * 2) + 1));
    if (output == ft_nullptr)
    {
        encoding_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        output[output_index++] = digits[(input[input_index] >> 4) & 0x0FU];
        output[output_index++] = digits[input[input_index] & 0x0FU];
        input_index++;
    }
    output[output_index] = '\0';
    encoding_set_error(FT_ERR_SUCCESS);
    return (output);
}

uint8_t *encoding_hex_decode(const char *input, ft_size_t input_size,
    ft_size_t *output_size)
{
    uint8_t *output;
    ft_size_t input_index;
    ft_size_t output_index;
    int32_t high_value;
    int32_t low_value;

    if (output_size != ft_nullptr)
        *output_size = 0;
    if ((input == ft_nullptr && input_size != 0) || input_size % 2 != 0)
    {
        encoding_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    output = static_cast<uint8_t *>(cma_malloc((input_size / 2) + 1));
    if (output == ft_nullptr)
    {
        encoding_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        high_value = encoding_hex_value(input[input_index]);
        low_value = encoding_hex_value(input[input_index + 1]);
        if (high_value < 0 || low_value < 0)
        {
            cma_free(output);
            encoding_set_error(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        output[output_index++] = static_cast<uint8_t>((high_value << 4)
            | low_value);
        input_index += 2;
    }
    if (output_size != ft_nullptr)
        *output_size = output_index;
    encoding_set_error(FT_ERR_SUCCESS);
    return (output);
}

char *encoding_percent_encode(const uint8_t *input, ft_size_t input_size)
{
    const char *digits;
    char *output;
    ft_size_t input_index;
    ft_size_t output_size;
    ft_size_t output_index;

    if (input == ft_nullptr && input_size != 0)
    {
        encoding_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    output_size = 0;
    input_index = 0;
    while (input_index < input_size)
    {
        if (encoding_is_unreserved_percent_byte(input[input_index]) == FT_TRUE)
        {
            if (encoding_add_overflows(output_size, 1) == FT_TRUE)
            {
                encoding_set_error(FT_ERR_OUT_OF_RANGE);
                return (ft_nullptr);
            }
            output_size++;
        }
        else
        {
            if (encoding_add_overflows(output_size, 3) == FT_TRUE)
            {
                encoding_set_error(FT_ERR_OUT_OF_RANGE);
                return (ft_nullptr);
            }
            output_size += 3;
        }
        input_index++;
    }
    output = static_cast<char *>(cma_malloc(output_size + 1));
    if (output == ft_nullptr)
    {
        encoding_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    digits = "0123456789ABCDEF";
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        if (encoding_is_unreserved_percent_byte(input[input_index]) == FT_TRUE)
            output[output_index++] = static_cast<char>(input[input_index]);
        else
        {
            output[output_index++] = '%';
            output[output_index++] = digits[(input[input_index] >> 4) & 0x0FU];
            output[output_index++] = digits[input[input_index] & 0x0FU];
        }
        input_index++;
    }
    output[output_index] = '\0';
    encoding_set_error(FT_ERR_SUCCESS);
    return (output);
}

uint8_t *encoding_percent_decode(const char *input, ft_size_t input_size,
    ft_size_t *output_size)
{
    uint8_t *output;
    ft_size_t input_index;
    ft_size_t output_index;
    int32_t high_value;
    int32_t low_value;

    if (output_size != ft_nullptr)
        *output_size = 0;
    if (input == ft_nullptr && input_size != 0)
    {
        encoding_set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    output = static_cast<uint8_t *>(cma_malloc(input_size + 1));
    if (output == ft_nullptr)
    {
        encoding_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        if (input[input_index] == '%')
        {
            if (input_index + 2 >= input_size)
            {
                cma_free(output);
                encoding_set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            high_value = encoding_hex_value(input[input_index + 1]);
            low_value = encoding_hex_value(input[input_index + 2]);
            if (high_value < 0 || low_value < 0)
            {
                cma_free(output);
                encoding_set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            output[output_index++] = static_cast<uint8_t>((high_value << 4)
                | low_value);
            input_index += 3;
        }
        else
            output[output_index++] = static_cast<uint8_t>(input[input_index++]);
    }
    if (output_size != ft_nullptr)
        *output_size = output_index;
    encoding_set_error(FT_ERR_SUCCESS);
    return (output);
}

int32_t encoding_utf8_validate(const char *input, ft_size_t input_size)
{
    ft_size_t index;
    uint32_t code_point;
    ft_size_t sequence_length;
    int32_t result;

    if (input == ft_nullptr && input_size != 0)
        return (encoding_set_error(FT_ERR_INVALID_ARGUMENT));
    index = 0;
    while (index < input_size)
    {
        result = ft_utf8_next(input, input_size, &index, &code_point,
                &sequence_length);
        if (result != FT_ERR_SUCCESS)
            return (encoding_set_error(result));
    }
    return (encoding_set_error(FT_ERR_SUCCESS));
}

int32_t encoding_utf8_next_codepoint(const char *input, ft_size_t input_size,
    ft_size_t *index_pointer, uint32_t *code_point_pointer)
{
    ft_size_t sequence_length;
    int32_t result;

    result = ft_utf8_next(input, input_size, index_pointer, code_point_pointer,
            &sequence_length);
    return (encoding_set_error(result));
}
