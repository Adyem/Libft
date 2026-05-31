#include "time.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include <limits>

static void time_interval_delete_string(ft_string *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)string->destroy();
    delete string;
    return ;
}

static ft_bool time_interval_is_duration(const char *string_input)
{
    ft_size_t index;

    if (!string_input)
        return (FT_FALSE);
    index = 0;
    if (string_input[index] == '+' || string_input[index] == '-')
        index += 1;
    if (string_input[index] != 'P' && string_input[index] != 'p')
        return (FT_FALSE);
    return (FT_TRUE);
}

static ft_bool time_interval_duration_has_whole_seconds(t_duration_milliseconds duration_value)
{
    if (duration_value.milliseconds % 1000 != 0)
        return (FT_FALSE);
    return (FT_TRUE);
}

static ft_bool time_interval_add_seconds(t_time base_time, int64_t seconds, t_time *out_time)
{
    int64_t result_seconds;

    if (!out_time)
        return (FT_FALSE);
    if (seconds > 0 && base_time > std::numeric_limits<t_time>::max() - seconds)
        return (FT_FALSE);
    if (seconds < 0)
    {
        if (seconds == std::numeric_limits<int64_t>::min())
        {
            if (base_time < static_cast<t_time>(0))
                return (FT_FALSE);
        }
        else if (base_time < std::numeric_limits<t_time>::min() - seconds)
            return (FT_FALSE);
    }
    result_seconds = base_time + seconds;
    *out_time = result_seconds;
    return (FT_TRUE);
}

