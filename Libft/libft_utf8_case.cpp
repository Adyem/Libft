#include "libft.hpp"
#include "libft_utf8.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

uint32_t ft_utf8_case_ascii_lower(uint32_t code_point)
{
    if (code_point >= 'A' && code_point <= 'Z')
        return (code_point + 32);
    return (code_point);
}

uint32_t ft_utf8_case_ascii_upper(uint32_t code_point)
{
    if (code_point >= 'a' && code_point <= 'z')
        return (code_point - 32);
    return (code_point);
}

int ft_utf8_encode(uint32_t code_point, char *buffer, size_t buffer_size,
        size_t *encoded_length_pointer)
{
    size_t required_length;

    if (buffer == ft_nullptr && buffer_size != 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_FAILURE);
    }
    if (code_point > 0x10FFFF)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_FAILURE);
    }
    if (code_point >= 0xD800 && code_point <= 0xDFFF)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_FAILURE);
    }
    if (code_point <= 0x7F)
        required_length = 1;
    else if (code_point <= 0x7FF)
        required_length = 2;
    else if (code_point <= 0xFFFF)
        required_length = 3;
    else
        required_length = 4;
    if (buffer_size <= required_length)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (FT_FAILURE);
    }
    if (required_length == 1)
        buffer[0] = static_cast<char>(code_point);
    else if (required_length == 2)
    {
        buffer[0] = static_cast<char>(0xC0 | ((code_point >> 6) & 0x1F));
        buffer[1] = static_cast<char>(0x80 | (code_point & 0x3F));
    }
    else if (required_length == 3)
    {
        buffer[0] = static_cast<char>(0xE0 | ((code_point >> 12) & 0x0F));
        buffer[1] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | (code_point & 0x3F));
    }
    else
    {
        buffer[0] = static_cast<char>(0xF0 | ((code_point >> 18) & 0x07));
        buffer[1] = static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        buffer[3] = static_cast<char>(0x80 | (code_point & 0x3F));
    }
    buffer[required_length] = '\0';
    if (encoded_length_pointer != ft_nullptr)
        *encoded_length_pointer = required_length;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}

int ft_utf8_transform(const char *input, size_t input_length,
        char *output_buffer, size_t output_buffer_size,
        ft_utf8_case_hook case_hook)
{
    size_t effective_length;
    size_t input_index;
    size_t output_index;
    int error_code;

    if (input == ft_nullptr || output_buffer == ft_nullptr
        || case_hook == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_FAILURE);
    }
    effective_length = input_length;
    if (effective_length == 0)
    {
        effective_length = ft_strlen_size_t(input);
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (FT_FAILURE);
        }
    }
    input_index = 0;
    output_index = 0;
    while (input_index < effective_length)
    {
        size_t working_index;
        uint32_t decoded_code_point;
        size_t sequence_length;
        uint32_t mapped_code_point;
        char encoded_buffer[5];
        size_t encoded_length;
        size_t copy_index;

        working_index = input_index;
        decoded_code_point = 0;
        sequence_length = 0;
        if (ft_utf8_next(input, effective_length, &working_index,
                &decoded_code_point, &sequence_length) != FT_SUCCESS)
        {
            error_code = ft_global_error_stack_drop_last_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_INVALID_ARGUMENT;
            ft_global_error_stack_push(error_code);
            return (FT_FAILURE);
        }
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (FT_FAILURE);
        }
        mapped_code_point = case_hook(decoded_code_point);
        encoded_length = 0;
        if (ft_utf8_encode(mapped_code_point, encoded_buffer,
                sizeof(encoded_buffer), &encoded_length) != FT_SUCCESS)
        {
            error_code = ft_global_error_stack_drop_last_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_INVALID_ARGUMENT;
            ft_global_error_stack_push(error_code);
            return (FT_FAILURE);
        }
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (FT_FAILURE);
        }
        if (output_index + encoded_length >= output_buffer_size)
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (FT_FAILURE);
        }
        copy_index = 0;
        while (copy_index < encoded_length)
        {
            output_buffer[output_index] = encoded_buffer[copy_index];
            output_index++;
            copy_index++;
        }
        input_index = working_index;
    }
    if (output_index >= output_buffer_size)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (FT_FAILURE);
    }
    output_buffer[output_index] = '\0';
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}

int ft_utf8_transform_alloc(const char *input, char **output_pointer,
        ft_utf8_case_hook case_hook)
{
    size_t code_point_count;
    size_t allocation_size;
    char *allocated_buffer;
    size_t input_length;
    int error_code;

    if (input == ft_nullptr || output_pointer == ft_nullptr
        || case_hook == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_FAILURE);
    }
    *output_pointer = ft_nullptr;
    code_point_count = 0;
    if (ft_utf8_count(input, &code_point_count) != FT_SUCCESS)
    {
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    allocation_size = (code_point_count * 4) + 1;
    allocated_buffer = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(allocation_size)));
    if (allocated_buffer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (FT_FAILURE);
    }
    input_length = ft_strlen_size_t(input);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        cma_free(allocated_buffer);
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    if (ft_utf8_transform(input, input_length, allocated_buffer,
            allocation_size, case_hook) != FT_SUCCESS)
    {
        int transform_error;

        transform_error = ft_global_error_stack_drop_last_error();
        if (transform_error == FT_ERR_SUCCESSS)
            transform_error = FT_ERR_INVALID_ARGUMENT;
        cma_free(allocated_buffer);
        *output_pointer = ft_nullptr;
        ft_global_error_stack_push(transform_error);
        return (FT_FAILURE);
    }
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        cma_free(allocated_buffer);
        *output_pointer = ft_nullptr;
        ft_global_error_stack_push(error_code);
        return (FT_FAILURE);
    }
    *output_pointer = allocated_buffer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}
