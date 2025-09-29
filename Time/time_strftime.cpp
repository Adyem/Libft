#include "time.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

static size_t   format_time_component(char *destination, size_t destination_size, int value, int minimum_width)
{
    char        reversed_digits[32];
    size_t      digit_count;
    bool        is_negative;
    size_t      required_length;
    size_t      pad_length;
    size_t      index;
    size_t      copy_index;

    if (destination_size == 0)
        return (0);
    digit_count = 0;
    is_negative = false;
    if (value < 0)
    {
        is_negative = true;
        value = -value;
    }
    if (value == 0)
    {
        reversed_digits[digit_count] = '0';
        digit_count++;
    }
    while (value > 0)
    {
        reversed_digits[digit_count] = static_cast<char>('0' + (value % 10));
        value /= 10;
        digit_count++;
    }
    if (is_negative)
    {
        reversed_digits[digit_count] = '-';
        digit_count++;
    }
    required_length = digit_count;
    if (minimum_width > static_cast<int>(required_length))
        required_length = static_cast<size_t>(minimum_width);
    if (required_length + 1 > destination_size)
    {
        ft_errno = FT_ERANGE;
        return (0);
    }
    pad_length = 0;
    if (required_length > digit_count)
        pad_length = required_length - digit_count;
    index = 0;
    while (index < pad_length)
    {
        destination[index] = '0';
        index++;
    }
    copy_index = 0;
    while (copy_index < digit_count)
    {
        destination[pad_length + copy_index] = reversed_digits[digit_count - copy_index - 1];
        copy_index++;
    }
    destination[required_length] = '\0';
    ft_errno = ER_SUCCESS;
    return (required_length);
}

size_t  time_strftime(char *buffer, size_t size, const char *format, const t_time_info *time_info)
{
    size_t  format_index;
    size_t  output_index;
    char    number_buffer[16];
    size_t  length;
    int     value;

    if (!buffer || size == 0 || !format || !time_info)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    format_index = 0;
    output_index = 0;
    while (format[format_index] && output_index + 1 < size)
    {
        if (format[format_index] == '%' && format[format_index + 1])
        {
            if (format[format_index + 1] == '%')
            {
                buffer[output_index] = '%';
                output_index++;
                format_index += 2;
            }
            else
            {
                if (format[format_index + 1] == 'Y')
                    value = time_info->year + 1900;
                else if (format[format_index + 1] == 'm')
                    value = time_info->month + 1;
                else if (format[format_index + 1] == 'd')
                    value = time_info->month_day;
                else if (format[format_index + 1] == 'H')
                    value = time_info->hours;
                else if (format[format_index + 1] == 'M')
                    value = time_info->minutes;
                else if (format[format_index + 1] == 'S')
                    value = time_info->seconds;
                else
                {
                    format_index++;
                    continue;
                }
                int minimum_width;

                if (format[format_index + 1] == 'Y')
                    minimum_width = 4;
                else
                    minimum_width = 2;
                length = format_time_component(number_buffer, sizeof(number_buffer), value, minimum_width);
                if (length == 0 && ft_errno != ER_SUCCESS)
                {
                    if (output_index < size)
                        buffer[output_index] = '\0';
                    else if (size > 0)
                        buffer[size - 1] = '\0';
                    return (0);
                }
                size_t number_index;

                number_index = 0;
                while (number_index < length && output_index + 1 < size)
                {
                    buffer[output_index] = number_buffer[number_index];
                    output_index++;
                    number_index++;
                }
                format_index += 2;
            }
        }
        else
        {
            buffer[output_index] = format[format_index];
            output_index++;
            format_index++;
        }
    }
    buffer[output_index] = '\0';
    ft_errno = ER_SUCCESS;
    return (output_index);
}

