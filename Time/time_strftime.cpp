#include "time.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

static size_t   format_time_component(char *destination, size_t destination_size, int value, int minimum_width)
{
    char                    reversed_digits[32];
    size_t                  digit_count;
    bool                    is_negative;
    size_t                  required_length;
    size_t                  index;
    size_t                  copy_index;
    long long               signed_value;
    unsigned long long      magnitude;

    if (destination_size == 0)
        return (0);
    digit_count = 0;
    is_negative = false;
    signed_value = value;
    magnitude = static_cast<unsigned long long>(signed_value);
    if (signed_value < 0)
    {
        is_negative = true;
        magnitude = static_cast<unsigned long long>(-signed_value);
    }
    if (magnitude == 0)
    {
        if (digit_count >= sizeof(reversed_digits))
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (0);
        }
        reversed_digits[digit_count] = '0';
        digit_count++;
    }
    while (magnitude > 0)
    {
        if (digit_count >= sizeof(reversed_digits))
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (0);
        }
        reversed_digits[digit_count] = static_cast<char>('0' + (magnitude % 10));
        magnitude /= 10;
        digit_count++;
    }
    if (is_negative)
    {
        if (digit_count >= sizeof(reversed_digits))
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (0);
        }
        reversed_digits[digit_count] = '-';
        digit_count++;
    }
    required_length = digit_count;
    if (minimum_width > static_cast<int>(required_length))
        required_length = static_cast<size_t>(minimum_width);
    if (required_length >= destination_size)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    destination[required_length] = '\0';
    index = required_length;
    copy_index = 0;
    while (copy_index < digit_count)
    {
        index--;
        destination[index] = reversed_digits[copy_index];
        copy_index++;
    }
    while (index > 0)
    {
        index--;
        destination[index] = '0';
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (required_length);
}

size_t  time_strftime(char *buffer, size_t size, const char *format, const t_time_info *time_info)
{
    size_t  format_index;
    size_t  output_index;
    char    number_buffer[16];
    size_t  length;
    int     value;
    bool    lock_acquired;
    int     lock_error;
    bool    format_failed;
    size_t  formatted_length;
    int     error_code;

    if (!buffer || size == 0 || !format || !time_info)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    lock_acquired = false;
    lock_error = time_info_lock(time_info, &lock_acquired);
    error_code = ft_global_error_stack_drop_last_error();
    if (lock_error != 0 || error_code != FT_ERR_SUCCESSS)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    format_index = 0;
    output_index = 0;
    format_failed = false;
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
                int minimum_width;
                size_t number_index;

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
                if (format[format_index + 1] == 'Y')
                    minimum_width = 4;
                else
                    minimum_width = 2;
                length = format_time_component(number_buffer, sizeof(number_buffer), value, minimum_width);
                error_code = ft_global_error_stack_drop_last_error();
                if (length == 0 && error_code != FT_ERR_SUCCESSS)
                {
                    if (output_index < size)
                        buffer[output_index] = '\0';
                    else if (size > 0)
                        buffer[size - 1] = '\0';
                    format_failed = true;
                    break;
                }
                if (error_code != FT_ERR_SUCCESSS)
                {
                    if (output_index < size)
                        buffer[output_index] = '\0';
                    else if (size > 0)
                        buffer[size - 1] = '\0';
                    format_failed = true;
                    break;
                }
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
    if (!format_failed)
    {
        buffer[output_index] = '\0';
        formatted_length = output_index;
        error_code = FT_ERR_SUCCESSS;
    }
    else
    {
        formatted_length = 0;
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_OUT_OF_RANGE;
    }
    time_info_unlock(time_info, lock_acquired);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (formatted_length == 0)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (formatted_length);
}