static ft_bool time_interval_append_text(ft_string *destination, const char *text)
{
    if (!destination || !text)
        return (FT_FALSE);
    if (destination->append(text) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (FT_TRUE);
}

static char *time_interval_copy_segment(const char *string_input, ft_size_t segment_length)
{
    char *segment_buffer;
    ft_size_t segment_index;

    segment_buffer = new (std::nothrow) char[segment_length + 1];
    if (segment_buffer == ft_nullptr)
        return (ft_nullptr);
    segment_index = 0;
    while (segment_index < segment_length)
    {
        segment_buffer[segment_index] = string_input[segment_index];
        segment_index += 1;
    }
    segment_buffer[segment_length] = '\0';
    return (segment_buffer);
}

ft_string    *time_format_interval(t_time start_time, t_time end_time)
{
    ft_string *start_string;
    ft_string *end_string;
    ft_string *formatted;

    start_string = time_format_rfc3339(start_time);
    if (start_string == ft_nullptr)
        return (ft_nullptr);
    end_string = time_format_rfc3339(end_time);
    if (end_string == ft_nullptr)
    {
        time_interval_delete_string(start_string);
        return (ft_nullptr);
    }
    formatted = new (std::nothrow) ft_string();
    if (formatted == ft_nullptr)
    {
        time_interval_delete_string(start_string);
        time_interval_delete_string(end_string);
        return (ft_nullptr);
    }
    if (formatted->initialize() != FT_ERR_SUCCESS)
    {
        time_interval_delete_string(start_string);
        time_interval_delete_string(end_string);
        delete formatted;
        return (ft_nullptr);
    }
    if (!time_interval_append_text(formatted, start_string->c_str())
        || !time_interval_append_text(formatted, "/")
        || !time_interval_append_text(formatted, end_string->c_str()))
    {
        time_interval_delete_string(start_string);
        time_interval_delete_string(end_string);
        time_interval_delete_string(formatted);
        return (ft_nullptr);
    }
    time_interval_delete_string(start_string);
    time_interval_delete_string(end_string);
    return (formatted);
}

ft_bool    time_parse_interval(const char *string_input, t_time *start_time, t_time *end_time, t_duration_milliseconds *duration_output)
{
    const char *separator;
    const char *left_text;
    const char *right_text;
    ft_size_t left_text_length;
    t_duration_milliseconds interval_duration;
    t_time left_timestamp;
    t_time right_timestamp;
    t_time computed_start;
    t_time computed_end;
    std::tm parsed_time;
    ft_bool left_is_duration;
    ft_bool right_is_duration;
    char *left_text_copy;

    if (!string_input || !start_time || !end_time)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    separator = ft_strchr(string_input, '/');
    if (!separator)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    left_text = string_input;
    right_text = separator + 1;
    if (*left_text == '\0' || *right_text == '\0')
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    left_is_duration = time_interval_is_duration(left_text);
    right_is_duration = time_interval_is_duration(right_text);
    if (left_is_duration && right_is_duration)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    left_text_length = static_cast<ft_size_t>(separator - left_text);
    left_text_copy = time_interval_copy_segment(left_text, left_text_length);
    if (left_text_copy == ft_nullptr)
    {
        (void)(FT_ERR_NO_MEMORY);
        return (FT_FALSE);
    }
    if (left_is_duration)
    {
        if (!time_parse_duration(left_text_copy, &interval_duration))
        {
            delete [] left_text_copy;
            return (FT_FALSE);
        }
        if (!time_interval_duration_has_whole_seconds(interval_duration))
        {
            delete [] left_text_copy;
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        if (!time_parse_rfc3339(right_text, &parsed_time, &right_timestamp))
        {
            delete [] left_text_copy;
            return (FT_FALSE);
        }
        if (!time_interval_add_seconds(right_timestamp,
                -(interval_duration.milliseconds / 1000),
                &computed_start))
        {
            delete [] left_text_copy;
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        computed_end = right_timestamp;
        *start_time = computed_start;
        *end_time = computed_end;
        if (duration_output)
            *duration_output = interval_duration;
        delete [] left_text_copy;
        (void)(FT_ERR_SUCCESS);
        return (FT_TRUE);
    }
    if (right_is_duration)
    {
        if (!time_parse_rfc3339(left_text_copy, &parsed_time, &left_timestamp))
        {
            delete [] left_text_copy;
            return (FT_FALSE);
        }
        if (!time_parse_duration(right_text, &interval_duration))
        {
            delete [] left_text_copy;
            return (FT_FALSE);
        }
        if (!time_interval_duration_has_whole_seconds(interval_duration))
        {
            delete [] left_text_copy;
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        if (!time_interval_add_seconds(left_timestamp,
                interval_duration.milliseconds / 1000,
                &computed_end))
        {
            delete [] left_text_copy;
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        computed_start = left_timestamp;
        *start_time = computed_start;
        *end_time = computed_end;
        if (duration_output)
            *duration_output = interval_duration;
        delete [] left_text_copy;
        (void)(FT_ERR_SUCCESS);
        return (FT_TRUE);
    }
    if (!time_parse_rfc3339(left_text_copy, &parsed_time, &left_timestamp))
    {
        delete [] left_text_copy;
        return (FT_FALSE);
    }
    if (!time_parse_rfc3339(right_text, &parsed_time, &right_timestamp))
    {
        delete [] left_text_copy;
        return (FT_FALSE);
    }
    computed_start = left_timestamp;
    computed_end = right_timestamp;
    *start_time = computed_start;
    *end_time = computed_end;
    if (duration_output)
    {
        int64_t difference_seconds;
        int64_t difference_milliseconds;

        difference_seconds = computed_end - computed_start;
        if (difference_seconds > 0
            && difference_seconds > (std::numeric_limits<int64_t>::max() / 1000))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        if (difference_seconds < 0
            && difference_seconds < (std::numeric_limits<int64_t>::min() / 1000))
        {
            (void)(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        difference_milliseconds = difference_seconds * 1000;
        duration_output->mutex = ft_nullptr;
        duration_output->thread_safe_enabled = FT_FALSE;
        duration_output->milliseconds = difference_milliseconds;
    }
    delete [] left_text_copy;
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}
