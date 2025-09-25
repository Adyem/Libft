#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "compression.hpp"

static int base64_char_value(unsigned char character)
{
    if (character >= 'A' && character <= 'Z')
        return (character - 'A');
    if (character >= 'a' && character <= 'z')
        return (character - 'a' + 26);
    if (character >= '0' && character <= '9')
        return (character - '0' + 52);
    if (character == '+')
        return (62);
    if (character == '/')
        return (63);
    return (-1);
}

static int  is_base64_whitespace(unsigned char character)
{
    if (character == ' ')
        return (1);
    if (character == '\n')
        return (1);
    if (character == '\r')
        return (1);
    if (character == '\t')
        return (1);
    if (character == '\f')
        return (1);
    if (character == '\v')
        return (1);
    return (0);
}

unsigned char    *ft_base64_encode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *encoded_size)
{
    const char      *base64_table;
    unsigned char   *output_buffer;
    std::size_t     output_length;
    std::size_t     input_index;
    std::size_t     output_index;
    unsigned char   byte_one;
    unsigned char   byte_two;
    unsigned char   byte_three;
    int             has_byte_two;
    int             has_byte_three;

    if (!input_buffer || !encoded_size)
        return (ft_nullptr);
    base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    output_length = ((input_size + 2) / 3) * 4;
    output_buffer = static_cast<unsigned char *>(cma_malloc(output_length + 1));
    if (!output_buffer)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        byte_one = input_buffer[input_index];
        input_index++;
        byte_two = 0;
        byte_three = 0;
        has_byte_two = 0;
        has_byte_three = 0;
        if (input_index < input_size)
        {
            byte_two = input_buffer[input_index];
            input_index++;
            has_byte_two = 1;
        }
        if (input_index < input_size)
        {
            byte_three = input_buffer[input_index];
            input_index++;
            has_byte_three = 1;
        }
        output_buffer[output_index] = static_cast<unsigned char>(base64_table[byte_one >> 2]);
        output_index++;
        output_buffer[output_index] = static_cast<unsigned char>(base64_table[((byte_one & 0x03) << 4) | (byte_two >> 4)]);
        output_index++;
        if (has_byte_two)
        {
            output_buffer[output_index] = static_cast<unsigned char>(base64_table[((byte_two & 0x0F) << 2) | (byte_three >> 6)]);
            output_index++;
        }
        else
        {
            output_buffer[output_index] = '=';
            output_index++;
        }
        if (has_byte_three)
        {
            output_buffer[output_index] = static_cast<unsigned char>(base64_table[byte_three & 0x3F]);
            output_index++;
        }
        else
        {
            output_buffer[output_index] = '=';
            output_index++;
        }
    }
    output_buffer[output_index] = '\0';
    *encoded_size = output_index;
    return (output_buffer);
}

unsigned char    *ft_base64_decode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decoded_size)
{
    unsigned char   *output_buffer;
    std::size_t     output_length;
    std::size_t     sanitized_length;
    std::size_t     input_index;
    std::size_t     sanitized_index;
    std::size_t     output_index;
    int             value_one;
    int             value_two;
    int             value_three;
    int             value_four;
    unsigned char   char_three;
    unsigned char   char_four;
    unsigned char   current_character;
    unsigned char   chunk[4];
    int             chunk_count;
    int             has_char_three;
    int             has_char_four;

    if (!input_buffer || !decoded_size)
        return (ft_nullptr);
    *decoded_size = 0;
    sanitized_length = 0;
    input_index = 0;
    while (input_index < input_size)
    {
        if (!is_base64_whitespace(input_buffer[input_index]))
            sanitized_length++;
        input_index++;
    }
    if (sanitized_length == 0)
    {
        output_buffer = static_cast<unsigned char *>(cma_malloc(1));
        if (!output_buffer)
            return (ft_nullptr);
        return (output_buffer);
    }
    if (sanitized_length % 4 == 1)
        return (ft_nullptr);
    output_length = ((sanitized_length + 3) / 4) * 3;
    output_buffer = static_cast<unsigned char *>(cma_malloc(output_length));
    if (!output_buffer)
        return (ft_nullptr);
    input_index = 0;
    sanitized_index = 0;
    output_index = 0;
    while (sanitized_index < sanitized_length)
    {
        chunk_count = 0;
        while (chunk_count < 4 && input_index < input_size)
        {
            current_character = input_buffer[input_index];
            input_index++;
            if (is_base64_whitespace(current_character))
                continue;
            chunk[chunk_count] = current_character;
            chunk_count++;
        }
        if (chunk_count == 0)
            break ;
        sanitized_index += chunk_count;
        if (chunk_count < 2)
        {
            cma_free(output_buffer);
            return (ft_nullptr);
        }
        value_one = base64_char_value(chunk[0]);
        if (value_one == -1)
        {
            cma_free(output_buffer);
            return (ft_nullptr);
        }
        value_two = base64_char_value(chunk[1]);
        if (value_two == -1)
        {
            cma_free(output_buffer);
            return (ft_nullptr);
        }
        has_char_three = 0;
        value_three = 0;
        if (chunk_count > 2)
        {
            char_three = chunk[2];
            has_char_three = 1;
        }
        else
        {
            char_three = '=';
        }
        has_char_four = 0;
        value_four = 0;
        if (chunk_count > 3)
        {
            char_four = chunk[3];
            has_char_four = 1;
        }
        else
        {
            char_four = '=';
        }
        if (has_char_three && char_three != '=')
        {
            value_three = base64_char_value(char_three);
            if (value_three == -1)
            {
                cma_free(output_buffer);
                return (ft_nullptr);
            }
        }
        else
            value_three = 0;
        if (has_char_four && char_four != '=')
        {
            value_four = base64_char_value(char_four);
            if (value_four == -1)
            {
                cma_free(output_buffer);
                return (ft_nullptr);
            }
        }
        if (has_char_three && char_three == '=' && has_char_four && char_four != '=')
        {
            cma_free(output_buffer);
            return (ft_nullptr);
        }
        if ((has_char_three && char_three == '=') || (has_char_four && char_four == '='))
        {
            if (sanitized_index < sanitized_length)
            {
                cma_free(output_buffer);
                return (ft_nullptr);
            }
        }
        output_buffer[output_index] = static_cast<unsigned char>((value_one << 2) | (value_two >> 4));
        output_index++;
        if (has_char_three && char_three != '=')
        {
            output_buffer[output_index] = static_cast<unsigned char>(((value_two & 0x0F) << 4) | (value_three >> 2));
            output_index++;
        }
        if (has_char_four && char_four != '=')
        {
            output_buffer[output_index] = static_cast<unsigned char>(((value_three & 0x03) << 6) | value_four);
            output_index++;
        }
    }
    *decoded_size = output_index;
    return (output_buffer);
}
