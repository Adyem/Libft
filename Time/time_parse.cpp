#include "time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <ctime>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>

bool    time_parse_iso8601(const char *string_input, std::tm *time_output, t_time *timestamp_output)
{
    std::tm parsed_time;
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    int seconds;
    char timezone_character;
    std::time_t epoch_time;

    if (!string_input)
        return (false);
    std::memset(&parsed_time, 0, sizeof(parsed_time));
    if (std::sscanf(string_input, "%d-%d-%dT%d:%d:%d%c", &year, &month, &day, &hours, &minutes, &seconds, &timezone_character) != 7)
        return (false);
    if (timezone_character != 'Z')
        return (false);
    parsed_time.tm_year = year - 1900;
    parsed_time.tm_mon = month - 1;
    parsed_time.tm_mday = day;
    parsed_time.tm_hour = hours;
    parsed_time.tm_min = minutes;
    parsed_time.tm_sec = seconds;
    parsed_time.tm_isdst = 0;
    if (time_output)
        *time_output = parsed_time;
    if (timestamp_output)
    {
        epoch_time = cmp_timegm(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
            return (false);
        *timestamp_output = static_cast<t_time>(epoch_time);
    }
    return (true);
}

bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output)
{
    std::tm parsed_time;
    std::istringstream input_stream;
    std::time_t epoch_time;

    if (!string_input || !format)
        return (false);
    std::memset(&parsed_time, 0, sizeof(parsed_time));
    input_stream.str(string_input);
    input_stream >> std::get_time(&parsed_time, format);
    if (input_stream.fail())
        return (false);
    parsed_time.tm_isdst = 0;
    if (time_output)
        *time_output = parsed_time;
    if (timestamp_output)
    {
        epoch_time = std::mktime(&parsed_time);
        if (epoch_time == static_cast<std::time_t>(-1))
            return (false);
        *timestamp_output = static_cast<t_time>(epoch_time);
    }
    return (true);
}

