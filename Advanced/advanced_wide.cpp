#include "advanced.hpp"
#include "../Basic/utf8.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdint>
#include <new>
#include "../Basic/basic.hpp"

static ft_size_t ft_utf16_effective_length(const char16_t *input,
                                                ft_size_t input_length)
{
    ft_size_t computed_length;

    computed_length = input_length;
    if (computed_length != 0 || input == ft_nullptr)
        return (computed_length);
    computed_length = 0;
    while (input[computed_length] != 0)
    {
        computed_length++;
    }
    return (computed_length);
}

static ft_size_t ft_utf32_effective_length(const char32_t *input,
                                                ft_size_t input_length)
{
    ft_size_t computed_length;

    computed_length = input_length;
    if (computed_length != 0 || input == ft_nullptr)
        return (computed_length);
    computed_length = 0;
    while (input[computed_length] != 0)
    {
        computed_length++;
    }
    return (computed_length);
}

static int32_t ft_utf16_decode_unit(const char16_t *input, ft_size_t length,
        ft_size_t index, uint32_t *code_point_pointer, ft_size_t *advance)
{
    char16_t first_unit;

    first_unit = input[index];
    if (first_unit >= 0xD800 && first_unit <= 0xDBFF)
    {
        if (index + 1 >= length)
        {
            return (FT_FAILURE);
        }
        char16_t second_unit;

        second_unit = input[index + 1];
        if (second_unit < 0xDC00 || second_unit > 0xDFFF)
        {
            return (FT_FAILURE);
        }
        *code_point_pointer = 0x10000;
        *code_point_pointer += static_cast<uint32_t>(first_unit - 0xD800) << 10;
        *code_point_pointer += static_cast<uint32_t>(second_unit - 0xDC00);
        *advance = 2;
        return (FT_SUCCESS);
    }
    if (first_unit >= 0xDC00 && first_unit <= 0xDFFF)
    {
        return (FT_FAILURE);
    }
    *code_point_pointer = static_cast<uint32_t>(first_unit);
    *advance = 1;
    return (FT_SUCCESS);
}

static ft_string *create_empty_string(void)
{
    ft_string *result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    if (result->initialize() != FT_ERR_SUCCESSS)
    {
        delete result;
        return (ft_nullptr);
    }
    return (result);
}

ft_string *adv_utf16_to_utf8(const char16_t *input, ft_size_t input_length)
{
    if (input == ft_nullptr && input_length != 0)
        return (ft_nullptr);
    ft_string *result = create_empty_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    ft_size_t length = ft_utf16_effective_length(input, input_length);
    ft_size_t index = 0;
    while (index < length)
    {
        uint32_t code_point;
        ft_size_t advance;
        char encoded_buffer[5];
        ft_size_t encoded_length;

        if (ft_utf16_decode_unit(input, length, index, &code_point, &advance)
                != FT_SUCCESS)
        {
            delete result;
            return (ft_nullptr);
        }
        if (ft_utf8_encode(code_point, encoded_buffer, sizeof(encoded_buffer),
                &encoded_length) != FT_SUCCESS)
        {
            delete result;
            return (ft_nullptr);
        }
        if (result->append(encoded_buffer, encoded_length) != FT_ERR_SUCCESSS)
        {
            delete result;
            return (ft_nullptr);
        }
        index += advance;
    }
    return (result);
}

ft_string *adv_utf32_to_utf8(const char32_t *input, ft_size_t input_length)
{
    if (input == ft_nullptr && input_length != 0)
        return (ft_nullptr);
    ft_string *result = create_empty_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    ft_size_t length = ft_utf32_effective_length(input, input_length);
    ft_size_t index = 0;
    while (index < length)
    {
        uint32_t code_point;
        char encoded_buffer[5];
        ft_size_t encoded_length;

        code_point = static_cast<uint32_t>(input[index]);
        if (code_point >= 0xD800 && code_point <= 0xDFFF)
        {
            delete result;
            return (ft_nullptr);
        }
        if (code_point > 0x10FFFF)
        {
            delete result;
            return (ft_nullptr);
        }
        if (ft_utf8_encode(code_point, encoded_buffer, sizeof(encoded_buffer),
                &encoded_length) != FT_SUCCESS)
        {
            delete result;
            return (ft_nullptr);
        }
        if (result->append(encoded_buffer, encoded_length) != FT_ERR_SUCCESSS)
        {
            delete result;
            return (ft_nullptr);
        }
        index++;
    }
    return (result);
}

static char16_t *ft_allocate_utf16(ft_size_t code_unit_count)
{
    char16_t *buffer;

    buffer = static_cast<char16_t *>(cma_malloc((code_unit_count + 1)
                * sizeof(char16_t)));
    if (buffer == ft_nullptr)
        return (ft_nullptr);
    buffer[code_unit_count] = static_cast<char16_t>(0);
    return (buffer);
}

char16_t *ft_utf8_to_utf16(const char *input, ft_size_t input_length,
                                ft_size_t *output_length_pointer)
{
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = 0;
    if (input == ft_nullptr && input_length != 0)
        return (ft_nullptr);
    ft_size_t effective_length = input_length;
    if (effective_length == 0 && input != ft_nullptr)
        effective_length = ft_strlen_size_t(input);
    ft_size_t index = 0;
    ft_size_t code_unit_count = 0;
    while (index < effective_length)
    {
        ft_size_t working_index = index;
        uint32_t code_point = 0;
        ft_size_t sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
            return (ft_nullptr);
        if (code_point <= 0xFFFF)
        {
            if (code_point >= 0xD800 && code_point <= 0xDFFF)
                return (ft_nullptr);
            code_unit_count++;
        }
        else
            code_unit_count += 2;
        index = working_index;
    }
    char16_t *result = ft_allocate_utf16(code_unit_count);
    if (result == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    ft_size_t output_index = 0;
    while (index < effective_length)
    {
        ft_size_t working_index = index;
        uint32_t code_point = 0;
        ft_size_t sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        if (code_point <= 0xFFFF)
            result[output_index++] = static_cast<char16_t>(code_point);
        else
        {
            uint32_t adjusted_value = code_point - 0x10000;
            result[output_index++] = static_cast<char16_t>(0xD800
                    + (adjusted_value >> 10));
            result[output_index++] = static_cast<char16_t>(0xDC00
                    + (adjusted_value & 0x3FF));
        }
        index = working_index;
    }
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = output_index;
    return (result);
}


char32_t *ft_utf8_to_utf32(const char *input, ft_size_t input_length,
                                ft_size_t *output_length_pointer)
{
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = 0;
    if (input == ft_nullptr && input_length != 0)
        return (ft_nullptr);
    ft_size_t effective_length = input_length;
    if (effective_length == 0 && input != ft_nullptr)
        effective_length = ft_strlen_size_t(input);
    ft_size_t index = 0;
    ft_size_t code_point_count = 0;
    while (index < effective_length)
    {
        ft_size_t working_index = index;
        uint32_t code_point = 0;
        ft_size_t sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
            return (ft_nullptr);
        code_point_count++;
        index = working_index;
    }
    char32_t *result = static_cast<char32_t *>(cma_malloc((code_point_count + 1)
                * sizeof(char32_t)));
    if (result == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    ft_size_t output_index = 0;
    while (index < effective_length)
    {
        ft_size_t working_index = index;
        uint32_t code_point = 0;
        ft_size_t sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        result[output_index++] = static_cast<char32_t>(code_point);
        index = working_index;
    }
    result[output_index] = static_cast<char32_t>(0);
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = output_index;
    return (result);
}
