#include "time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <ctime>
#include <cstdio>
#include <sstream>
#include <iomanip>

static bool is_leap_year(int year)
{
    if (year % 4 != 0)
        return (false);
    if (year % 100 != 0)
        return (true);
    if (year % 400 != 0)
        return (false);
    return (true);
}

static int get_days_in_month(int year, int month)
{
    if (month == 1)
        return (31);
    if (month == 2)
    {
        if (is_leap_year(year))
            return (29);
        return (28);
    }
    if (month == 3)
        return (31);
    if (month == 4)
        return (30);
    if (month == 5)
        return (31);
    if (month == 6)
        return (30);
    if (month == 7)
        return (31);
    if (month == 8)
        return (31);
    if (month == 9)
        return (30);
    if (month == 10)
        return (31);
    if (month == 11)
        return (30);
    if (month == 12)
        return (31);
    return (0);
}

static bool parse_timezone_offset(const char *timezone_buffer, int *offset_seconds)
{
    const char  *offset_part;
    size_t      offset_length;
    int         offset_hours;
    int         offset_minutes;
    int         sign_multiplier;

    if (!timezone_buffer || !offset_seconds)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if ((timezone_buffer[0] == 'Z' || timezone_buffer[0] == 'z')
        && timezone_buffer[1] == '\0')
    {
        *offset_seconds = 0;
        return (true);
    }
    if (timezone_buffer[0] != '+' && timezone_buffer[0] != '-')
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    sign_multiplier = 1;
    if (timezone_buffer[0] == '-')
        sign_multiplier = -1;
    offset_part = timezone_buffer + 1;
    offset_length = ft_strlen(offset_part);
    if (offset_length == 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    offset_hours = 0;
    offset_minutes = 0;
    if (ft_strchr(offset_part, ':'))
    {
        if (std::sscanf(offset_part, "%d:%d", &offset_hours, &offset_minutes) != 2)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (false);
        }
    }
    else if (offset_length == 2)
    {
        if (std::sscanf(offset_part, "%d", &offset_hours) != 1)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (false);
        }
        offset_minutes = 0;
    }
    else if (offset_length == 4)
    {
        if (std::sscanf(offset_part, "%2d%2d", &offset_hours, &offset_minutes) != 2)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (false);
        }
    }
    else
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (offset_hours < 0 || offset_hours > 23)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (offset_minutes < 0 || offset_minutes > 59)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    *offset_seconds = sign_multiplier * ((offset_hours * 60) + offset_minutes) * 60;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    time_parse_iso8601(const char *string_input, std::tm *time_output, t_time *timestamp_output)
{
    std::tm parsed_time;
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    int seconds;
    char timezone_buffer[7];
    std::time_t epoch_time;
    int offset_seconds;
    std::time_t adjusted_epoch;
    std::tm *utc_time;
    int error_code;

    if (!string_input)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    ft_memset(&parsed_time, 0, sizeof(parsed_time));
    ft_memset(timezone_buffer, 0, sizeof(timezone_buffer));
    if (std::sscanf(string_input, "%d-%d-%dT%d:%d:%d%6s", &year, &month, &day, &hours, &minutes, &seconds, timezone_buffer) != 7)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (month < 1 || month > 12)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (day < 1 || day > get_days_in_month(year, month))
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (hours < 0 || hours > 23)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (minutes < 0 || minutes > 59)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (seconds < 0 || seconds > 60)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (!parse_timezone_offset(timezone_buffer, &offset_seconds))
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (false);
    }
    ft_global_error_stack_pop_newest();
    parsed_time.tm_year = year - 1900;
    parsed_time.tm_mon = month - 1;
    parsed_time.tm_mday = day;
    parsed_time.tm_hour = hours;
    parsed_time.tm_min = minutes;
    parsed_time.tm_sec = seconds;
    parsed_time.tm_isdst = 0;
    if (timestamp_output)
    {
        epoch_time = cmp_timegm(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (false);
        }
        adjusted_epoch = epoch_time - static_cast<std::time_t>(offset_seconds);
        *timestamp_output = static_cast<t_time>(adjusted_epoch);
    }
    else
    {
        epoch_time = cmp_timegm(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (false);
        }
        adjusted_epoch = epoch_time - static_cast<std::time_t>(offset_seconds);
    }
    if (time_output)
    {
        std::time_t epoch_copy;

        epoch_copy = adjusted_epoch;
        utc_time = std::gmtime(&epoch_copy);
        if (!utc_time)
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (false);
        }
        *time_output = *utc_time;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output, bool interpret_as_utc)
{
    std::tm parsed_time;
    std::istringstream input_stream;
    std::time_t epoch_time;

    if (!string_input || !format)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    ft_memset(&parsed_time, 0, sizeof(parsed_time));
    input_stream.str(string_input);
    input_stream >> std::get_time(&parsed_time, format);
    if (input_stream.fail())
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    parsed_time.tm_isdst = 0;
    if (interpret_as_utc)
    {
        epoch_time = cmp_timegm(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (false);
        }
    }
    else
    {
        epoch_time = std::mktime(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
        {
            ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
            return (false);
        }
    }
    if (timestamp_output)
        *timestamp_output = static_cast<t_time>(epoch_time);
    if (time_output)
    {
        if (interpret_as_utc)
        {
            std::time_t epoch_copy;
            std::tm *utc_time;

            epoch_copy = epoch_time;
            utc_time = std::gmtime(&epoch_copy);
            if (!utc_time)
            {
                ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                return (false);
            }
            *time_output = *utc_time;
        }
        else
            *time_output = parsed_time;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output)
{
    return (time_parse_custom(string_input, format, time_output, timestamp_output, false));
}
