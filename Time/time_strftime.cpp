#include "time.hpp"
#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"
#include "../Errno/errno.hpp"

size_t  time_strftime(char *buffer, size_t size, const char *format, const t_time_info *time_info)
{
    size_t  format_index;
    size_t  output_index;
    char    number_buffer[5];
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
                int snprintf_result;

                if (format[format_index + 1] == 'Y')
                    snprintf_result = pf_snprintf(number_buffer, sizeof(number_buffer), "%04d", value);
                else
                    snprintf_result = pf_snprintf(number_buffer, sizeof(number_buffer), "%02d", value);
                if (snprintf_result < 0 || ft_errno != ER_SUCCESS)
                {
                    if (output_index < size)
                        buffer[output_index] = '\0';
                    else if (size > 0)
                        buffer[size - 1] = '\0';
                    return (0);
                }
                length = ft_strlen(number_buffer);
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

