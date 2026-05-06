#include "time.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

static ft_size_t   format_time_component(char *destination, ft_size_t destination_size, int32_t value, int32_t minimum_width)
{
    char                    reversed_digits[32];
    ft_size_t                  digit_count;
    ft_bool                    is_negative;
    ft_size_t                  required_length;
    ft_size_t                  index;
    ft_size_t                  copy_index;
    int64_t               signed_value;
    uint64_t              magnitude;

    if (destination_size == 0)
        return (0);
    digit_count = 0;
    is_negative = FT_FALSE;
    signed_value = value;
    magnitude = static_cast<uint64_t>(signed_value);
    if (signed_value < 0)
    {
        is_negative = FT_TRUE;
        magnitude = static_cast<uint64_t>(-signed_value);
    }
    if (magnitude == 0)
    {
        if (digit_count >= sizeof(reversed_digits))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (0);
        }
        reversed_digits[digit_count] = '0';
        digit_count++;
    }
    while (magnitude > 0)
    {
        if (digit_count >= sizeof(reversed_digits))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
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
            (void)(FT_ERR_OUT_OF_RANGE);
            return (0);
        }
        reversed_digits[digit_count] = '-';
        digit_count++;
    }
    required_length = digit_count;
    if (minimum_width > static_cast<int32_t>(required_length))
        required_length = static_cast<ft_size_t>(minimum_width);
    if (required_length >= destination_size)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
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
    (void)(FT_ERR_SUCCESS);
    return (required_length);
}

ft_size_t  time_strftime(char *buffer, ft_size_t size, const char *format, const t_time_info *time_info)
{
    ft_size_t  format_index;
    ft_size_t  output_index;
    char    number_buffer[16];
    ft_size_t  length;
    int32_t     value;
    ft_bool    lock_acquired;
    int32_t     lock_error;
    ft_bool    format_failed;
    ft_size_t  formatted_length;
    int32_t     error_code;

    if (!buffer || size == 0 || !format || !time_info)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    lock_acquired = FT_FALSE;
    lock_error = time_info_lock(time_info, &lock_acquired);
    error_code = FT_ERR_SUCCESS;
    if (lock_error != 0 || error_code != FT_ERR_SUCCESS)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_STATE;
        (void)(error_code);
        return (0);
    }
    format_index = 0;
    output_index = 0;
    format_failed = FT_FALSE;
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
                int32_t minimum_width;
                ft_size_t number_index;

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
                error_code = FT_ERR_SUCCESS;
                if (length == 0 && error_code != FT_ERR_SUCCESS)
                {
                    if (output_index < size)
                        buffer[output_index] = '\0';
                    else if (size > 0)
                        buffer[size - 1] = '\0';
                    format_failed = FT_TRUE;
                    break ;
                }
                if (error_code != FT_ERR_SUCCESS)
                {
                    if (output_index < size)
                        buffer[output_index] = '\0';
                    else if (size > 0)
                        buffer[size - 1] = '\0';
                    format_failed = FT_TRUE;
                    break ;
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
        error_code = FT_ERR_SUCCESS;
    }
    else
    {
        formatted_length = 0;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_OUT_OF_RANGE;
    }
    time_info_unlock(time_info, lock_acquired);
    error_code = FT_ERR_SUCCESS;
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)(error_code);
        return (0);
    }
    if (formatted_length == 0)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    (void)(FT_ERR_SUCCESS);
    return (formatted_length);
}
