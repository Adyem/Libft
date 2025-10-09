#include "libft.hpp"
#include "libft_utf8.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdint>

static size_t ft_utf16_effective_length(const char16_t *input, size_t input_length)
{
    size_t computed_length;

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

static size_t ft_utf32_effective_length(const char32_t *input, size_t input_length)
{
    size_t computed_length;

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

size_t ft_wstrlen(const wchar_t *string)
{
    size_t length;

    ft_errno = ER_SUCCESS;
    if (string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    length = 0;
    while (string[length] != L'\0')
    {
        length++;
    }
    return (length);
}

static int ft_utf16_decode_unit(const char16_t *input, size_t length,
        size_t index, uint32_t *code_point_pointer, size_t *advance)
{
    char16_t first_unit;

    first_unit = input[index];
    if (first_unit >= 0xD800 && first_unit <= 0xDBFF)
    {
        if (index + 1 >= length)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (FT_FAILURE);
        }
        char16_t second_unit;

        second_unit = input[index + 1];
        if (second_unit < 0xDC00 || second_unit > 0xDFFF)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
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
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    *code_point_pointer = static_cast<uint32_t>(first_unit);
    *advance = 1;
    return (FT_SUCCESS);
}

ft_string ft_utf16_to_utf8(const char16_t *input, size_t input_length)
{
    ft_string result;
    size_t length;
    size_t index;

    ft_errno = ER_SUCCESS;
    if (input == ft_nullptr && input_length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_string(FT_ERR_INVALID_ARGUMENT));
    }
    length = ft_utf16_effective_length(input, input_length);
    index = 0;
    while (index < length)
    {
        uint32_t code_point;
        size_t advance;
        char encoded_buffer[5];
        size_t encoded_length;

        if (ft_utf16_decode_unit(input, length, index, &code_point, &advance) != FT_SUCCESS)
            return (ft_string(ft_errno));
        encoded_length = 0;
        if (ft_utf8_encode(code_point, encoded_buffer, sizeof(encoded_buffer),
                &encoded_length) != FT_SUCCESS)
            return (ft_string(ft_errno));
        result.append(encoded_buffer, encoded_length);
        if (result.get_error() != ER_SUCCESS)
            return (ft_string(result.get_error()));
        index += advance;
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

ft_string ft_utf32_to_utf8(const char32_t *input, size_t input_length)
{
    ft_string result;
    size_t length;
    size_t index;

    ft_errno = ER_SUCCESS;
    if (input == ft_nullptr && input_length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_string(FT_ERR_INVALID_ARGUMENT));
    }
    length = ft_utf32_effective_length(input, input_length);
    index = 0;
    while (index < length)
    {
        uint32_t code_point;
        char encoded_buffer[5];
        size_t encoded_length;

        code_point = static_cast<uint32_t>(input[index]);
        if (code_point >= 0xD800 && code_point <= 0xDFFF)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_string(FT_ERR_INVALID_ARGUMENT));
        }
        if (code_point > 0x10FFFF)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_string(FT_ERR_INVALID_ARGUMENT));
        }
        encoded_length = 0;
        if (ft_utf8_encode(code_point, encoded_buffer, sizeof(encoded_buffer),
                &encoded_length) != FT_SUCCESS)
            return (ft_string(ft_errno));
        result.append(encoded_buffer, encoded_length);
        if (result.get_error() != ER_SUCCESS)
            return (ft_string(result.get_error()));
        index++;
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

static char16_t *ft_allocate_utf16(size_t code_unit_count)
{
    char16_t *buffer;

    buffer = static_cast<char16_t *>(cma_malloc((code_unit_count + 1) * sizeof(char16_t)));
    if (buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    buffer[code_unit_count] = static_cast<char16_t>(0);
    return (buffer);
}

char16_t *ft_utf8_to_utf16(const char *input, size_t input_length, size_t *output_length_pointer)
{
    size_t effective_length;
    size_t index;
    size_t code_unit_count;
    char16_t *result;

    ft_errno = ER_SUCCESS;
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = 0;
    if (input == ft_nullptr && input_length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    effective_length = input_length;
    if (effective_length == 0 && input != ft_nullptr)
        effective_length = ft_strlen_size_t(input);
    index = 0;
    code_unit_count = 0;
    while (index < effective_length)
    {
        size_t working_index;
        uint32_t code_point;
        size_t sequence_length;

        working_index = index;
        code_point = 0;
        sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
            return (ft_nullptr);
        if (code_point <= 0xFFFF)
        {
            if (code_point >= 0xD800 && code_point <= 0xDFFF)
            {
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (ft_nullptr);
            }
            code_unit_count++;
        }
        else
        {
            code_unit_count += 2;
        }
        index = working_index;
    }
    result = ft_allocate_utf16(code_unit_count);
    if (result == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    size_t output_index;

    output_index = 0;
    while (index < effective_length)
    {
        size_t working_index;
        uint32_t code_point;
        size_t sequence_length;

        working_index = index;
        code_point = 0;
        sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
        {
            int decode_error;

            decode_error = ft_errno;
            cma_free(result);
            ft_errno = decode_error;
            return (ft_nullptr);
        }
        if (code_point <= 0xFFFF)
        {
            if (code_point >= 0xD800 && code_point <= 0xDFFF)
            {
                cma_free(result);
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (ft_nullptr);
            }
            result[output_index] = static_cast<char16_t>(code_point);
            output_index++;
        }
        else
        {
            uint32_t adjusted_value;

            adjusted_value = code_point - 0x10000;
            result[output_index] = static_cast<char16_t>(0xD800 + (adjusted_value >> 10));
            result[output_index + 1] = static_cast<char16_t>(0xDC00 + (adjusted_value & 0x3FF));
            output_index += 2;
        }
        index = working_index;
    }
    result[output_index] = static_cast<char16_t>(0);
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = output_index;
    ft_errno = ER_SUCCESS;
    return (result);
}

char32_t *ft_utf8_to_utf32(const char *input, size_t input_length, size_t *output_length_pointer)
{
    size_t effective_length;
    size_t index;
    size_t code_point_count;
    char32_t *result;

    ft_errno = ER_SUCCESS;
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = 0;
    if (input == ft_nullptr && input_length != 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    effective_length = input_length;
    if (effective_length == 0 && input != ft_nullptr)
        effective_length = ft_strlen_size_t(input);
    index = 0;
    code_point_count = 0;
    while (index < effective_length)
    {
        size_t working_index;
        uint32_t code_point;
        size_t sequence_length;

        working_index = index;
        code_point = 0;
        sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
            return (ft_nullptr);
        code_point_count++;
        index = working_index;
    }
    result = static_cast<char32_t *>(cma_malloc((code_point_count + 1) * sizeof(char32_t)));
    if (result == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    index = 0;
    size_t output_index;

    output_index = 0;
    while (index < effective_length)
    {
        size_t working_index;
        uint32_t code_point;
        size_t sequence_length;

        working_index = index;
        code_point = 0;
        sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index, &code_point,
                &sequence_length) != FT_SUCCESS)
        {
            int decode_error;

            decode_error = ft_errno;
            cma_free(result);
            ft_errno = decode_error;
            return (ft_nullptr);
        }
        result[output_index] = static_cast<char32_t>(code_point);
        output_index++;
        index = working_index;
    }
    result[output_index] = static_cast<char32_t>(0);
    if (output_length_pointer != ft_nullptr)
        *output_length_pointer = output_index;
    ft_errno = ER_SUCCESS;
    return (result);
}
