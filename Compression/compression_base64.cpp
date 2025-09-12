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
    output_buffer = static_cast<unsigned char *>(cma_malloc(output_length));
    if (!output_buffer)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        byte_one = input_buffer[input_index];
        input_index++;
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
    *encoded_size = output_index;
    return (output_buffer);
}

unsigned char    *ft_base64_decode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decoded_size)
{
    unsigned char   *output_buffer;
    std::size_t     output_length;
    std::size_t     input_index;
    std::size_t     output_index;
    int             value_one;
    int             value_two;
    int             value_three;
    int             value_four;
    unsigned char   char_three;
    unsigned char   char_four;

    if (!input_buffer || !decoded_size)
        return (ft_nullptr);
    if (input_size % 4 != 0)
        return (ft_nullptr);
    output_length = (input_size / 4) * 3;
    if (input_buffer[input_size - 1] == '=')
    {
        output_length--;
        if (input_buffer[input_size - 2] == '=')
            output_length--;
    }
    output_buffer = static_cast<unsigned char *>(cma_malloc(output_length));
    if (!output_buffer)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        value_one = base64_char_value(input_buffer[input_index]);
        input_index++;
        value_two = base64_char_value(input_buffer[input_index]);
        input_index++;
        char_three = input_buffer[input_index];
        if (char_three != '=')
            value_three = base64_char_value(char_three);
        else
            value_three = 0;
        input_index++;
        char_four = input_buffer[input_index];
        if (char_four != '=')
            value_four = base64_char_value(char_four);
        else
            value_four = 0;
        input_index++;
        output_buffer[output_index] = static_cast<unsigned char>((value_one << 2) | (value_two >> 4));
        output_index++;
        if (char_three != '=')
        {
            output_buffer[output_index] = static_cast<unsigned char>(((value_two & 0x0F) << 4) | (value_three >> 2));
            output_index++;
        }
        if (char_four != '=')
        {
            output_buffer[output_index] = static_cast<unsigned char>(((value_three & 0x03) << 6) | value_four);
            output_index++;
        }
    }
    *decoded_size = output_index;
    return (output_buffer);
}
