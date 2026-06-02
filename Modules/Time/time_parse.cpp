#include "time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include <ctime>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <limits>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static ft_bool is_leap_year(int32_t year)
{
    if (year % 4 != 0)
        return (FT_FALSE);
    if (year % 100 != 0)
        return (FT_TRUE);
    if (year % 400 != 0)
        return (FT_FALSE);
    return (FT_TRUE);
}

static int32_t get_days_in_month(int32_t year, int32_t month)
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

static ft_bool parse_timezone_offset(const char *timezone_buffer, int32_t *offset_seconds)
{
    const char  *offset_part;
    ft_size_t      offset_length;
    int32_t         offset_hours;
    int32_t         offset_minutes;
    int32_t         sign_multiplier;

    if (!timezone_buffer || !offset_seconds)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if ((timezone_buffer[0] == 'Z' || timezone_buffer[0] == 'z')
        && timezone_buffer[1] == '\0')
    {
        *offset_seconds = 0;
        return (FT_TRUE);
    }
    if (timezone_buffer[0] != '+' && timezone_buffer[0] != '-')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    sign_multiplier = 1;
    if (timezone_buffer[0] == '-')
        sign_multiplier = -1;
    offset_part = timezone_buffer + 1;
    offset_length = ft_strlen(offset_part);
    if (offset_length == 0)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    offset_hours = 0;
    offset_minutes = 0;
    if (ft_strchr(offset_part, ':'))
    {
        if (std::sscanf(offset_part, "%d:%d", &offset_hours, &offset_minutes) != 2)
        {
            (void)(FT_ERR_INVALID_ARGUMENT);
            return (FT_FALSE);
        }
    }
    else if (offset_length == 2)
    {
        if (std::sscanf(offset_part, "%d", &offset_hours) != 1)
        {
            (void)(FT_ERR_INVALID_ARGUMENT);
            return (FT_FALSE);
        }
        offset_minutes = 0;
    }
    else if (offset_length == 4)
    {
        if (std::sscanf(offset_part, "%2d%2d", &offset_hours, &offset_minutes) != 2)
        {
            (void)(FT_ERR_INVALID_ARGUMENT);
            return (FT_FALSE);
        }
    }
    else
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (offset_hours < 0 || offset_hours > 23)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (offset_minutes < 0 || offset_minutes > 59)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    *offset_seconds = sign_multiplier * ((offset_hours * 60) + offset_minutes) * 60;
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_parse_iso8601(const char *string_input, std::tm *time_output, t_time *timestamp_output)
{
    std::tm parsed_time;
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hours;
    int32_t minutes;
    int32_t seconds;
    char timezone_buffer[7];
    std::time_t epoch_time;
    int32_t offset_seconds;
    std::time_t adjusted_epoch;
    std::tm *utc_time;
    int32_t error_code;

    if (!string_input)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    ft_memset(&parsed_time, 0, sizeof(parsed_time));
    ft_memset(timezone_buffer, 0, sizeof(timezone_buffer));
    if (std::sscanf(string_input, "%d-%d-%dT%d:%d:%d%6s", &year, &month, &day, &hours, &minutes, &seconds, timezone_buffer) != 7)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (month < 1 || month > 12)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (day < 1 || day > get_days_in_month(year, month))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (hours < 0 || hours > 23)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (minutes < 0 || minutes > 59)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (seconds < 0 || seconds > 60)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (!parse_timezone_offset(timezone_buffer, &offset_seconds))
    {
        error_code = FT_ERR_SUCCESS;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        (void)(error_code);
        return (FT_FALSE);
    }
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
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        adjusted_epoch = epoch_time - static_cast<std::time_t>(offset_seconds);
        *timestamp_output = adjusted_epoch;
    }
    else
    {
        epoch_time = cmp_timegm(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
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
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        *time_output = *utc_time;
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

static ft_bool time_parse_rfc3339_timezone_buffer(const char *timezone_buffer)
{
    if (!timezone_buffer)
        return (FT_FALSE);
    if (timezone_buffer[0] == 'Z' && timezone_buffer[1] == '\0')
        return (FT_TRUE);
    if (timezone_buffer[0] != '+' && timezone_buffer[0] != '-')
        return (FT_FALSE);
    if (timezone_buffer[1] < '0' || timezone_buffer[1] > '9')
        return (FT_FALSE);
    if (timezone_buffer[2] < '0' || timezone_buffer[2] > '9')
        return (FT_FALSE);
    if (timezone_buffer[3] != ':')
        return (FT_FALSE);
    if (timezone_buffer[4] < '0' || timezone_buffer[4] > '9')
        return (FT_FALSE);
    if (timezone_buffer[5] < '0' || timezone_buffer[5] > '9')
        return (FT_FALSE);
    if (timezone_buffer[6] != '\0')
        return (FT_FALSE);
    return (FT_TRUE);
}

ft_bool    time_parse_rfc3339(const char *string_input, std::tm *time_output, t_time *timestamp_output)
{
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hours;
    int32_t minutes;
    int32_t seconds;
    char timezone_buffer[7];
    int32_t parsed_items;

    if (!string_input)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    ft_memset(timezone_buffer, 0, sizeof(timezone_buffer));
    parsed_items = std::sscanf(string_input, "%d-%d-%dT%d:%d:%d%6s",
            &year, &month, &day, &hours, &minutes, &seconds, timezone_buffer);
    if (parsed_items != 7)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (!time_parse_rfc3339_timezone_buffer(timezone_buffer))
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (!time_parse_iso8601(string_input, time_output, timestamp_output))
        return (FT_FALSE);
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_parse_timezone_offset(const char *string_input, int32_t *offset_minutes)
{
    int32_t sign_multiplier;
    int32_t offset_hours;
    int32_t offset_minutes_part;

    if (!string_input || !offset_minutes)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if ((string_input[0] == 'Z' || string_input[0] == 'z') && string_input[1] == '\0')
    {
        *offset_minutes = 0;
        (void)(FT_ERR_SUCCESS);
        return (FT_TRUE);
    }
    if (string_input[0] != '+' && string_input[0] != '-')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (string_input[1] < '0' || string_input[1] > '9')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (string_input[2] < '0' || string_input[2] > '9')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (string_input[3] != ':')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (string_input[4] < '0' || string_input[4] > '9')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (string_input[5] < '0' || string_input[5] > '9')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (string_input[6] != '\0')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    sign_multiplier = 1;
    if (string_input[0] == '-')
        sign_multiplier = -1;
    offset_hours = ((string_input[1] - '0') * 10) + (string_input[2] - '0');
    offset_minutes_part = ((string_input[4] - '0') * 10) + (string_input[5] - '0');
    if (offset_hours < 0 || offset_hours > 23)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (offset_minutes_part < 0 || offset_minutes_part > 59)
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    *offset_minutes = sign_multiplier * ((offset_hours * 60) + offset_minutes_part);
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

static ft_bool time_parse_duration_add_magnitude(uint64_t current_total, uint64_t component, uint64_t *next_total)
{
    uint64_t limit_magnitude;

    if (!next_total)
        return (FT_FALSE);
    limit_magnitude = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    limit_magnitude += UINT64_C(1);
    if (component > limit_magnitude)
        return (FT_FALSE);
    if (current_total > limit_magnitude - component)
        return (FT_FALSE);
    *next_total = current_total + component;
    return (FT_TRUE);
}

static ft_bool time_parse_duration_read_number(const char *string_input, ft_size_t *index, uint64_t *value_output)
{
    uint64_t value;
    ft_bool saw_digit;
    int32_t digit;

    if (!string_input || !index || !value_output)
        return (FT_FALSE);
    value = 0;
    saw_digit = FT_FALSE;
    while (string_input[*index] >= '0' && string_input[*index] <= '9')
    {
        digit = string_input[*index] - '0';
        if (value > (std::numeric_limits<uint64_t>::max() - static_cast<uint64_t>(digit)) / UINT64_C(10))
            return (FT_FALSE);
        value = (value * UINT64_C(10)) + static_cast<uint64_t>(digit);
        *index += 1;
        saw_digit = FT_TRUE;
    }
    if (!saw_digit)
        return (FT_FALSE);
    *value_output = value;
    return (FT_TRUE);
}

static ft_bool time_parse_duration_read_fraction(const char *string_input, ft_size_t *index, uint64_t *fraction_milliseconds)
{
    uint64_t fraction_value;
    uint64_t fraction_scale;
    ft_bool saw_digit;
    int32_t digit;

    if (!string_input || !index || !fraction_milliseconds)
        return (FT_FALSE);
    if (string_input[*index] != '.')
    {
        *fraction_milliseconds = 0;
        return (FT_TRUE);
    }
    *index += 1;
    fraction_value = 0;
    fraction_scale = UINT64_C(100);
    saw_digit = FT_FALSE;
    while (string_input[*index] >= '0' && string_input[*index] <= '9')
    {
        digit = string_input[*index] - '0';
        if (saw_digit && fraction_scale > 0)
        {
            fraction_value += static_cast<uint64_t>(digit) * fraction_scale;
            fraction_scale /= UINT64_C(10);
        }
        else if (!saw_digit)
        {
            fraction_value += static_cast<uint64_t>(digit) * UINT64_C(100);
            fraction_scale = UINT64_C(10);
        }
        *index += 1;
        saw_digit = FT_TRUE;
    }
    if (!saw_digit)
        return (FT_FALSE);
    *fraction_milliseconds = fraction_value;
    return (FT_TRUE);
}

ft_bool    time_parse_duration(const char *string_input, t_duration_milliseconds *duration_output)
{
    ft_size_t index;
    ft_bool is_negative;
    ft_bool in_time_section;
    ft_bool saw_component;
    ft_bool saw_weeks;
    ft_bool saw_days;
    ft_bool saw_hours;
    ft_bool saw_minutes;
    ft_bool saw_seconds;
    uint64_t total_magnitude;
    uint64_t component_magnitude;
    uint64_t whole_part;
    uint64_t fraction_part;
    uint64_t component_total;
    t_duration_milliseconds duration_value;
    uint64_t limit_magnitude;
    char designator;
    ft_bool had_fraction;

    if (!string_input || !duration_output)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    index = 0;
    is_negative = FT_FALSE;
    in_time_section = FT_FALSE;
    saw_component = FT_FALSE;
    saw_weeks = FT_FALSE;
    saw_days = FT_FALSE;
    saw_hours = FT_FALSE;
    saw_minutes = FT_FALSE;
    saw_seconds = FT_FALSE;
    total_magnitude = 0;
    limit_magnitude = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    limit_magnitude += UINT64_C(1);
    had_fraction = FT_FALSE;
    if (string_input[index] == '+' || string_input[index] == '-')
    {
        if (string_input[index] == '-')
            is_negative = FT_TRUE;
        index += 1;
    }
    if (string_input[index] != 'P' && string_input[index] != 'p')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    index += 1;
    if (string_input[index] == '\0')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    while (string_input[index] != '\0')
    {
        if (string_input[index] == 'T' || string_input[index] == 't')
        {
            if (in_time_section || saw_weeks)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            in_time_section = FT_TRUE;
            index += 1;
            if (string_input[index] == '\0')
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            continue;
        }
        if (!time_parse_duration_read_number(string_input, &index, &whole_part))
        {
            (void)(FT_ERR_INVALID_ARGUMENT);
            return (FT_FALSE);
        }
        fraction_part = 0;
        if (string_input[index] == '.')
        {
            if (!in_time_section)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            if (!time_parse_duration_read_fraction(string_input, &index, &fraction_part))
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            had_fraction = FT_TRUE;
        }
        designator = string_input[index];
        if (designator == '\0')
        {
            (void)(FT_ERR_INVALID_ARGUMENT);
            return (FT_FALSE);
        }
        if (had_fraction && !(designator == 'S' || designator == 's'))
        {
            (void)(FT_ERR_INVALID_ARGUMENT);
            return (FT_FALSE);
        }
        if (designator == 'W' || designator == 'w')
        {
            if (saw_component || in_time_section || saw_days || saw_hours || saw_minutes || saw_seconds)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            if (whole_part > limit_magnitude / (UINT64_C(7) * UINT64_C(24) * UINT64_C(60) * UINT64_C(60) * UINT64_C(1000)))
            {
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            component_magnitude = whole_part * (UINT64_C(7) * UINT64_C(24) * UINT64_C(60) * UINT64_C(60) * UINT64_C(1000));
            saw_weeks = FT_TRUE;
        }
        else if (designator == 'D' || designator == 'd')
        {
            if (in_time_section || saw_weeks || saw_days)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            if (whole_part > limit_magnitude / (UINT64_C(24) * UINT64_C(60) * UINT64_C(60) * UINT64_C(1000)))
            {
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            component_magnitude = whole_part * (UINT64_C(24) * UINT64_C(60) * UINT64_C(60) * UINT64_C(1000));
            saw_days = FT_TRUE;
        }
        else if (designator == 'H' || designator == 'h')
        {
            if (!in_time_section || saw_weeks || saw_hours)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            if (whole_part > limit_magnitude / (UINT64_C(60) * UINT64_C(60) * UINT64_C(1000)))
            {
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            component_magnitude = whole_part * (UINT64_C(60) * UINT64_C(60) * UINT64_C(1000));
            saw_hours = FT_TRUE;
        }
        else if (designator == 'M' || designator == 'm')
        {
            if (!in_time_section)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            if (saw_weeks || saw_minutes)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            if (whole_part > limit_magnitude / (UINT64_C(60) * UINT64_C(1000)))
            {
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            component_magnitude = whole_part * (UINT64_C(60) * UINT64_C(1000));
            saw_minutes = FT_TRUE;
        }
        else if (designator == 'S' || designator == 's')
        {
            if (!in_time_section || saw_weeks || saw_seconds)
            {
                (void)(FT_ERR_INVALID_ARGUMENT);
                return (FT_FALSE);
            }
            if (whole_part > limit_magnitude / UINT64_C(1000))
            {
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            component_magnitude = whole_part * UINT64_C(1000);
            if (fraction_part > UINT64_C(999))
            {
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            if (component_magnitude > limit_magnitude - fraction_part)
            {
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            component_magnitude += fraction_part;
            saw_seconds = FT_TRUE;
        }
        else
        {
            (void)(FT_ERR_INVALID_ARGUMENT);
            return (FT_FALSE);
        }
        if (!time_parse_duration_add_magnitude(total_magnitude, component_magnitude, &component_total))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        total_magnitude = component_total;
        saw_component = FT_TRUE;
        index += 1;
    }
    if (!saw_component)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    if (!is_negative)
    {
        if (total_magnitude > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        duration_value.milliseconds = static_cast<int64_t>(total_magnitude);
    }
    else
    {
        if (total_magnitude > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + UINT64_C(1))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        if (total_magnitude == static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + UINT64_C(1))
            duration_value.milliseconds = std::numeric_limits<int64_t>::min();
        else
            duration_value.milliseconds = -static_cast<int64_t>(total_magnitude);
    }
    duration_value.mutex = ft_nullptr;
    duration_value.thread_safe_enabled = FT_FALSE;
    *duration_output = duration_value;
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output, ft_bool interpret_as_utc)
{
    std::tm parsed_time;
    std::istringstream input_stream;
    std::time_t epoch_time;

    if (!string_input || !format)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    ft_memset(&parsed_time, 0, sizeof(parsed_time));
    input_stream.str(string_input);
    input_stream >> std::get_time(&parsed_time, format);
    if (input_stream.fail())
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    parsed_time.tm_isdst = 0;
    if (interpret_as_utc)
    {
        epoch_time = cmp_timegm(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
    }
    else
    {
        epoch_time = std::mktime(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
    }
    if (timestamp_output)
        *timestamp_output = epoch_time;
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
                (void)(FT_ERR_OUT_OF_RANGE);
                return (FT_FALSE);
            }
            *time_output = *utc_time;
        }
        else
            *time_output = parsed_time;
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output)
{
    return (time_parse_custom(string_input, format, time_output, timestamp_output, FT_FALSE));
}
