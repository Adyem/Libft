#include "time.hpp"
#include <new>

ft_string *time_format_relative(t_duration_milliseconds duration_value)
{
    ft_string *duration_text;
    ft_string *result;
    int32_t error_code;

    duration_text = time_format_duration(duration_value);
    if (duration_text == ft_nullptr)
        return (ft_nullptr);
    result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
    {
        delete duration_text;
        return (ft_nullptr);
    }
    error_code = result->initialize();
    if (error_code == FT_ERR_SUCCESS)
    {
        if (duration_value.milliseconds < 0)
            error_code = result->append(duration_text->c_str());
        else
        {
            error_code = result->append("in ");
            if (error_code == FT_ERR_SUCCESS)
                error_code = result->append(duration_text->c_str());
        }
    }
    delete duration_text;
    if (error_code != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }
    return (result);
}
